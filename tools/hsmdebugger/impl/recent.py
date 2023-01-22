# Copyright (C) 2021 Igor Krechetov
# Distributed under MIT license. See file LICENSE for details

from PySide6.QtCore import QSettings
from PySide6.QtGui import QAction

class RecentFilesManager():
    def __init__(self, configPath, menuRecentHsm, menuRecentLogs, callbackOpenRecentHSM, callbackOpenRecentLog):
        self.configPath = configPath
        self.maxEntries = 10
        self.menuRecentHsm = menuRecentHsm
        self.menuRecentLogs = menuRecentLogs
        self.callbackOpenRecentHSM = callbackOpenRecentHSM
        self.callbackOpenRecentLog = callbackOpenRecentLog
        self.recentHsmFiles = []
        self.recentLogFiles = []
        self.loadRecentItems()
        self.updateMenuRecentHsm()
        self.updateMenuRecentLogs()

    def loadRecentItems(self):
        settings = QSettings(self.configPath, QSettings.IniFormat)
        settings.beginGroup("recent_hsm")
        recentItems = settings.childKeys()
        for itemId in recentItems:
            self.recentHsmFiles.append(settings.value(itemId))
        settings.endGroup()
        settings.beginGroup("recent_logs")
        recentItems = settings.childKeys()
        for itemId in recentItems:
            self.recentLogFiles.append(settings.value(itemId))
        settings.endGroup()

    def onActionClearRecentHSM(self):
        settings = QSettings(self.configPath, QSettings.IniFormat)
        settings.remove("recent_hsm")
        settings.sync()
        self.recentHsmFiles.clear()
        self.updateMenuRecentHsm()

    def onActionClearRecentLogs(self):
        settings = QSettings(self.configPath, QSettings.IniFormat)
        settings.remove("recent_logs")
        settings.sync()
        self.recentLogFiles.clear()
        self.updateMenuRecentLogs()

    def addPathToList(self, newPath, settingsGroup, items, updateFunc):
        settings = QSettings(self.configPath, QSettings.IniFormat)
        settings.beginGroup(settingsGroup)
        recentItems = settings.childKeys()
        items.clear()
        items.append(newPath)
        for itemId in recentItems:
            itemPath = settings.value(itemId)
            if itemPath != newPath:
                items.append(settings.value(itemId))
                if len(items) >= self.maxEntries:
                    break
        settings.endGroup()
        settings.remove(settingsGroup)
        index = 1
        for curItem in items:
            settings.setValue(f"{settingsGroup}/item{index}", curItem)
            index += 1
        settings.sync()
        updateFunc()

    def addPathToRecentHSMList(self, path):
        self.addPathToList(path, "recent_hsm", self.recentHsmFiles, self.updateMenuRecentHsm)

    def addPathToRecentLogsList(self, path):
        self.addPathToList(path, "recent_logs", self.recentLogFiles, self.updateMenuRecentLogs)

    def updateMenuItems(self, menu, items, callbackItem, callbackClear):
        menu.clear()
        if len(items) > 0:
            menu.setEnabled(True)
            for curPath in items:
                entryAction = QAction(curPath, menu)
                entryAction.triggered.connect(callbackItem)
                menu.addAction(entryAction)
            menu.addSeparator()
            menu.addAction("Clear Menu").triggered.connect(callbackClear)
        else:
            menu.setEnabled(False)

    def updateMenuRecentHsm(self):
        self.updateMenuItems(self.menuRecentHsm,
                             self.recentHsmFiles,
                             self.callbackOpenRecentHSM,
                             self.onActionClearRecentHSM)

    def updateMenuRecentLogs(self):
        self.updateMenuItems(self.menuRecentLogs,
                             self.recentLogFiles,
                             self.callbackOpenRecentLog,
                             self.onActionClearRecentLogs)
