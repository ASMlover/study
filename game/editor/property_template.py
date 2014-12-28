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
        self.treeWidget.expandAll()

        for prop in property_list:
            for dict_item in prop.items():
                item = QTreeWidgetItem(self.treeWidget)
                item.setText(0, dict_item[0])
                item.setText(1, str(dict_item[1]))
                self.treeWidget.addTopLevelItem(item)

    def on_property_creating(self):
        QMessageBox.aboutQt(self)


if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)

    win = QWidget()
    win.show()

    prop_list = [{'key1':1}, {'key2':2}, {'key3':3}, {'key4':''}, {'key5':'value5'}]
    e = EditorPropertyTemplate(win, prop_list)
    e.show()

    sys.exit(app.exec_())
