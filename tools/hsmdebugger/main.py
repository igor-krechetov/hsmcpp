# Copyright (C) 2021 Igor Krechetov
# Distributed under MIT license. See file LICENSE for details

# This Python file uses the following encoding: utf-8
import sys
import os
import yaml
import importlib.util
import subprocess
import threading
from qclickableslider import QClickableSlider
from qimageviewarea import QImageViewArea

from PySide6.QtWidgets import QApplication, QMainWindow, QMessageBox, QLabel, QFileDialog
from PySide6.QtCore import QFile, QSettings, Signal, Slot
from PySide6.QtGui import QStandardItemModel, QStandardItem, QPixmap, QMovie
from PySide6.QtUiTools import QUiLoader


class hsmdebugger(QMainWindow):
    signalPlantumlDone = Signal(str)

    def __init__(self):
        super(hsmdebugger, self).__init__()

        self.hsmViewScaleFactor = 1.0
        self.threadPlantuml = None
        self.plantuml = None
        self.hsm = None
        self.lastDirectory = "~/"

        self.load_settings()

        (loadScxml2genStatus, errMsg) = self.loadScxml2genModule()
        if loadScxml2genStatus is False:
            QMessageBox.critical(None, "Error", f"Failed to load scxml2gen.py module.\n{errMsg}",
                                 buttons=QMessageBox.Ok)
            exit(1)

        self.load_ui()
        self.configure_ui()
        self.prepareFramesModel()
        self.configureActions()
        self.configureStatusBar()
        self.window.show()

    def onActionOpenHsm(self):
        scxmlPath = self.selectFile("Select HSM file", "SCXML files (*.scxml);;All files (*.*)")
        if scxmlPath:
            print("Loading: " + scxmlPath)
            self.unloadHsmLog()
            if self.loadScxml(scxmlPath) is False:
                QMessageBox.critical(QApplication.activeWindow(), "Error", "Failed to load HSM",
                                     buttons=QMessageBox.Ok)

    def onActionOpenHsmLog(self):
        if self.hsm:
            logPath = self.selectFile("Select HSM log file", "HSMCPP Log Files (*.hsmlog);;All files (*.*)")
            if logPath:
                if self.loadHsmLog(logPath):
                    self.updateFrames()
                    self.setCurrentFrameIndex(0)
                else:
                    QMessageBox.critical(QApplication.activeWindow(), "Error", "Failed to load HSM log",
                                         buttons=QMessageBox.Ok)

    def onActionSearch(self):
        if self.window.actionSearch.isChecked():
            self.window.searchFilter.show()
            self.window.searchFilter.setFocus()
        else:
            self.window.searchFilter.hide()

    def onActionPrevFrame(self):
        self.window.frameSelector.setValue(self.window.frameSelector.value() - 1)

    def onActionNextFrame(self):
        self.window.frameSelector.setValue(self.window.frameSelector.value() + 1)

    def onActionZoomIn(self):
        self.scaleView(self.hsmViewScaleFactor + 0.1)

    def onActionZoomOut(self):
        self.scaleView(self.hsmViewScaleFactor - 0.1)

    def onActionFitToView(self):
        imageSize = self.window.hsmStateView.pixmap().size()
        if imageSize.height() > 0:
            viewSize = self.window.hsmStateViewArea.size()
            margin = self.window.hsmStateViewAreaContents.layout().contentsMargins().top()
            margin += self.window.hsmStateViewAreaContents.layout().contentsMargins().bottom()
            self.scaleView((viewSize.height() - margin) / imageSize.height())
        else:
            self.scaleView(1.0)

    def onActionResetZoom(self):
        self.scaleView(1.0)

    def onActionSettings(self):
        print("TODO")

    def onActionGoToFrame(self):
        print("TODO")

    def onActionShowOnlineHelp(self):
        print("TODO")

    def onActionAbout(self):
        print("TODO")

    def onFrameSelected(self, currentIndex, previousIndex):
        self.setCurrentFrameIndex(currentIndex.row())

    def load_settings(self):
        settings = QSettings("./config.ini", QSettings.IniFormat)
        self.pathScxml2gen = settings.value("environment/scxml2gen", "../scxml2gen/scxml2gen.py")
        self.styleColors = {"active_state": settings.value("style/active_state", "38EB4B"),
                            "blocked_transition": settings.value("style/blocked_transition", "EB8421")}# FFAA00

    def load_ui(self):
        loader = QUiLoader()
        loader.registerCustomWidget(QClickableSlider)
        loader.registerCustomWidget(QImageViewArea)
        path = os.path.join(os.path.dirname(__file__), "form.ui")
        ui_file = QFile(path)
        ui_file.open(QFile.ReadOnly)
        self.window = loader.load(ui_file, self)
        ui_file.close()

    def configure_ui(self):
        self.window.hsmStateViewWait.setMovie( QMovie("./res/busy.gif") )
        self.window.hsmStateViewWait.movie().start()
        self.window.hsmStateViewWait.hide()
        self.window.frameSelector.setSingleStep(1)
        self.window.frameSelector.setPageStep(1)
        self.window.searchFilter.hide()
        self.window.hsmStateView.hide()

    def configureActions(self):
        self.window.actionOpenHsm.triggered.connect(self.onActionOpenHsm)
        self.window.actionOpenHsmLog.triggered.connect(self.onActionOpenHsmLog)
        self.window.actionSearch.triggered.connect(self.onActionSearch)
        self.window.actionPrevFrame.triggered.connect(self.onActionPrevFrame)
        self.window.actionNextFrame.triggered.connect(self.onActionNextFrame)
        self.window.actionZoomIn.triggered.connect(self.onActionZoomIn)
        self.window.actionZoomOut.triggered.connect(self.onActionZoomOut)
        self.window.actionResetZoom.triggered.connect(self.onActionResetZoom)
        self.window.actionFitToView.triggered.connect(self.onActionFitToView)
        self.window.actionSettings.triggered.connect(self.onActionSettings)
        self.window.actionGoToFrame.triggered.connect(self.onActionGoToFrame)
        self.window.actionShowOnlineHelp.triggered.connect(self.onActionShowOnlineHelp)
        self.window.actionAbout.triggered.connect(self.onActionAbout)

        self.window.frames.selectionModel().currentRowChanged.connect(self.onFrameSelected)
        self.signalPlantumlDone.connect(self.plantumlGenerationDone)
        self.enableHsmActions(False)
        self.enableLogActions(False)

    def enableHsmActions(self, enable):
        self.window.actionOpenHsmLog.setEnabled(enable)

    def enableLogActions(self, enable):
        self.window.actionSearch.setEnabled(enable)
        self.window.actionPrevFrame.setEnabled(enable)
        self.window.actionNextFrame.setEnabled(enable)
        self.window.actionZoomIn.setEnabled(enable)
        self.window.actionZoomOut.setEnabled(enable)
        self.window.actionResetZoom.setEnabled(enable)
        self.window.actionFitToView.setEnabled(enable)

    def configureStatusBar(self):
        self.statusBarFrame = QLabel("")
        self.statusBarLog = QLabel("")
        self.window.statusbar.addWidget(self.statusBarFrame)
        self.window.statusbar.addWidget(self.statusBarLog)

    def prepareFramesModel(self):
        self.modelFrames = QStandardItemModel()
        self.window.frames.setModel(self.modelFrames)
        self.window.frames.verticalHeader().hide()

    def scaleView(self, newScale):
        self.hsmViewScaleFactor = newScale
        self.window.hsmStateView.setFixedSize(self.hsmViewScaleFactor * self.window.hsmStateView.pixmap().size())

    def selectFile(self, title, filter):
        selectedPath = None
        dlgFileSelector = QFileDialog(QApplication.activeWindow(), title)
        dlgFileSelector.setOption(QFileDialog.DontUseNativeDialog)
        dlgFileSelector.setDirectory(self.lastDirectory)
        dlgFileSelector.setNameFilter(filter)
        if dlgFileSelector.exec_():
            selectedPath = dlgFileSelector.selectedFiles()[0]
            self.lastDirectory = os.path.dirname(selectedPath)
        return selectedPath

    def loadScxml2genModule(self):
        loaded = False
        msg = ""

        if os.path.exists(self.pathScxml2gen):
            try:
                spec = importlib.util.spec_from_file_location("", self.pathScxml2gen)
                self.scxml2gen = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(self.scxml2gen)
                loaded = True
            except:
                loaded = False
                msg = f"<{self.pathScxml2gen}> contains an error or has incorrect format"
        else:
            msg = f"<{self.pathScxml2gen}> wasn't found. Check that configuration file contains correct path."
        return (loaded, msg)

    def loadScxml(self, path):
        loaded = True
        self.enableHsmActions(False)
        try:
            self.hsm = self.scxml2gen.parseScxml(path)
            self.enableHsmActions(True)
        except:
            self.hsm = None
            loaded = False
        return loaded

    def unloadHsmLog(self):
        print("TODO")
        self.enableLogActions(False)

    def loadHsmLog(self, path):
        loaded = False
        self.enableLogActions(False)

        with open(path, 'r') as stream:
            try:
                self.hsmLog = yaml.safe_load(stream)
                self.enableLogActions(True)
                loaded = True
            except yaml.YAMLError as exc:
                print(exc)

        if loaded:
            self.currentFrameIndex = 0
            self.window.frameSelector.setMaximum(len(self.hsmLog))
        return loaded

    def threadPlantumlGeneration(self, format, destDirectory, srcFile):
        self.plantuml = subprocess.Popen(["plantuml", format, "-o", destDirectory, srcFile])
        rc = self.plantuml.wait()
        if rc == 0:
            (base, ext) = os.path.splitext(srcFile)
            self.signalPlantumlDone.emit(f"{destDirectory}/{os.path.basename(base)}.png")

    @Slot(str)
    def plantumlGenerationDone(self, path):
        self.plantuml = None
        self.threadPlantuml = None
        if len(path) > 0:
            self.window.hsmStateView.setPixmap(QPixmap(path))
            self.scaleView(self.hsmViewScaleFactor)
            self.window.hsmStateViewWait.hide()
            self.window.hsmStateView.show()

    def plantumlGeneratePng(self, src):
        if self.threadPlantuml:
            if self.plantuml:
                self.plantuml.kill()
                self.plantuml.wait()
                self.plantuml = None
            self.threadPlantuml.join()
        self.threadPlantuml = threading.Thread(target=self.threadPlantumlGeneration, args=("-tpng", "./gen", src,))
        self.threadPlantuml.start()

    def setCurrentFrameIndex(self, index):
        if index < len(self.hsmLog):
            self.updateHsmFrame(index)
            self.currentFrameIndex = index
            self.window.frameSelector.setValue(index)
            self.updateStatusBar()

    def updateFrames(self):
        self.modelFrames.clear()
        self.modelFrames.setHorizontalHeaderLabels(['ID', 'Timestamp', 'Action'])
        self.window.frames.setColumnWidth(0, 25)
        self.window.frames.setColumnWidth(1, 190)
        self.window.frames.setColumnWidth(2, 600)
        id = 1

        for entry in self.hsmLog:
            items = [QStandardItem(str(id)), QStandardItem(entry['timestamp']), QStandardItem(entry['action'])]
            self.modelFrames.appendRow(items)
            id += 1

    def updateStatusBar(self):
        self.statusBarFrame.setText(f"Frame {self.currentFrameIndex + 1} / {len(self.hsmLog)}")
        # TODO: update other fields

    def updateHsmFrame(self, index):
        if index < len(self.hsmLog):
            logEntry = self.hsmLog[index]
            highlight = {"active_states": [], "callback": {}, "transitions": {}}
            callbackStateId = None
            transitionStateId = None

            # --------------------
            # Active states
            if logEntry["active_states"]:
                for activeState in logEntry["active_states"]:
                    highlight["active_states"].append(activeState)

            # --------------------
            # Transitions
            if logEntry["action"] == "transition":
                transitionStateId = logEntry["target_state"]
                highlight["transitions"][transitionStateId] = {"from": logEntry["from_state"],
                                                                      "event": logEntry["event"]}
            elif logEntry["action"] == "transition_entrypoint":
                transitionStateId = logEntry["target_state"]
                highlight["transitions"][transitionStateId] = {"from": "",
                                                                      "event": logEntry["event"]}
            # --------------------
            # Callbacks
            elif logEntry["action"] == "callback_enter":
                callbackStateId = logEntry["target_state"]
                highlight["callback"][callbackStateId] = {"onentry": logEntry['status'] != "failed"}
            elif logEntry["action"] == "callback_state":
                callbackStateId = logEntry["target_state"]
                highlight["callback"][callbackStateId] = {"onstate": True}
            elif logEntry["action"] == "callback_exit":
                callbackStateId = logEntry["from_state"]
                highlight["callback"][callbackStateId] = {"onexit": logEntry['status'] != "failed"}

            # Store args if needed
            if "args" in logEntry:
                if transitionStateId:
                    highlight["transitions"][transitionStateId]["args"] = logEntry["args"]
                elif callbackStateId:
                    highlight["callback"][callbackStateId]["args"] = logEntry["args"]

            highlight["style"] = self.styleColors

            self.window.hsmStateView.hide()
            self.window.hsmStateViewWait.show()
            self.scxml2gen.generatePlantuml(self.hsm, "./test.plantuml", highlight)
            self.plantumlGeneratePng("./test.plantuml")


if __name__ == "__main__":
    app = QApplication([])
    main = hsmdebugger()
    sys.exit(app.exec_())
