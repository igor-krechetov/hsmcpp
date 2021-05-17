# Copyright (C) 2021 Igor Krechetov
# Distributed under MIT license. See file LICENSE for details

import re
from PySide6.QtCore import QSortFilterProxyModel


class QFramesSearchModel(QSortFilterProxyModel):
    filterAction = ""
    filterArgs = ""
    regexAction = None
    regexArgs = None
    disableFiltering = True

    def disableFilter(self):
        self.disableFiltering = True
        self.invalidateFilter()

    def enableFilter(self):
        self.disableFiltering = False
        self.invalidateFilter()

    def setFilter(self, newFilter):
        if len(newFilter) > 0:
            posSeparator = newFilter.find(":")
            if posSeparator >= 0:
                self.filterAction = newFilter[:posSeparator]
                self.filterArgs = newFilter[posSeparator + 1:]
                self.regexArgs = re.compile(f".*{self.filterArgs}.*")
            else:
                self.filterAction = newFilter
                self.regexArgs = None

            self.regexAction = re.compile(f".*{self.filterAction}.*")
        else:
            self.filterAction = ""
            self.filterArgs = ""
            self.regexAction = None
            self.regexArgs = None
        self.invalidateFilter()

    def filterAcceptsRow(self, source_row, source_parent):
        accept = True
        if (self.disableFiltering is False) and (self.regexAction is not None):
            operationIndex = self.sourceModel().index(source_row, 2, source_parent)
            isCorrectAction = self.regexAction.match(self.sourceModel().data(operationIndex))
            isCorrectArgs = True

            if isCorrectAction and (self.regexArgs is not None):
                argsIndex = self.sourceModel().index(source_row, 3, source_parent)
                isCorrectArgs = self.regexArgs.match(self.sourceModel().data(argsIndex))
            accept = (isCorrectAction is not None) and (isCorrectArgs is not None)

        return accept

