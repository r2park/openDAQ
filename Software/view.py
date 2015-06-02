from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtWebKit import *
import math as mth
import pyqtgraph as pg
import numpy as np
import csv
import sys
from GChartWrapper import *


class main_view(QWidget):
    def __init__(self):
        QDialog.__init__(self)
        layout = QGridLayout()
        self.setWindowTitle("openDAQ")
        impbutton = QPushButton("Import log")
        showgraphbtn = QPushButton("show graphs")
        clsbutton = QPushButton("Close")
        self.tbwidget = QTableWidget()
        self.tbwidget.setMinimumHeight(100)
        self.tbwidget.setMinimumSize(400,400)
        ScaleUpbtn = QPushButton("Zoom in")
        ScaleDownbtn = QPushButton("Zoom Out")
        self.FullScreen = QPushButton("Full screen")

        #This is what I want but will not work....
        self.scene = QGraphicsScene()

        self.view = QGraphicsView()
        self.view.setMouseTracking(True)
        self.view.setScene(self.scene)
        self.graphItem = pg.GraphItem()
        self.scene.addItem(self.graphItem)
        self.view.setMinimumSize(400, 400)


        layout.addWidget(self.view, 0, 0)
        layout.addWidget(self.tbwidget, 0, 1)
        layout.addWidget(clsbutton, 1, 0)
        layout.addWidget(impbutton, 2, 0)
        layout.addWidget(showgraphbtn, 1, 1)

        layout.addWidget(ScaleUpbtn, 3, 0)
        layout.addWidget(ScaleDownbtn, 3, 1)
        layout.addWidget(self.FullScreen, 2, 1)
        self.setLayout(layout)

        clsbutton.clicked.connect(self.close)
        impbutton.clicked.connect(self.import_data)
        showgraphbtn.clicked.connect(self.OpenCharts)
        ScaleUpbtn.clicked.connect(self.ScaleViewUp)
        ScaleDownbtn.clicked.connect(self.ScaleViewDown)
        self.FullScreen.clicked.connect(self.fullscreen)
        self.ViewScale = 1
        self.view.scale(self.ViewScale, self.ViewScale)


    def fullscreen(self):

        # I have to see how to minimze and maximize works properly fully.
        if self.windowState() & Qt.WindowFullScreen():
            self.showNormal()
            self.FullScreen.setText("Full screen")
        else:
            self.showFullScreen()
            self.FullScreen.setText("Minimize")


    def OpenCharts(self):
        chartDialog = ShowChart()
        if chartDialog.exec_():
            print("Correct for now")


    def import_data(self):
        path = '.'
        self.Opdilog = QFileDialog.getOpenFileName(self, "Select the file to import", directory=path,
                                                   filter="CSV files (*.csv)")
        f = open(self.Opdilog)
        rcount = open(self.Opdilog)

        plt = open(self.Opdilog)
        readcsv = csv.reader(f)
        readcsvcount = csv.reader(rcount)
        readforplot = csv.reader(plt)

        headerNames = ["time", "latitude", "longitude", "Speed", "Speed 2", "Engin RPM", "Break Pedal",
                       "Accelerator pedal", "Steering wheel", "X axis g-force", "Y axis g-force", "Z axis g-force","Tire Temp inside","Tire temp Middle","Tire Temp Outside"]
        self.tbwidget.setColumnCount(len(headerNames))
        self.tbwidget.setRowCount(sum(1 for rows in readcsvcount))
        self.tbwidget.setHorizontalHeaderLabels(headerNames)
        timelist = []
        speedlist = []
        speedlist2 = []
        rpmlist = []
        self.tiretempInside =  []
        tireTempMiddle = []
        tireTempOutside = []
        breakPedal = []
        rowcount = 0
        column = 0
        ## Define positions of nodes
        pos = []
        for row in readcsv:
            try:
                timelist.append(float(row[0]))
                speedlist.append(float(row[3]))
                speedlist2.append(float(row[4]))
                rpmlist.append(float(row[5]))
            except:
                print("Csv file is not right!!")
            try:
                self.tiretempInside.append(float(row[12]))
                tireTempMiddle.append(float(row[13]))
                tireTempOutside.append(float(row[14]))
            except:
                print("Does not have print info")
            breakPedal.append(float(row[6]))
            ##Earth raduis in metters
            R = 6371000
            pos.append([float(R * mth.cos(float(row[1]))), float(R * mth.cos(float(row[2])))])
            while column < len(headerNames):
                try:
                    item = QTableWidgetItem(row[column])
                    item.setFlags(Qt.ItemIsEnabled)
                    self.tbwidget.setItem(rowcount, column, item)
                    column = column + 1
                except:
                    print("Does not have all the columns")
            rowcount = rowcount + 1
            column = 0
        ## Update the graph
        pen = QPen()
        pen.setStyle(Qt.SolidLine)
        pen.setWidth(3)
        self.graphItem.setData(pos=np.array(pos), pen=pen, pxMode=False)
        speedPlot = pg.plot(x = timelist, y= speedlist,title= "Vehicles speed"  ,pen=(255,255,255,200))
        speedPlot.plot(x = timelist, y= speedlist2 ,pen=(255,0,0,200))
        pmPlot = pg.plot(x = timelist, y= rpmlist,title= "Vehicles RPM",pen=(255,0,0,200))
        self.tireTempPlot    = pg.plot(x=timelist ,  y= self.tiretempInside ,pen = (255,0,0,200),title="Tire temperature")
        self.tireTempPlot.plot(x=timelist ,  y= tireTempMiddle ,pen = (0,255,0,200))
        self.tireTempPlot.plot(x=timelist,y=tireTempOutside ,pen = (0,0,255,200) )
        self.vLine = pg.InfiniteLine(angle=90, movable=False)
        self.hLine = pg.InfiniteLine(angle=0, movable=False)
        self.tireTempPlot.addItem(self.vLine,ignoreBounds =True)
        self.tireTempPlot.addItem(self.hLine,ignoreBounds =True)
        #self.tireTempPlot.mouseMoveEvent.connect(self.MousePlotPos)
        breakPedalPlot  = pg.plot(x=timelist ,  y= breakPedal ,title="break pedal position")

        self.view.fitInView(self.scene.sceneRect(), Qt.KeepAspectRatio)

        # use this to zero out plane cordinaes
        print(self.scene.sceneRect())

    def MousePlotPos(self,evt):
        pos = evt[0]  ## using signal proxy turns original arguments into a tuple
        if self.tireTempPlot.sceneBoundingRect().contains(pos):
            mousePoint = self.tireTempPlot.mapSceneToView(pos)
            index = int(mousePoint.x())
            #if index > 0 and index < len(self.tiretempInside):
            #label.setText("<span style='font-size: 12pt'>x=%0.1f,   <span style='color: red'>y1=%0.1f</span>,   <span style='color: green'>y2=%0.1f</span>" % (mousePoint.x(), data1[index], data2[index]))
            self.vLine.setPos(mousePoint.x())
            self.hLine.setPos(mousePoint.y())
    def ScaleViewUp(self):'''
        self.ViewScale += 1
        self.view.scale(self.ViewScale, self.ViewScale)'''


    def ScaleViewDown(self):
        print("in correct function")
        if self.ViewScale > 1:
            self.ViewScale -= 1
            self.view.scale(self.ViewScale, self.ViewScale)


class QGraphicsScene(QGraphicsScene):
    def mouseMoveEvent(self, QMouseEvent):
        #I don
        '''
        cursourpos = QCursor.pos()
        print(cursourpos)
        '''
class ShowChart(QDialog):
    def __init__(self):
        QDialog.__init__(self)
        self.setWindowTitle("New dialog")
        btnClose = QPushButton("Close")
        btnClose.clicked.connect(self.reject)
        layout = QGridLayout()
        layout.addWidget(btnClose, 0, 0)
        self.setLayout(layout)


app = QApplication(sys.argv)
dialog = main_view()
dialog.show()
app.exec_()



