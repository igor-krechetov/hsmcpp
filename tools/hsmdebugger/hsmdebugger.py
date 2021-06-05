# Copyright (C) 2021 Igor Krechetov
# Distributed under MIT license. See file LICENSE for details

# This Python file uses the following encoding: utf-8
import sys
import os
import yaml
import importlib.util
import subprocess
import threading
import hashlib
from pathlib import Path

from impl.qclickableslider import QClickableSlider
from impl.qimageviewarea import QImageViewArea
from impl.recent import RecentFilesManager
from impl.search import QFramesSearchModel
from impl.utils import utils
from impl.settings import QSettingsDialog

from PySide6.QtCore import Qt
from PySide6.QtWidgets import QApplication, QMessageBox, QLabel, QInputDialog
from PySide6.QtCore import QFile, Signal, Slot, QObject
from PySide6.QtGui import QStandardItemModel, QStandardItem, QPixmap, QMovie, QIcon
from PySide6.QtUiTools import QUiLoader


class hsmdebugger(QObject):
    signalPlantumlDone = Signal(str, str)
    appTitle = "HSM Debugger"
    appDir = Path(os.path.dirname(__file__))
    configPath = appDir / "config.ini"

    def __init__(self):
        super(hsmdebugger, self).__init__()
        self.threadPlantuml = None
        self.plantuml = None
        self.hsm = None
        self.hsmLog = None
        if sys.platform == "win32":
            self.lastDirectory = ""
        else:
            self.lastDirectory = "~/"
        self.currentScxmlPath = None
        self.currentLogPath = None
        self.hsmViewScaleFactor = 1.0
        self.currentFrameIndex = None

        self.load_settings()
        (loadScxml2genStatus, errMsg) = self.loadScxml2genModule()
        if loadScxml2genStatus is False:
            QMessageBox.critical(None, "Error", f"Failed to load scxml2gen.py module.\n{errMsg}",
                                 buttons=QMessageBox.Ok)
            exit(1)

        self.load_ui()
        self.configure_ui()
        self.recentFiles = RecentFilesManager(str(self.configPath),
                                              self.window.menuFileRecentHSM,
                                              self.window.menuFileRecentLogs,
                                              self.onActionOpenRecentHSM,
                                              self.onActionOpenRecentLog)
        self.prepareFramesModel()
        self.configureActions()
        self.configureStatusBar()
        self.window.show()
        if self.settings.loadLastHSM is True:
            if len(self.recentFiles.recentHsmFiles) > 0:
                self.loadScxml(self.recentFiles.recentHsmFiles[0])

    def onActionOpenHsm(self):
        (scxmlPath, lastDir) = utils.selectFile("Select HSM file", "SCXML files (*.scxml);;All files (*)", self.lastDirectory)
        if scxmlPath:
            self.lastDirectory = lastDir
            self.loadScxml(scxmlPath)

    def onActionOpenHsmLog(self):
        if self.hsm:
            (logPath, lastDir) = utils.selectFile("Select HSM log file", "HSMCPP Log Files (*.hsmlog);;All files (*)", self.lastDirectory)
            if logPath:
                self.lastDirectory = lastDir
                self.loadHsmLog(logPath)

    def onActionOpenRecentHSM(self):
        self.loadScxml(self.sender().text())

    def onActionOpenRecentLog(self):
        self.loadHsmLog(self.sender().text())

    def onActionSearch(self):
        if self.window.actionSearch.isChecked():
            self.window.searchFilter.show()
            self.window.searchFilter.setFocus()
            self.modelFrames.enableFilter()
        else:
            self.window.searchFilter.hide()
            self.modelFrames.disableFilter()

    def onActionShowFrames(self):
        if self.window.actionShowFrames.isChecked():
            self.window.frames.show()
        else:
            self.window.frames.hide()

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
        if self.settings.show():
            # TODO: only needed if colors changed
            self.updateHsmFrame(self.currentFrameIndex)

    def onActionGoToFrame(self):
        if self.hsmLog:
            (newIndex, wasOk) = QInputDialog.getInt(QApplication.activeWindow(),
                                                    "Go to Frame",
                                                    f"Frame index (1 ~ {len(self.hsmLog) + 1}):",
                                                    value=self.currentFrameIndex + 1,
                                                    minValue=1, maxValue=len(self.hsmLog) + 1)
            if wasOk:
                self.setCurrentFrameIndex(newIndex - 1)

    def onActionShowOnlineHelp(self):
        print("TODO")

    def onActionAbout(self):
        print("TODO")

    def onFrameSelected(self, currentIndex, previousIndex):
        if currentIndex.row() >= 0:
            indexId = self.modelFrames.index(currentIndex.row(), 0)
            newFrameIndex = int(self.modelFrames.data(indexId)) - 1
            self.setCurrentFrameIndex(newFrameIndex)

    def onFrameSelectorUpdated(self, newFrameIndex):
        self.setCurrentFrameIndex(newFrameIndex)

    def onSearchFilterChanged(self, newFilter):
        self.modelFrames.setFilter(newFilter)

    def load_settings(self):
        self.settings = QSettingsDialog(str(self.configPath))

    def load_ui(self):
        loader = QUiLoader()
        loader.registerCustomWidget(QClickableSlider)
        loader.registerCustomWidget(QImageViewArea)
        path = str(self.appDir / "ui" / "form.ui")
        ui_file = QFile(path)
        ui_file.open(QFile.ReadOnly)
        self.window = loader.load(ui_file)
        ui_file.close()

    def configure_ui(self):
        self.window.hsmStateViewWait.setMovie(QMovie(str(self.appDir / "res" / "busy.gif")))
        self.window.hsmStateViewWait.movie().start()
        self.window.hsmStateViewWait.hide()
        self.window.frameSelector.setSingleStep(1)
        self.window.frameSelector.setPageStep(1)
        self.window.frameSelector.setMaximum(0)
        self.window.searchFilter.hide()
        self.window.hsmStateView.hide()
        self.window.hsmStateView.setAttribute(Qt.WA_TranslucentBackground)

    def configureActions(self):
        self.window.actionOpenHsm.triggered.connect(self.onActionOpenHsm)
        self.window.actionOpenHsmLog.triggered.connect(self.onActionOpenHsmLog)
        self.window.actionSearch.triggered.connect(self.onActionSearch)
        self.window.actionShowFrames.triggered.connect(self.onActionShowFrames)
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

        self.window.searchFilter.textChanged.connect(self.onSearchFilterChanged)
        self.window.frames.selectionModel().currentRowChanged.connect(self.onFrameSelected)
        self.window.frameSelector.valueChanged.connect(self.onFrameSelectorUpdated)
        self.signalPlantumlDone.connect(self.plantumlGenerationDone)
        self.enableHsmActions(False)
        self.enableLogActions(False)

    def enableHsmActions(self, enable):
        self.window.actionOpenHsmLog.setEnabled(enable)
        if len(self.recentFiles.recentLogFiles) > 0:
            self.window.menuFileRecentLogs.setEnabled(enable)
        else:
            self.window.menuFileRecentLogs.setEnabled(False)

    def enableLogActions(self, enable):
        self.window.actionSearch.setEnabled(enable)
        self.window.actionPrevFrame.setEnabled(enable)
        self.window.actionNextFrame.setEnabled(enable)
        self.window.actionZoomIn.setEnabled(enable)
        self.window.actionZoomOut.setEnabled(enable)
        self.window.actionResetZoom.setEnabled(enable)
        self.window.actionFitToView.setEnabled(enable)
        self.window.actionGoToFrame.setEnabled(enable)

    def configureStatusBar(self):
        self.statusBarFrame = QLabel("")
        self.statusBarLog = QLabel("")
        self.window.statusbar.addPermanentWidget(self.statusBarLog, 9999)
        self.window.statusbar.addPermanentWidget(self.statusBarFrame, 0)

    def prepareFramesModel(self):
        self.modelFrames = QFramesSearchModel()
        self.modelFrames.setSourceModel(QStandardItemModel())
        self.window.frames.setModel(self.modelFrames)
        self.window.frames.verticalHeader().hide()

    def scaleView(self, newScale):
        if self.hsmLog:
            self.hsmViewScaleFactor = newScale
            self.window.hsmStateView.setFixedSize(self.hsmViewScaleFactor * self.window.hsmStateView.pixmap().size())

    def loadScxml2genModule(self):
        loaded = False
        msg = ""
        if os.path.exists(self.settings.pathScxml2gen):
            try:
                spec = importlib.util.spec_from_file_location("", self.settings.pathScxml2gen)
                self.scxml2gen = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(self.scxml2gen)
                loaded = True
            except:
                loaded = False
                msg = f"<{self.settings.pathScxml2gen}> contains an error or has incorrect format"
        else:
            msg = f"<{self.settings.pathScxml2gen}> wasn't found. Check that configuration file contains correct path."
        return (loaded, msg)

    def loadScxml(self, path):
        loaded = True
        if path and len(path) > 0:
            self.unloadHsmLog()
            self.enableHsmActions(False)
            try:
                self.hsm = self.scxml2gen.parseScxml(path)
            except:
                self.hsm = None
                loaded = False
                QMessageBox.critical(QApplication.activeWindow(), "Error", "Failed to load HSM",
                                     buttons=QMessageBox.Ok)

            if loaded:
                self.hsmId = self.getDataChecksum(str(self.hsm))
                self.currentScxmlPath = path
                self.recentFiles.addPathToRecentHSMList(path)
                self.enableHsmActions(True)
                self.updateStatusBar()
        else:
            loaded = False
        return loaded

    def unloadHsmLog(self):
        print("TODO")
        self.enableLogActions(False)

    def loadHsmLog(self, path):
        loaded = False

        if path and len(path) > 0:
            self.enableLogActions(False)
            with open(path, 'r') as stream:
                try:
                    self.hsmLog = yaml.safe_load(stream)
                    self.enableLogActions(True)
                    loaded = True
                except yaml.YAMLError as exc:
                    QMessageBox.critical(QApplication.activeWindow(), "Error", f"Failed to load HSM log:\n{exc}",
                                         buttons=QMessageBox.Ok)
            if loaded:
                self.currentLogPath = path
                self.recentFiles.addPathToRecentLogsList(path)
                self.updateFrames()
                self.setCurrentFrameIndex(0)
        return loaded

    def threadPlantumlGeneration(self, format, destDirectory, srcFile):
        if self.settings.pathPlantuml.endswith(".jar"):
            argsPlantUml = ["java", "-jar", self.settings.pathPlantuml]
        else:
            argsPlantUml = [self.settings.pathPlantuml]

        try:
            self.plantuml = subprocess.Popen(argsPlantUml + [format, "-o", Path("./"), srcFile])
        except FileNotFoundError:
            self.signalPlantumlDone.emit("", "not_found")

        if self.plantuml:
            rc = self.plantuml.wait()
            if rc == 0:
                (base, ext) = os.path.splitext(srcFile)
                self.signalPlantumlDone.emit(str(Path(destDirectory) / f"{os.path.basename(base)}.png"), "ok")
            else:
                self.signalPlantumlDone.emit("", "cancelled")

    @Slot(str)
    def plantumlGenerationDone(self, path, status):
        self.plantuml = None
        self.threadPlantuml = None
        if len(path) > 0:
            if len(path) > 0:
                self.window.hsmStateView.setPixmap(QPixmap(path))
                self.scaleView(self.hsmViewScaleFactor)
                self.window.hsmStateViewWait.hide()
                self.window.hsmStateView.show()
        elif status == "not_found":
            self.window.hsmStateView.setPixmap(QPixmap(str(self.appDir / "res" / "failed.png")))
            self.scaleView(0.5)
            self.window.hsmStateViewWait.hide()
            self.window.hsmStateView.show()
            QMessageBox.critical(None, "Error",
                                 f"Failed to run Plantuml. Check that path to binary is correctly specified",
                                 buttons=QMessageBox.Ok)

    def plantumlGeneratePng(self, src, outDir):
        if self.threadPlantuml:
            if self.plantuml:
                self.plantuml.kill()
                self.plantuml.wait()
                self.plantuml = None
            self.threadPlantuml.join()
        self.threadPlantuml = threading.Thread(target=self.threadPlantumlGeneration, args=("-tpng", outDir, src,))
        self.threadPlantuml.start()

    def setCurrentFrameIndex(self, index):
        if (self.currentFrameIndex != index) and (index < len(self.hsmLog)):
            self.updateHsmFrame(index)
            self.currentFrameIndex = index
            self.window.frameSelector.setValue(index)
            self.updateStatusBar()

    def updateFrames(self):
        self.modelFrames.sourceModel().clear()
        if self.hsmLog:
            self.window.frameSelector.setMaximum(len(self.hsmLog) - 1)
            self.modelFrames.sourceModel().setHorizontalHeaderLabels(['ID', 'Timestamp', 'Action', 'Arguments'])
            self.window.frames.setColumnWidth(0, 25)
            self.window.frames.setColumnWidth(1, 190)
            self.window.frames.setColumnWidth(2, 250)
            self.window.frames.setColumnWidth(3, 600)
            id = 1
            for entry in self.hsmLog:
                entryArgs = ""
                if ('args' in entry) and (entry['args'] is not None):
                    entryArgs = str(entry['args'])
                items = [QStandardItem(str(id)), QStandardItem(entry['timestamp']), QStandardItem(entry['action']),
                         QStandardItem(entryArgs)]
                self.modelFrames.sourceModel().appendRow(items)
                id += 1

    def updateStatusBar(self):
        if self.hsmLog:
            self.statusBarFrame.setText(f"Frame {self.currentFrameIndex + 1} / {len(self.hsmLog)}")

        if self.currentScxmlPath:
            self.window.setWindowTitle(f"{self.appTitle}  \u2014  [{os.path.basename(self.currentScxmlPath)}]")
        else:
            self.window.setWindowTitle(self.appTitle)

        if self.currentLogPath:
            self.statusBarLog.setText(self.currentLogPath)
        else:
            self.statusBarLog.setText("")

    def updateHsmFrame(self, index):
        if (self.hsmLog is not None) and (index < len(self.hsmLog)):
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

            highlight["style"] = self.settings.styleColors

            self.window.hsmStateView.hide()
            self.window.hsmStateViewWait.show()
            self.generateHsmFrameImage(index, highlight)

    def generateHsmFrameImage(self, index, highlight):
        dirCacheRoot = Path("./cache")
        dirHsmCache = dirCacheRoot / self.hsmId

        if os.path.exists(dirCacheRoot) is False:
            os.mkdir(dirCacheRoot)
        if os.path.exists(dirHsmCache) is False:
            os.mkdir(dirHsmCache)

        plantumlContent = self.scxml2gen.generatePlantumlInMemory(self.hsm, highlight)
        plantumlChecksumNew = self.getDataChecksum(plantumlContent)
        pathPlantumlFile = str(dirHsmCache / f"{plantumlChecksumNew}.plantuml")
        pathFrameImage = str(dirHsmCache / f"{plantumlChecksumNew}.png")

        if (os.path.exists(pathFrameImage) is False) or (os.path.getsize(pathFrameImage) == 0):
            print("Generate image...")
            self.createFile(pathPlantumlFile, plantumlContent)
            self.plantumlGeneratePng(pathPlantumlFile, dirHsmCache)
        else:
            print(f"Use existing image: {pathFrameImage}")
            self.signalPlantumlDone.emit(pathFrameImage, "ok")

    def getFileChecksum(self, path):
        hash_md5 = hashlib.md5()
        with open(path, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()

    def getDataChecksum(self, data):
        hash_md5 = hashlib.md5()
        hash_md5.update(data.encode('utf-8'))
        return hash_md5.hexdigest()

    def readFile(self, path):
        content = None
        try:
            with open(path, "r") as f:
                content = f.read()
        except FileNotFoundError:
            content = None
        return content

    def createFile(self, path, content):
        with open(path, "w") as f:
            f.write(content)


if __name__ == "__main__":
    if sys.platform == 'win32':
        # NOTE: This is needed to display the app icon on the taskbar on Windows 7
        import ctypes
        myappid = 'igorkrechetov.hsmdebugger'
        ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(myappid)

    app = QApplication(sys.argv)
    main = hsmdebugger()
    app.setWindowIcon(QIcon(str(Path("./res/hsmdebugger.ico"))))
    sys.exit(app.exec_())
