#!/usr/bin/env python
# -*- encoding: utf-8 -*-

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

class AbstractEditor(object):
    def __init__(self):
        self.modified = False

    def modified(self):
        return self.modified

class NumberEditor(QDoubleSpinBox):
    pass
