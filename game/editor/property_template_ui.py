# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'property_template.ui'
#
# Created: Sun Dec 28 16:22:54 2014
#      by: PyQt5 UI code generator 5.3.2
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_PropertyTemplateForm(object):
    def setupUi(self, PropertyTemplateForm):
        PropertyTemplateForm.setObjectName("PropertyTemplateForm")
        PropertyTemplateForm.resize(429, 375)
        self.treeWidget = QtWidgets.QTreeWidget(PropertyTemplateForm)
        self.treeWidget.setGeometry(QtCore.QRect(0, 0, 431, 321))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.treeWidget.setFont(font)
        self.treeWidget.setAutoFillBackground(True)
        self.treeWidget.setLineWidth(2)
        self.treeWidget.setColumnCount(2)
        self.treeWidget.setObjectName("treeWidget")
        self.treeWidget.headerItem().setText(0, "1")
        self.treeWidget.headerItem().setText(1, "2")
        self.treeWidget.header().setVisible(True)
        self.buttonCreate = QtWidgets.QPushButton(PropertyTemplateForm)
        self.buttonCreate.setGeometry(QtCore.QRect(160, 350, 75, 23))
        self.buttonCreate.setObjectName("buttonCreate")

        self.retranslateUi(PropertyTemplateForm)
        QtCore.QMetaObject.connectSlotsByName(PropertyTemplateForm)

    def retranslateUi(self, PropertyTemplateForm):
        _translate = QtCore.QCoreApplication.translate
        PropertyTemplateForm.setWindowTitle(_translate("PropertyTemplateForm", "PropertyTemplate"))
        self.buttonCreate.setText(_translate("PropertyTemplateForm", "创建"))

