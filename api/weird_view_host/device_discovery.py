"""
This file implements device discovery components.
"""
from PyQt5.QtCore import pyqtSignal, QThread
from socket import socket, AF_INET, SOCK_DGRAM, SOL_SOCKET, SO_BROADCAST, SO_REUSEADDR
from datetime import datetime, timedelta
from weird_view import WV_DISC, WV_DISC_REPLY, WV_LIST, WV_LIST_REPLY
from time import sleep

# Seconds after which we will retry a discover request.
DISC_TIMEOUT    = 5

# Enable/disable debugging for this module.
DEBUG           = True

"""
This thread class is responsible for discovering devices on the network.
"""
class DiscoveryThread(QThread):
    
    # Initialize class signals.
    device_signal = pyqtSignal(str, tuple)
    next_retry = None
    
    """
    This function is responsible for initializing any related objects for 
    discovery thread.
    """
    def __init__(self):
        QThread.__init__(self)
    
    """
    This function is responsible for starting a discovery thread.
    """
    def start_discover(self):
        
        if DEBUG:
            print("Starting discover thread.")
        
        # Create a UDP socket.
        self.udp_socket = socket(AF_INET, SOCK_DGRAM)
        
        # Set socket options.
        self.udp_socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.udp_socket.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
        self.udp_socket.settimeout(0.1)
        
        # Initialize discovery parameters.
        self.next_retry = datetime.now()
        self.running = True
        
        # Start discovery in an other thread.
        self.start()
    
    """
    This function is responsible for stoping the discovery thread.
    """
    def stop_discover(self):
        
        # If discover thread is not running.
        if self.running == True:
            if DEBUG:
                print("Stoping discover thread.")
                
            # Set flag to stop the discover loop.
            self.running = False
            
            # Close the UDP socket.
            self.udp_socket.close()
            
            # Wait while thread is running.
            while self.isFinished() == False:
                sleep(0.05)
        
            # Terminate discovery task.
            self.terminate()
            self.deleteLater()
                
        elif DEBUG:
            print("Discover thread already stopped.")

    """
    Thread entry point.
    """
    def run(self):
        
        # We will keep sending a request until a device respond to our request.
        while self.running == True:
            
            # If we need to resend a discover request.
            if datetime.now() >= self.next_retry:
                
                if DEBUG:
                    print("Sending a discover request on broadcast address.")
                
                try:
                    # Broadcast discover to all listening device.
                    self.udp_socket.sendto(bytes.fromhex(WV_DISC), ("255.255.255.255", 11000))
                
                    # Retry sending a discover request after configured time-out.
                    self.next_retry = datetime.now() + timedelta(0, DISC_TIMEOUT)
                    
                except:
                    pass
            
            try:
                # Receive data form the UDP port.
                rx_data, remote_address = self.udp_socket.recvfrom(65535)
                
                # If discover reply was received.
                if rx_data[0 : 4] == bytes.fromhex(WV_DISC_REPLY):
                    if DEBUG:
                        print("Discover reply received from :", remote_address)
                    
                    # Signal device address to any listeners.
                    self.device_signal.emit(str(rx_data[4 : ].decode('ascii')), remote_address)
                
            except:
                continue

"""
This thread class is responsible for discoving supported plugins on a device.
"""
class PluginDiscoveryThread(QThread):
    
    # Initialize class signals.
    plugin_signal = pyqtSignal(list)
    next_retry = None
    
    """
    This function is responsible for initializing any related objects for 
    plugin discovery thread.
    """
    def __init__(self):
        QThread.__init__(self)
    
    """
    This function is responsible for starting a discovery thread.
    """
    def start_discover(self, address):
        
        if DEBUG:
            print("Starting plugin discovery thread.")
            
        # Create a UDP socket.
        self.udp_socket = socket(AF_INET, SOCK_DGRAM)
        self.address = address
        
        # Set socket options.
        self.udp_socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.udp_socket.settimeout(1)
        
        # Setup discovery parameters.
        self.next_retry = datetime.now()
        
        # Start discovery in an other thread.
        self.start()
    
    """
    This function is responsible for stoping the discovery thread.
    """
    def stop_discover(self):
        
        # Close the UDP socket.
        self.udp_socket.close()
        
        # Wait while thread is running.
        while self.isFinished() == False:
            sleep(0.05)
        
        # Terminate discovery task.
        self.terminate()
        self.deleteLater()

    """
    Thread entry point.
    """
    def run(self):
        
        while True:
            
            # If we need to resend a discover request.
            if datetime.now() >= self.next_retry:
                if DEBUG:
                    print("Sending a plugin discovery request.")
                
                # Broadcast discover to all listening device.
                self.udp_socket.sendto(bytes.fromhex(WV_LIST), self.address)
                
                # Retry sending a discover request after configured time-out.
                self.next_retry = datetime.now() + timedelta(0, 1)
            
            try:
                # Receive data form the UDP port.
                rx_data = self.udp_socket.recv(65535)
                
                # If discover reply was received.
                if rx_data[0 : 4] == bytes.fromhex(WV_LIST_REPLY):
                    if DEBUG:
                        print("List reply received.")
                    
                    plugin_list = []
                    rx_data = rx_data[4 : ]
                    
                    # We should have atleast 2 bytes in the data to parse a 
                    # plugin.
                    while len(rx_data) >= 4:
                        # Parse the discovery list.
                        this_plugin = {}
                        
                        # Pull the plugin id.
                        this_plugin['id'] = ((int(rx_data[0]) << 8) + int(rx_data[1]))
                        rx_data = rx_data[2 : ]
                        
                        # Pull the plugin type.
                        this_plugin['type'] = str(int(rx_data[0]))
                        rx_data = rx_data[1 : ]
                        
                        # Check if we can have the name of the plugin.
                        if ((rx_data[0] + 1) > len(rx_data)):
                            break
                        
                        # Pull the plugin name.
                        this_plugin['name'] = (str(rx_data[1 : (rx_data[0] + 1)].decode('ascii')))
                        rx_data = rx_data[(rx_data[0] + 1) : ]
                        
                        if DEBUG:
                            print("Plugin parsed : ", this_plugin)
                        
                        # Add the parsedd plugin to the plugin list.
                        plugin_list.append(this_plugin)
                        
                    # If plugin list was successfully parsed.
                    if len(rx_data) == 0:
                        
                        # Signal the plugin list we received.
                        self.plugin_signal.emit(plugin_list)
                        break
                
            except:
                continue
        