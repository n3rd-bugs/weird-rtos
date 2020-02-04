"""
This file implements analog plugin for werid view.
"""
from PyQt5 import QtWidgets
from PyQt5.Qt import QWidget, QTimer, QDoubleValidator, QThread
from PyQt5.QtCore import pyqtSignal, QMutex, QWaitCondition
from socket import socket, AF_INET, SOCK_DGRAM, SOL_SOCKET, SO_REUSEADDR
from weird_view import WV_UPDATE, WV_UPDATE_REPLY, WV_REQ_TIMEOUT

# Enable/disable debugging for this module.
DEBUG           = True

"""
This is analog refresh thread class.
"""
class AnalogRefreshThread(QThread):
    
    # Initialize class signals.
    data_signal = pyqtSignal(int, int, int)
    
    """
    This function is responsible for initializing any related objects for 
    refresh thread.
    """
    def __init__(self, plugin_id, address, socket):
        QThread.__init__(self)
        self.plugin_id = plugin_id
        self.address = address
        self.udp_socket = socket
        self.finished.connect(self.quit)
        self.wait = QWaitCondition()
        self.mutex = QMutex()

    """
    This is the entry point for refresh thread.
    """
    def run(self):
        
        # Run indefinately.
        while True:
            
            # Wait for trigger.
            self.mutex.lock()
            self.wait.wait(self.mutex)
            self.mutex.unlock()
            
            # Receive and discard any datagrams from this socket.
            self.udp_socket.setblocking(0)
            try:
                while True:
                    _ = self.udp_socket.recv(65535)
            except:
                pass
            self.udp_socket.setblocking(1)
            self.udp_socket.settimeout(WV_REQ_TIMEOUT)
            
            try:
                rx_data = None
                
                if DEBUG:
                    print("Sending an update request for", self.plugin_id, "to", self.address)
                
                # Send a request update for this plugin.
                self.udp_socket.sendto(bytes.fromhex(WV_UPDATE) + bytes([((self.plugin_id & 0xFF00) >> 8), (self.plugin_id & 0xFF)]), self.address)
                
                # Receive data form the UDP port.
                rx_data = self.udp_socket.recv(65535)
            
                # If we did receive a vaild reply.
                if (rx_data[ : 4] == bytes.fromhex(WV_UPDATE_REPLY)):
                    
                    # Remove packet descriptor.
                    rx_data = rx_data[4 : ]
                    
                    # If we did receive some data.
                    if len(rx_data) == 12:
                        if DEBUG:
                            print("Got an update for", self.plugin_id, "from", self.address)
                        
                        # Parse and emit the received data.
                        self.data_signal.emit(int((rx_data[0] << 24) + (rx_data[1] << 16) + (rx_data[2] << 8) + (rx_data[3] << 0)), 
                                              int((rx_data[4] << 24) + (rx_data[5] << 16) + (rx_data[6] << 8) + (rx_data[7] << 0)), 
                                              int((rx_data[8] << 24) + (rx_data[9] << 16) + (rx_data[10] << 8) + (rx_data[11] << 0)))
                        
                    else:
                        if DEBUG:
                            print("No data received for", self.plugin_id, "from", self.address, "or invalid data was received")
                else:
                    if DEBUG:
                        print("Invalid header for", self.plugin_id, "from", self.address)
    
            except:
                
                # Nothing to do here.
                pass

