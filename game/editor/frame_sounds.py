#!/usr/bin/env python
# -*- encoding: utf-8 -*-

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from frame_sounds_ui import Ui_FrameSoundsForm

class EditorFrameSounds(QDialog, Ui_FrameSoundsForm):
    def __init__(self, parent, title=u'创建音乐'):
        super(EditorFrameSounds, self).__init__(parent)
        self.setupUi(self)
        self.setWindowTitle(title)

        self.btnCreate.clicked.connect(self.on_framesound_creating)

    def show(self):
        self.exec_()

    def on_framesound_creating(self):
        frame_index = self.frame_index_.value()
        frame_sound = self.frame_sound_.text()
        if frame_sound == '':
            QMessageBox.warning(self, u'创建音乐', '请填写音乐')
            return

        print('------------- EditorFrameSounds.on_framesound_creating', frame_index, frame_sound)
        # TODO:
        self.close()


if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    
    win = QWidget()
    win.show()

    e = EditorFrameSounds(win)
    e.show()
    
    sys.exit(app.exec_())
