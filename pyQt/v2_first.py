#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys, serial, time, threading
from datetime import datetime

try:
    import Queue
except:
    import queue as Queue

from PyQt5.QtCore import Qt, QThread, pyqtSignal
from PyQt5.QtGui import QIcon, QTextCursor
from PyQt5.QtWidgets import (QMainWindow, QApplication, QAction, QWidget,
    qApp, QPushButton, QDesktopWidget, QHBoxLayout, QVBoxLayout, QLabel, QLineEdit, QTextEdit, QCheckBox )

# Globals
WIN_WITH, WIN_HEIGHT = 684, 400     # Window size
SER_TIMEOUT = 0.1                   # Time out for serial RX
RETURN_CHAR = "\n"                  # Char to be sent when Enter key pressed
PASTE_CHAR = "\x16"                 # Ctrl code for clipboard paste
baudrate = 9600                   # Default boudrate
portname = "/dev/ttyUSB0"#"/dev/ttyACM0"                # Default port name
hexmode = False                     # Flag to enable hex dispaly

# find available ports
# https://github.com/mcagriaksoy/Serial-Communication-GUI-Program
#ports = [
#    p.device
#    for p in serial.tools.list_ports.comports()
#    if 'USB' in p.description
#]



# Convert a string to bytes
def str_bytes(s):
    return s.encode('latin-1')

# Convert bytes to string
def bytes_str(d):
    return d if type(d) is str else "".join([chr(b) for b in d])

# Return hexadecimal values of data
def hexdump(data):
    return " ".join(["%02X" % ord(b) for b in data])

# Return a string with high-bit chars replaced by hex values
def textdump(data):
    return "".join(["[%02X]" % ord(b) if b>'\x7e' else b for b in data])

# Display incoming serial data
def display(s):
    if not hexmode:
        sys.stdout.write(textdump(str(s)) + '\n')
    else:
        sys.stdout.write(hexdump(s) + ' ' + '\n')

# Custom text box, catching keystrokes
class MyTextBox(QTextEdit):
    def __init__(self, *args):
        QTextEdit.__init__(self, *args)

    def keyPressEvent(self, event):     # Send keypress to parent's handler
        print("you type - %s\n", event)
        #self.parentWidget().keypress_handler(event)

