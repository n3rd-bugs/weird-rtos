"""
This file implements log plugin for werid view.
"""
from PyQt5 import QtWidgets, QtGui
from PyQt5.Qt import QWidget, QTimer, QDoubleValidator, QThread
from PyQt5.QtCore import pyqtSignal, QMutex, QWaitCondition
from socket import socket, AF_INET, SOCK_DGRAM, SOL_SOCKET, SO_REUSEADDR
from weird_view import WV_UPDATE, WV_UPDATE_REPLY, WV_PLUGIN_LOG_UPDATE, WV_PLUGIN_LOG_APPEND,\
    WV_REQ_TIMEOUT

# Enable/disable debugging for this module.
DEBUG           = True

"""
This is log refresh thread class.
"""
class LogRefreshThread(QThread):
    
    # Initialize class signals.
    data_signal = pyqtSignal(bool, str)
    
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
            rx_data = None
            
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
                    if len(rx_data) > 0:
                        if DEBUG:
                            print("Got an update for", self.plugin_id, "from", self.address)
                        
                        # Check if we need to update the existing data.
                        if (rx_data[0] == WV_PLUGIN_LOG_UPDATE):
                            
                            # Signal the data to update the plugin display.
                            self.data_signal.emit(False, rx_data[1 : ].decode('ascii'))
                        
                        # Check if we need to append the existing data.
                        elif (rx_data[0] == WV_PLUGIN_LOG_APPEND):
                            
                            # Signal the data to append the plugin display.
                            self.data_signal.emit(True, rx_data[1 : ].decode('ascii'))
                        else:
                            if DEBUG:
                                print("Invalid header for", self.plugin_id, "from", self.address)
                    else:
                        if DEBUG:
                            print("No data received for", self.plugin_id, "from", self.address)
                else:
                    if DEBUG:
                        print("Invalid header for", self.plugin_id, "from", self.address)
    
            except:
                
                # Nothing to do here.
                pass

"""
This is weird view log plugin class.
"""
class PluginLog(QWidget):
    
    """
    This function is responsible for initializing a log plugin. 
    """
    def __init__(self, window, grid, name, plugin_id, address):
        
        # Save the window data.
        self.address = address
        self.name = name
        self.plugin_id = plugin_id
        
        # Initialize this class.
        super(PluginLog, self).__init__()
        
        # Initialize a UDP socket for this plugin.
        self.udp_socket = socket(AF_INET, SOCK_DGRAM)
        
        # Set socket options.
        self.udp_socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.udp_socket.settimeout(WV_REQ_TIMEOUT)
        
        # Initialize a timer to query data for this plugin.
        self.timer = QTimer()
        self.timer.timeout.connect(self.refresh);
        
        # Initialize update thread.
        self.refresh_thread = LogRefreshThread(self.plugin_id, self.address, self.udp_socket)
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
        self.Refresh.setText('1000')
        grid.addWidget(self.Refresh, (grid.rowCount() - 1), 3)
        
        self.PluginData = QtWidgets.QTextBrowser(window)
        font = QtGui.QFont()
        font.setFamily("Consolas")
        self.PluginData.setFont(font)
        grid.addWidget(self.PluginData, grid.rowCount(), 0, 1, grid.columnCount())
    
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
    def do_update(self, append, data):
        
        if DEBUG:
            print("Got an update for", self.plugin_id)
        
        # Check if we need to append the existing data.
        if (append):
            # Append existing data.
            self.PluginData.append(data)
        
        # Update existing data.
        else:
            
            # Update data in plugin window.
            self.PluginData.setText(data)
    
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
        