import os
import sys
from pathlib import Path

from .utils import utils

from PySide6.QtWidgets import QApplication, QDialog, QDialogButtonBox, QColorDialog
from PySide6.QtUiTools import QUiLoader
from PySide6.QtCore import QFile, QSettings
from PySide6.QtGui import QColor


class QSettingsDialog:
    def __init__(self, configPath):
        self.window = None
        self.configPath = configPath
        self.load_settings()
        self.lastDirectory = str(Path(os.path.dirname(__file__)))

    def load_settings(self):
        if sys.platform == "win32":
            defaultPlantuml = "plantuml.jar"
        else:
            defaultPlantuml = "plantuml"
        settings = QSettings(self.configPath, QSettings.IniFormat)
        self.pathScxml2gen = settings.value("environment/scxml2gen", str(Path("../scxml2gen/scxml2gen.py")))
        self.pathPlantuml = settings.value("environment/plantuml", defaultPlantuml)
        self.loadLastHSM = bool(settings.value("common/load_last_hsm", True))
        self.styleColors = {"active_state": settings.value("style/active_state", "38EB4B"),
                            "blocked_transition": settings.value("style/blocked_transition", "EB8421")}

    def load_ui(self):
        if self.window is None:
            loader = QUiLoader()
            path = os.path.join(os.path.dirname(__file__), "..", "ui", "settings.ui")
            ui_file = QFile(path)
            ui_file.open(QFile.ReadOnly)
            self.window = loader.load(ui_file, QApplication.activeWindow())
            ui_file.close()
            self.window.setFixedSize(self.window.size())
            self.configure_ui()

    def configure_ui(self):
        self.window.btnChangeActiveColor.clicked.connect(lambda: self.selectColor("active_state", self.window.colorActive))
        self.window.btnChangeBlockedColor.clicked.connect(lambda: self.selectColor("blocked_transition", self.window.colorBlocked))
        self.window.colorActive.textChanged.connect(lambda t: self.onColorValueChanged(self.window.colorActive))
        self.window.colorBlocked.textChanged.connect(lambda t: self.onColorValueChanged(self.window.colorBlocked))
        self.window.buttonBox.clicked.connect(self.onRestoreDefaults)
        self.window.btnSelectScxml2gen.clicked.connect(self.onSelectScxml2gen)
        self.window.btnSelectPlantuml.clicked.connect(self.onSelectPlantuml)

    def update_ui(self):
        if self.window is not None:
            self.window.pathScxml2gen.setText(self.pathScxml2gen)
            self.window.pathPlantuml.setText(self.pathPlantuml)
            self.window.loadLastHSM.setChecked(self.loadLastHSM)
            self.setCurrentColor("active_state", self.window.colorActive, self.styleColors["active_state"])
            self.setCurrentColor("blocked_transition", self.window.colorBlocked, self.styleColors["blocked_transition"])

    def show(self):
        wasAccepted = False
        self.load_ui()
        self.update_ui()

        if self.window.exec_() == QDialog.Accepted:
            self.pathScxml2gen = self.window.pathScxml2gen.text()
            self.pathPlantuml = self.window.pathPlantuml.text()
            self.loadLastHSM = self.window.loadLastHSM.isChecked()
            self.styleColors = {"active_state": self.window.colorActive.text(),
                                "blocked_transition": self.window.colorBlocked.text()}
            self.saveSettings()
            wasAccepted = True
        return wasAccepted

    def saveSettings(self):
        settings = QSettings(self.configPath, QSettings.IniFormat)
        settings.setValue("environment/scxml2gen", self.pathScxml2gen)
        settings.setValue("environment/plantuml", self.pathPlantuml)
        settings.setValue("common/load_last_hsm", self.loadLastHSM)
        settings.setValue("style/active_state", self.styleColors["active_state"])
        settings.setValue("style/blocked_transition", self.styleColors["blocked_transition"])

    def onColorValueChanged(self, elem):
        newColor = QColor("#" + elem.text())
        if newColor.isValid():
            elem.setStyleSheet(f"background-color: {newColor.name()};")
        else:
            elem.setStyleSheet("")

    def selectColor(self, colorId, valueElement):
        newColor = QColorDialog.getColor(QColor("#" + valueElement.text()), QApplication.activeWindow())
        if newColor.isValid():
            self.setCurrentColor(colorId, valueElement, newColor.name(QColor.HexRgb)[1:].upper())

    def setCurrentColor(self, colorId, valueElement, newColor):
        self.styleColors[colorId] = newColor
        valueElement.setText(self.styleColors[colorId])
        valueElement.setStyleSheet(f"background-color: #{newColor};")

    def onRestoreDefaults(self, button):
        if self.window.buttonBox.standardButton(button) == QDialogButtonBox.RestoreDefaults:
            if sys.platform == "win32":
                self.pathPlantuml = "plantuml.jar"
            else:
                self.pathPlantuml = "plantuml"
            self.pathScxml2gen = "../scxml2gen/scxml2gen.py"
            self.loadLastHSM = True
            self.styleColors = {"active_state": "38EB4B",
                                "blocked_transition": "EB8421"}
            self.update_ui()
            self.saveSettings()

    def onSelectScxml2gen(self):
        (path, lastDir) = utils.selectFile("Specify path to scxml2gen.py", "Python Files (*.py)", self.lastDirectory)
        if path:
            self.window.pathScxml2gen.setText(path)
            self.lastDirectory = lastDir

    def onSelectPlantuml(self):
        (path, lastDir) = utils.selectFile("Specify path to Plantuml", "JAR Files (*.jar);;All Files (*)", self.lastDirectory)
        if path:
            self.window.pathPlantuml.setText(path)
            self.lastDirectory = lastDir