# Main window class
class myWindow(QMainWindow):
    text_update = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        #https://stackoverflow.com/questions/49494759/qmainwindow-vs-qwidget
        self.central_widget = QWidget()               # define central widget
        self.setCentralWidget(self.central_widget)    # set QMainWindow.centralWidget

        # Action definition
        exitAction = QAction(QIcon('exit.png'), '&Exit', self)
        exitAction.setShortcut('Ctrl+Q')
        exitAction.setStatusTip('Exit application')
        exitAction.triggered.connect(qApp.quit)

        connectAction = QAction(QIcon('connect.png'), '&Connect', self)
        connectAction.setStatusTip('Connect to target')

        # Tools bar defifnition
        self.toolbar = self.addToolBar('Exit')
        self.toolbar.addAction(exitAction)

        self.toolbar = self.addToolBar('Connect')
        self.toolbar.addAction(connectAction)

        # Status bar
        self.statusBar()
        self.statusBar().showMessage('Ready')

        # Menubar
        menubar = self.menuBar()
        fileMenu = menubar.addMenu('&File')
        fileMenu.addAction(exitAction)

        connectMenu = menubar.addMenu('&Connect')
        connectMenu.addAction(connectAction)

        # Controls (buttons, checkboxes)
        serialPortL = QLabel("Serial Port")
        self.serialPort = QLineEdit(str(portname))
        serialBaudRateL = QLabel("Baud Rate")
        self.serialBaudRate = QLineEdit(str(baudrate))

        self.hexMode = QCheckBox("HEX Mode", self)
        #self.hexMode.toggle()
        self.hexMode.stateChanged.connect(self.changeHexMode)

        syncButton = QPushButton("Sync time", self)
        getTimeButton = QPushButton("Get time", self)
        currentTimeL = QLabel("Current time")
        self.currentTime = QLineEdit()
        self.currentTime.setFixedWidth(80)
        targetTimeL = QLabel("Target time")
        self.targretTime = QLineEdit()
        self.targretTime.setFixedWidth(80)

        self.terminal = MyTextBox(self)
        self.terminal.setFixedHeight(300)
        terminalL = QLabel("-------= Terminal =-------")

        # Layout section
        vbox = QVBoxLayout()
        vbox.addWidget(serialPortL)
        vbox.addWidget(self.serialPort)
        vbox.addWidget(serialBaudRateL)
        vbox.addWidget(self.serialBaudRate)
        vbox.addWidget(self.hexMode)

        vbox.addWidget(currentTimeL)
        vbox.addWidget(self.currentTime)
        vbox.addWidget(targetTimeL)
        vbox.addWidget(self.targretTime)
        vbox.addWidget(getTimeButton)
        vbox.addWidget(syncButton)
        vbox.addStretch(1)

        vbox1 = QVBoxLayout()
        vbox1.addWidget(terminalL)
        vbox1.addWidget(self.terminal)
        vbox1.addStretch(1)

        hbox = QHBoxLayout()
        hbox.addLayout(vbox)
        hbox.addLayout(vbox1)
        self.centralWidget().setLayout(hbox)  # add the layout to the central widget

        # Serial Thread and
        self.text_update.connect(self.append_text)      # Connect text update to handle
        sys.stdout = self #.centralWidget()
        self.serth = SerialThread(portname, baudrate)   # Start serial thread
        self.serth.start()

        # Make binding between buttons and functions
        getTimeButton.clicked.connect(self.GetTime)
        syncButton.clicked.connect(self.SyncTime)

        # Geometry and position
        self.setGeometry(300, 300, 350, 300)
        self.center()
        self.setWindowTitle('Logger control pannel')
        self.show()

    def center(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def GetTime(self):
        now = datetime.now()
        current_time = now.strftime("%H:%M:%S")
        self.currentTime.setText(current_time)

    def SyncTime(self):
        self.targretTime.setText('Synced')
        self.serth.ser_out('\xFE\x44\x00\x08\x02\x9F\x25')
        time.sleep(1)


    def changeHexMode(self, state):
        if state == Qt.Checked:
            hexmode = True
            print("HEX Mode Enable\n", self.terminal)
        else : #state == Qt.Unchecked:
            hexmode = False
            print("HEX Mode Disable\n", self.terminal)

    def write(self, text):                      # Handle sys.stdout.write: update display
        self.text_update.emit(text)             # Send signal to synchronise call with main thread

    def flush(self):                            # Handle sys.stdout.flush: do nothing
        pass

    def append_text(self, text):                # Text display update handler
        cur = self.terminal.textCursor()
        cur.movePosition(QTextCursor.End) # Move cursor to end of text
        s = str(text)
        while s:
            head,sep,s = s.partition("\n")      # Split line at LF
            cur.insertText(head)                # Insert text at cursor
            if sep:                             # New line if LF
                cur.insertBlock()
        self.terminal.setTextCursor(cur)         # Update visible cursor

    def keypress_handler(self, event):          # Handle keypress from text box
        k = event.key()
        s = RETURN_CHAR if k==QtCore.Qt.Key_Return else event.text()
        if len(s)>0 and s[0]==PASTE_CHAR:       # Detect ctrl-V paste
            cb = QApplication.clipboard()
            self.serth.ser_out(cb.text())       # Send paste string to serial driver
        else:
            self.serth.ser_out(s)               # ..or send keystroke

    def closeEvent(self, event):                # Window closing
        self.serth.running = False              # Wait until serial thread terminates
        self.serth.wait()



# Thread to handle incoming & outgoing serial data
class SerialThread(QThread):
    def __init__(self, portname, baudrate): # Initialise with serial port details
        QThread.__init__(self)
        self.portname, self.baudrate = portname, baudrate
        self.txq = Queue.Queue()
        self.running = True

    def ser_out(self, s):                   # Write outgoing data to serial port if open
        self.txq.put(s)                     # ..using a queue to sync with reader thread
        sys.stdout.write('> ' + s + '\n')

    def ser_in(self, s):                    # Write incoming serial data to screen
        display('<' + s)

    def run(self):                          # Run serial reader thread
        print("Opening %s at %u baud %s" % (self.portname, self.baudrate,
              "(hex display)" if hexmode else ""))
        try:
            self.ser = serial.Serial(self.portname, self.baudrate, timeout=SER_TIMEOUT)
            time.sleep(SER_TIMEOUT*1.2)
            self.ser.flushInput()
        except:
            self.ser = None
        if not self.ser:
            print("Can't open port")
            self.running = False
        while self.running:
            s = self.ser.read(self.ser.in_waiting or 1)
            if s:                                       # Get data from serial port
                self.ser_in(bytes_str(s))               # ..and convert to string
            if not self.txq.empty():
                txd = str(self.txq.get())               # If Tx data in queue, write to serial port
                self.ser.write(str_bytes(txd))
        if self.ser:                                    # Close serial port when thread finished
            self.ser.close()
            self.ser = None


if __name__ == '__main__':

    app = QApplication(sys.argv)
    ex = myWindow()
    sys.exit(app.exec_())


















