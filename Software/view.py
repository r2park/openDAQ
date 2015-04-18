from PyQt4.QtCore import *
from PyQt4.QtGui import *
import sys


class HelloWorld(QDialog):
    def __init__(self):
        QDialog.__init__(self)
        layout = QGridLayout()
        label = QLabel("Hello world")
        lineEdit = QLineEdit()
        button = QPushButton("Close")
        layout.addWidget(label,0,0)
        layout.addWidget(lineEdit,0,1)
        layout.addWidget(button,1,1)
        self.setLayout(layout)

        button.clicked.connect(self.close)
        lineEdit.textChanged.connect(label.setText)


app = QApplication(sys.argv)
dialog = HelloWorld()
dialog.show()
app.exec_()



