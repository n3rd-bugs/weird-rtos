"""
This file implements switch plugin for werid view.
"""
from PyQt5 import QtWidgets
from PyQt5.Qt import QWidget, QTimer, QDoubleValidator, QThread
from PyQt5.QtCore import pyqtSignal, QMutex, QWaitCondition
from socket import socket, AF_INET, SOCK_DGRAM, SOL_SOCKET, SO_REUSEADDR
from weird_view import WV_UPDATE, WV_UPDATE_REPLY, WV_REQ, WV_PLUGIN_SWITCH_ON, WV_PLUGIN_SWITCH_OFF,\
    WV_REQ_TIMEOUT

# Enable/disable debugging for this module.
DEBUG           = True

"""
This is switch refresh thread class.
"""
class SwitchRefreshThread(QThread):
    
    # Initialize class signals.
    data_signal = pyqtSignal(bool)
    
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
                self.udp_socket.sendto(bytes.fromhex(WV_UPDATE) + bytes([((self.plugin_id & 0xFF00) >> 8), (self.plugin_id & 0x00FF)]), self.address)
                
                # Receive data form the UDP port.
                rx_data = self.udp_socket.recv(65535)
            
                # If we did receive a vaild reply.
                if (rx_data[ : 4] == bytes.fromhex(WV_UPDATE_REPLY)):
                    
                    # Remove packet descriptor.
                    rx_data = rx_data[4 : ]
                    
                    # If we did receive some data.
                    if len(rx_data) == 1:
                        if DEBUG:
                            print("Got an update for", self.plugin_id, "from", self.address)
                        
                        # Check if we are now turned on.
                        if (rx_data[0] == WV_PLUGIN_SWITCH_ON):
                            
                            # Signal the data to update the plugin display.
                            self.data_signal.emit(True)
                        
                        # Check if we are now turned off.
                        elif (rx_data[0] == WV_PLUGIN_SWITCH_OFF):
                            
                            # Signal the data to append the plugin display.
                            self.data_signal.emit(False)
                        else:
                            if DEBUG:
                                print("Invalid header for", self.plugin_id, "from", self.address)
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
This is weird view swicth plugin class.
"""
class PluginSwitch(QWidget):
    
    """
    This function is responsible for initializing a log plugin. 
    """
    def __init__(self, window, grid, name, plugin_id, address):
        
        # Save the window data.
        self.address = address
        self.name = name
        self.plugin_id = plugin_id
        
        # Initialize this class.
        super(PluginSwitch, self).__init__()
        
        # Initialize a UDP socket for this plugin.
        self.udp_socket = socket(AF_INET, SOCK_DGRAM)
        
        # Set socket options.
        self.udp_socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.udp_socket.settimeout(WV_REQ_TIMEOUT)
        
        # Initialize a timer to query data for this plugin.
        self.timer = QTimer()
        self.timer.timeout.connect(self.refresh);
        
        # Initialize update thread.
        self.refresh_thread = SwitchRefreshThread(self.plugin_id, self.address, self.udp_socket)
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
        
        self.StateLable = QtWidgets.QLabel(window)
        self.StateLable.setText("Current State :")
        grid.addWidget(self.StateLable, grid.rowCount(), 0)
        
        self.State = QtWidgets.QLabel(window)
        self.State.setText("?")
        grid.addWidget(self.State, (grid.rowCount() - 1), 1)
        
        self.StateOn = QtWidgets.QPushButton(window)
        self.StateOn.setText("On")
        self.StateOn.clicked.connect(self.set_on)
        grid.addWidget(self.StateOn, (grid.rowCount() - 1), 2)
        
        self.StateOff = QtWidgets.QPushButton(window)
        self.StateOff.setText("Off")
        self.StateOff.clicked.connect(self.set_off)
        grid.addWidget(self.StateOff, (grid.rowCount() - 1), 3)
    
    """
    This is callback function for refresh timer.
    """
    def refresh(self):
        
        if DEBUG:
            print("Triggering an update for", self.plugin_id)
            
        # Wake the update thread.
        self.refresh_thread.wait.wakeAll()
    
    """
    This is callback function for turn on button.
    """
    def set_on(self):
        if DEBUG:
            print("Turned on requested for switch at", self.plugin_id)
                
        # Send an on request for this swicth.
        self.udp_socket.sendto(bytes.fromhex(WV_REQ) + bytes([((self.plugin_id & 0xFF00) >> 8), (self.plugin_id & 0x00FF)]) + bytes([WV_PLUGIN_SWITCH_ON]), self.address)
    
    """
    This is callback function for turn off button.
    """
    def set_off(self):
        if DEBUG:
            print("Turned off requested for switch at", self.plugin_id)
                
        # Send an off request for this swicth.
        self.udp_socket.sendto(bytes.fromhex(WV_REQ) + bytes([((self.plugin_id & 0xFF00) >> 8), (self.plugin_id & 0x00FF)]) + bytes([WV_PLUGIN_SWITCH_OFF]), self.address)
    
    """
    This function is responsible for updating plugin display data.
    """
    def do_update(self, state):
        if DEBUG:
            print("Got an update for", self.plugin_id)
        
        if state == True:
            if DEBUG:
                print("Switch turned on at", self.plugin_id)
            
            # We are now turned on.
            self.State.setText("On")
        else:
            if DEBUG:
                print("Switch turned off at", self.plugin_id)
            
            # We are now turned off.
            self.State.setText("Off")
            
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
        