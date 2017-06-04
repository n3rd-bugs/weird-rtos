"""
This file implements plugin window component.
"""
from PyQt5 import Qt, QtCore, QtWidgets
from device_discovery import PluginDiscoveryThread
from plugin_log import PluginLog
from plugin_wave import PluginWave
from plugin_switch import PluginSwitch
from plugin_analog import PluginAnalog
from weird_view import WV_PLUGIN_LOG, WV_PLUGIN_SWITCH, WV_PLUGIN_ANALOG, WV_PLUGIN_WAVE

PLUGIN_DB = {WV_PLUGIN_LOG : PluginLog, WV_PLUGIN_SWITCH : PluginSwitch, WV_PLUGIN_ANALOG : PluginAnalog, WV_PLUGIN_WAVE : PluginWave}

"""
This is weird view plugin window class.
"""
class PluginWindow(Qt.QMainWindow):
    
    """
    This function is responsible for initializing plugin window for a new device. 
    """
    def __init__(self, window, name, address):
        
        # Save the window data.
        self.address = address
        self.name = name
        self.mainwindow = window
        
        # Initialize this class.
        super(PluginWindow, self).__init__()
        
        # Initialize UI for plugin window.
        self.gridLayout = QtWidgets.QGridLayout(window)
        self.Grid = QtWidgets.QGridLayout()
        self.gridLayout.addLayout(self.Grid, 0, 0, 1, 1)
        self.mainwindow.setWindowTitle(self.name)
        
        # Start a thread to load a list of all the plugins from the remote.
        self.plugin_discovery = PluginDiscoveryThread()
        self.plugin_discovery.plugin_signal.connect(self.load_plugins)
        self.plugin_discovery.start_discover(address)
        
    """
    This function is responsible to display window title.
    """
    def retranslateUi(self, PluginWindow):
        _translate = QtCore.QCoreApplication.translate
        PluginWindow.setWindowTitle(_translate("PluginWindow", self.name))
    
    """
    This function is responsible for loading a plugin list for the given device.
    """
    def load_plugins(self, plugin_list):
        
        # Stop the plugin discovery.
        self.plugin_discovery.stop_discover()
        self.plugins = {}
        
        # Close the main window.
        self.mainwindow.close()
        
        # Initialize all the requested plugins.
        for plugin in plugin_list:
            
            # If we do support this plugin.
            if (plugin['type'] in PLUGIN_DB) and PLUGIN_DB[plugin['type']] is not None:
                
                # Initialize plugin widget.
                self.plugins[str(plugin['id'])] = PLUGIN_DB[plugin['type']](self.mainwindow, self.Grid, plugin['name'], plugin['id'], self.address)
                
                # If this was not the last plugin.
                if plugin is not plugin_list[len(plugin_list) - 1]:
                    
                    # Add a line after adding a plugin.
                    self.plugins[str(plugin['id'])].EndLine = QtWidgets.QFrame(self.mainwindow)
                    self.plugins[str(plugin['id'])].EndLine.setFrameShape(QtWidgets.QFrame.HLine)
                    self.plugins[str(plugin['id'])].EndLine.setFrameShadow(QtWidgets.QFrame.Sunken)
                    self.Grid.addWidget(self.plugins[str(plugin['id'])].EndLine, self.Grid.rowCount(), 0, 1, self.Grid.columnCount())
        
        # Show the main window.
        self.mainwindow.showMaximized()