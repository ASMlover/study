# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frame_sounds.ui'
#
# Created: Sun Dec 28 11:41:35 2014
#      by: PyQt5 UI code generator 5.3.2
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_FrameSoundsForm(object):
    def setupUi(self, FrameSoundsForm):
        FrameSoundsForm.setObjectName("FrameSoundsForm")
        FrameSoundsForm.resize(311, 161)
        self.label = QtWidgets.QLabel(FrameSoundsForm)
        self.label.setGeometry(QtCore.QRect(20, 10, 41, 16))
        self.label.setObjectName("label")
        self.frame_index_ = QtWidgets.QSpinBox(FrameSoundsForm)
        self.frame_index_.setGeometry(QtCore.QRect(70, 10, 211, 22))
        self.frame_index_.setAlignment(QtCore.Qt.AlignCenter)
        self.frame_index_.setMaximum(100)
        self.frame_index_.setObjectName("frame_index_")
        self.label_2 = QtWidgets.QLabel(FrameSoundsForm)
        self.label_2.setGeometry(QtCore.QRect(20, 60, 41, 16))
        self.label_2.setObjectName("label_2")
        self.frame_sound_ = QtWidgets.QLineEdit(FrameSoundsForm)
        self.frame_sound_.setGeometry(QtCore.QRect(70, 60, 211, 20))
        self.frame_sound_.setObjectName("frame_sound_")
        self.btnCreate = QtWidgets.QPushButton(FrameSoundsForm)
        self.btnCreate.setGeometry(QtCore.QRect(110, 130, 75, 23))
        self.btnCreate.setObjectName("btnCreate")

        self.retranslateUi(FrameSoundsForm)
        QtCore.QMetaObject.connectSlotsByName(FrameSoundsForm)

    def retranslateUi(self, FrameSoundsForm):
        _translate = QtCore.QCoreApplication.translate
        FrameSoundsForm.setWindowTitle(_translate("FrameSoundsForm", "FrameSoundsForm"))
        self.label.setText(_translate("FrameSoundsForm", "帧序："))
        self.label_2.setText(_translate("FrameSoundsForm", "音乐："))
        self.btnCreate.setText(_translate("FrameSoundsForm", "创建"))

