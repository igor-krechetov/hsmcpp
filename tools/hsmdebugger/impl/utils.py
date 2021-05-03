# Copyright (C) 2021 Igor Krechetov
# Distributed under MIT license. See file LICENSE for details
import os
import sys
from PySide6.QtWidgets import QApplication, QFileDialog

class utils(object):
    @staticmethod
    def selectFile(title, filter, lastDirectory):
        selectedPath = None
        dlgFileSelector = QFileDialog(QApplication.activeWindow(), title)
        # NOTE: native file dialog on Ubuntu returns fake file path which are impossible to use in "recent files"
        if sys.platform != "win32":
            dlgFileSelector.setOption(QFileDialog.DontUseNativeDialog)
        dlgFileSelector.setDirectory(lastDirectory)
        dlgFileSelector.setNameFilter(filter)
        if dlgFileSelector.exec_():
            selectedPath = dlgFileSelector.selectedFiles()[0]
            lastDirectory = os.path.dirname(selectedPath)
        return (selectedPath, lastDirectory)