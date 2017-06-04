"""
This file implements main logic for wierd view client.
"""
import sys
from PyQt5.QtWidgets import QApplication
from PyQt5.Qt import QMainWindow, QWidget
from device_selector import DeviceSelector
from plugin_window import PluginWindow

"""
This is weird view client class.
"""
class WeirdViewClient(QMainWindow):

    """
    Weird view client initializer.
    """
    def __init__(self):
        super().__init__()
        self.device_selector_window = QMainWindow()
        self.device_selector = DeviceSelector(self.device_selector_window)
        self.device_selector.device_selection_signal.connect(self.device_open)
        self.device_selector_window.show()
        
    """
    Weird view device open callback.
    """
    def device_open(self, name, address):
        
        # Close the old device selector window.
        self.device_selector_window.close()
        self.plugin_window = QWidget()
        self.plugin = PluginWindow(self.plugin_window, name, address)
        self.plugin_window.show()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = WeirdViewClient()
    sys.exit(app.exec_())
