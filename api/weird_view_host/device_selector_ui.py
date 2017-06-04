# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'device_selector_ui.ui'
#
# Created by: PyQt5 UI code generator 5.4.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_DeviceSelector(object):
    def setupUi(self, DeviceSelector):
        DeviceSelector.setObjectName("DeviceSelector")
        DeviceSelector.resize(325, 220)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(DeviceSelector.sizePolicy().hasHeightForWidth())
        DeviceSelector.setSizePolicy(sizePolicy)
        DeviceSelector.setMinimumSize(QtCore.QSize(325, 220))
        DeviceSelector.setMaximumSize(QtCore.QSize(325, 220))
        self.DeviceListWidget = QtWidgets.QWidget(DeviceSelector)
        self.DeviceListWidget.setObjectName("DeviceListWidget")
        self.DeviceListGrid = QtWidgets.QGridLayout(self.DeviceListWidget)
        self.DeviceListGrid.setObjectName("DeviceListGrid")
        self.DeviceListGrid1 = QtWidgets.QGridLayout()
        self.DeviceListGrid1.setObjectName("DeviceListGrid1")
        self.DeviceList = QtWidgets.QTableWidget(self.DeviceListWidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Minimum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.DeviceList.sizePolicy().hasHeightForWidth())
        self.DeviceList.setSizePolicy(sizePolicy)
        self.DeviceList.setMinimumSize(QtCore.QSize(305, 200))
        self.DeviceList.setMaximumSize(QtCore.QSize(305, 200))
        self.DeviceList.setFrameShadow(QtWidgets.QFrame.Plain)
        self.DeviceList.setSelectionMode(QtWidgets.QAbstractItemView.SingleSelection)
        self.DeviceList.setObjectName("DeviceList")
        self.DeviceList.setColumnCount(0)
        self.DeviceList.setRowCount(0)
        self.DeviceListGrid1.addWidget(self.DeviceList, 0, 0, 1, 1, QtCore.Qt.AlignHCenter|QtCore.Qt.AlignVCenter)
        self.DeviceListGrid.addLayout(self.DeviceListGrid1, 0, 0, 1, 1)
        DeviceSelector.setCentralWidget(self.DeviceListWidget)

        self.retranslateUi(DeviceSelector)
        QtCore.QMetaObject.connectSlotsByName(DeviceSelector)

    def retranslateUi(self, DeviceSelector):
        _translate = QtCore.QCoreApplication.translate
        DeviceSelector.setWindowTitle(_translate("DeviceSelector", "Select a device"))

