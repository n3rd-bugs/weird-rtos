"""
This file implements device section window.
"""
from device_selector_ui import Ui_DeviceSelector
from PyQt5 import Qt
from PyQt5.QtCore import pyqtSignal
from device_discovery import DiscoveryThread

# Enable/disable debugging for this module.
DEBUG           = True

"""
Device selection window.
"""
class DeviceSelector(Ui_DeviceSelector, Qt.QMainWindow):
    
    # List to maintain all the devices on the network.
    device_list = []
    
    # Device selection signal.
    device_selection_signal = pyqtSignal(str, tuple)
    
    """
    This function is responsible for initializing device selection window. 
    """
    def __init__(self, window):
        super(DeviceSelector, self).__init__()
        self.setupUi(window)
        
        self.DeviceList.setRowCount(0)
        self.DeviceList.setColumnCount(2)
        self.DeviceList.setHorizontalHeaderLabels(['Name', 'Network Address'])
        self.DeviceList.setColumnWidth(0, 200)
        self.DeviceList.setColumnWidth(1, 100)
        self.DeviceList.horizontalHeader().setSectionsClickable(False)
        self.DeviceList.horizontalHeader().setSectionResizeMode(Qt.QHeaderView.Fixed)
        self.DeviceList.verticalHeader().setVisible(False)
        
        # Start a device discovery.
        self.discovery = DiscoveryThread()
        self.discovery.device_signal.connect(self.got_a_device)
        self.discovery.start_discover()
        
        # Connect section.
        self.DeviceList.cellDoubleClicked.connect(self.device_selected)

    """
    This function is called when a device is detected on the network.
    """
    def got_a_device(self, name, address):
        
        if DEBUG:
            print("Device detected : ", name, " at ", address)
        
        # If given address is unique for this session.
        if address not in self.device_list:
            
            if DEBUG:
                print("This was a new device")
            
            # Add this address in the device list.
            self.device_list.append(address)
            
            # Add this device to the device list.
            self.DeviceList.setRowCount(self.DeviceList.rowCount() + 1)
            this_device = Qt.QTableWidgetItem(name)
            this_device.setFlags(Qt.Qt.ItemIsSelectable | Qt.Qt.ItemIsEnabled)
            this_device.address = address
            self.DeviceList.setItem((self.DeviceList.rowCount() - 1), 0, this_device)
            
            this_device = Qt.QTableWidgetItem(str(address[0]))
            this_device.setFlags(Qt.Qt.NoItemFlags)
            self.DeviceList.setItem((self.DeviceList.rowCount() - 1), 1, this_device)

    """
    This function is called when user selects a required device.
    """
    def device_selected(self, row, column):
        
        if DEBUG:
            print("Device selected at ", row, ", ",column)
        
        # Stop discover thread.
        self.discovery.stop_discover()
        
        # Signal the selected device information.
        self.device_selection_signal.emit(self.DeviceList.item(row, column).text(), self.DeviceList.item(row, column).address)