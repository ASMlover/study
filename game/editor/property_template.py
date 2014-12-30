#!/usr/bin/env python
# -*- encoding: utf-8 -*-

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from property_template_ui import Ui_PropertyTemplateForm

class PropertyItemType:
    BooleanValue = 1
    IntegerValue = 2
    DoubleValue = 3
    StringValue = 4

class EditorPropertyTemplate(QDialog, Ui_PropertyTemplateForm):
    def __init__(self, parent, property_list=[], title=u'属性编辑器'):
        super(EditorPropertyTemplate, self).__init__(parent)
        self.setupUi(self)
        self.setWindowTitle(title)

        self.setup_widgets(property_list)
        self.buttonCreate.clicked.connect(self.on_property_creating)

    def show(self):
        self.exec_()

    def setup_widgets(self, property_list):
        self.treeWidget.setHeaderLabels([u'属性', u'属性值'])
        self.treeWidget.setColumnWidth(0, 200)
        self.treeWidget.setStyleSheet('QTreeWidget::item{height:35px}')
        self.treeWidget.itemDoubleClicked.connect(self.on_property_editing)
        self.treeWidget.expandAll()

        for prop in property_list:
            item = QTreeWidgetItem(self.treeWidget)
            item.setText(0, prop['key'])
            item.setText(1, str(prop['value']))
            self.treeWidget.addTopLevelItem(item)

    def on_property_editing(self, item, column):
        if column == 1:
            item.setFlags(item.flags() | Qt.ItemIsEditable)
        else:
            item.setFlags(item.flags() & ~Qt.ItemIsEditable)

    def on_property_creating(self):
        QMessageBox.aboutQt(self)


if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)

    win = QWidget()
    win.show()

    prop_list = [{'key':'key1', 'value':1}, 
            {'key':'key2', 'value':2}, {'key':'key3', 'value':3}, 
            {'key':'key4', 'value':''}, {'key':'key5', 'value':'value5'}]
    e = EditorPropertyTemplate(win, prop_list)
    e.show()

    sys.exit(app.exec_())
