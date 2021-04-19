# Copyright (C) 2021 Igor Krechetov
# Distributed under MIT license. See file LICENSE for details

from PySide6.QtCore import Qt
from PySide6.QtWidgets import QScrollArea


class QImageViewArea(QScrollArea):
    isDragging = False

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.setCursor(Qt.CursorShape.OpenHandCursor)
            self.prevMousePosition = event.pos()
            self.isDragging = True
        else:
            super(QImageViewArea, self).mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.setCursor(Qt.CursorShape.ArrowCursor)
            self.isDragging = False
        else:
            super(QImageViewArea, self).mouseReleaseEvent(event)

    def mouseMoveEvent(self, event):
        super(QImageViewArea, self).mouseMoveEvent(event)
        if self.isDragging:
            delta = event.pos() - self.prevMousePosition
            self.prevMousePosition = event.pos()
            self.horizontalScrollBar().setValue(self.horizontalScrollBar().value() - delta.x())
            self.verticalScrollBar().setValue(self.verticalScrollBar().value() - delta.y())