"""
This is weird view analog plugin class.
"""
class PluginAnalog(QWidget):
    
    """
    This function is responsible for initializing a log plugin. 
    """
    def __init__(self, window, grid, name, plugin_id, address):
        
        # Save the window data.
        self.address = address
        self.name = name
        self.plugin_id = plugin_id
        
        # Initialize this class.
        super(PluginAnalog, self).__init__()
        
        # Initialize a UDP socket for this plugin.
        self.udp_socket = socket(AF_INET, SOCK_DGRAM)
        
        # Set socket options.
        self.udp_socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.udp_socket.settimeout(WV_REQ_TIMEOUT)
        
        # Initialize a timer to query data for this plugin.
        self.timer = QTimer()
        self.timer.timeout.connect(self.refresh);
        
        # Initialize update thread.
        self.refresh_thread = AnalogRefreshThread(self.plugin_id, self.address, self.udp_socket)
        self.refresh_thread.start()
        
        # Connect the data signal.
        self.refresh_thread.data_signal.connect(self.do_update)

        # Initialize UI for this plugin.
        self.PluginName = QtWidgets.QLabel(window)
        self.PluginName.setText(name)
        grid.addWidget(self.PluginName, grid.rowCount(), 0)
        
        RefreshSpacer = QtWidgets.QSpacerItem(65535, 20, QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Minimum)
        grid.addItem(RefreshSpacer, (grid.rowCount() - 1), 1)
        
        self.RefreshLable = QtWidgets.QLabel(window)
        self.RefreshLable.setText("Refresh Time (ms) :")
        grid.addWidget(self.RefreshLable, (grid.rowCount() - 1), 2)
        
        self.Refresh = QtWidgets.QLineEdit(window)
        self.Refresh.setValidator(QDoubleValidator(0, 999999, 2))
        self.Refresh.setMinimumWidth(50)
        self.Refresh.textChanged.connect(self.refersh_time_updated)
        self.Refresh.setText('500')
        grid.addWidget(self.Refresh, (grid.rowCount() - 1), 3)
        
        self.AnalogDial = QtWidgets.QDial(window)
        self.AnalogDial.setNotchesVisible(True)
        self.AnalogDial.setEnabled(False)
        grid.addWidget(self.AnalogDial, grid.rowCount(), 2, 5, 1)
        
        self.AnalogMaxLable = QtWidgets.QLabel(window)
        self.AnalogMaxLable.setText("Max Value :")
        grid.addWidget(self.AnalogMaxLable, (grid.rowCount() - 2), 0)
        
        self.AnalogMax = QtWidgets.QLabel(window)
        self.AnalogMax.setText("?")
        grid.addWidget(self.AnalogMax, (grid.rowCount() - 2), 1)
        
        self.AnalogCurrentLable = QtWidgets.QLabel(window)
        self.AnalogCurrentLable.setText("Current Value :")
        grid.addWidget(self.AnalogCurrentLable, (grid.rowCount() - 1), 0)
        
        self.AnalogCurrent = QtWidgets.QLabel(window)
        self.AnalogCurrent.setText("?")
        grid.addWidget(self.AnalogCurrent, (grid.rowCount() - 1), 1)
    
    """
    This is callback function for refresh timer.
    """
    def refresh(self):
        
        if DEBUG:
            print("Triggering an update for", self.plugin_id)
            
        # Wake the update thread.
        self.refresh_thread.wait.wakeAll()
        
    """
    This function is responsible for updating plugin display data.
    """
    def do_update(self, value, value_div, max_value):
        
        # Divider value should never be zero.
        if (value_div <= 0):
            value_div = 1
            
        if DEBUG:
            print("Got an update for", self.plugin_id)
            print("Analog values for", self.plugin_id, "value:", (value / value_div), "max:", max_value)
        
        # Display the received data.
        self.AnalogMax.setText(str(max_value))
        self.AnalogCurrent.setText(str(value / value_div))
        
        if (max_value < 10):
            self.AnalogDial.setPageStep(100)
            self.AnalogDial.setSingleStep(10)
            value_div = value_div / 100
            max_value = max_value * 100
        elif (max_value < 100):
            self.AnalogDial.setPageStep(10)
            self.AnalogDial.setSingleStep(1)
            value_div = value_div / 10
            max_value = max_value * 10
        else:
            value_div = value_div
            max_value = max_value
        
        # Display the analog data.
        self.AnalogDial.setMaximum(max_value)
        self.AnalogDial.setSliderPosition(value / value_div)
        
    """
    This is callback function when refresh period updates.
    """
    def refersh_time_updated(self):
        
        refresh_time = 0
        try:
            refresh_time = int(self.Refresh.text());
        except:
            pass
        if refresh_time < 1:
            refresh_time = 1;
        
        # Reset the timer with new refersh time.
        self.timer.start(refresh_time);
        