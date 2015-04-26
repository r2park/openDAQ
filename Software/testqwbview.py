import sys
import platform
from PyQt4.QtGui import QApplication
from PyQt4.QtWebKit import QWebPage

class Render(QWebPage):
    def __init__(self):
        self.app = QApplication([])
        QWebPage.__init__(self)

    @property
    def html(self):
        return self.mainFrame().toHtml().toAscii()

page = Render()
print sys.version, platform.platform()
print 'html attribute?', [p for p in dir(page) if 'html' in p]
print page.html