#!/usr/bin/env python3

import socket
import mysql.connector
from signal import signal, SIGINT
import sys 
from PyQt5 import QtCore, QtWidgets, uic
import threading

T_PORT = 9867
TCP_IP = '172.21.74.225'
BUF_SIZE = 30

# Update Login credentials for db server before use
USER="vatsal"
PASWWD="12345"
DATABASE="LPEDT"
TABLE="Power"
 

# Connect to db server 
mydb = mysql.connector.connect(
  host="localhost",
  user=USER,
  passwd=PASWWD, 
  buffered=True
)

mycursor = mydb.cursor()

# Create Database if not exists 
mycursor.execute("CREATE DATABASE IF NOT EXISTS {}".format(DATABASE))
mydb.commit()

# Connect to db server on specific database  
mydb = mysql.connector.connect(
  host="localhost",
  user=USER,
  passwd=PASWWD,
  database=DATABASE,
  buffered=True
)

mycursor = mydb.cursor()

# Create Table if not exists
mycursor.execute("CREATE TABLE IF NOT EXISTS {} (id INT, type VARCHAR(150), data FLOAT, time_stamp TIMESTAMP)".format(TABLE))

mydb.commit()

# create a socket object name 'k'
tcp_soc = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
tcp_soc.bind((TCP_IP, T_PORT))

global con, addr

def handler(signal_received, frame):
    # Handle any cleanup here
    print('SIGINT or CTRL-C detected. Exiting gracefully')
    con.close()
    exit(0)
class Ui(QtWidgets.QMainWindow):
    
    """
        Ui: This class inherits from QMainWindow and loads .ui file of GUI. It consist of all the functions that would be
        required to display data on the GUI and to obtain temperature reading from the DHT22 sensor.
    """
    
    def __init__(self):
        """
            __init__: This constructor searches al child objects created in GUI and links it with local variable defined in class 
        """
        super(Ui, self).__init__() # Call the inherited classes __init__ method
        uic.loadUi('project1_GUI.ui', self) # Load the .ui file
        self.button_refresh = self.findChild(QtWidgets.QPushButton, 'button_refresh') 
        #self.button_refresh.clicked.connect(self.refresh_data) 
        self.label_temp = self.findChild(QtWidgets.QLabel, 'label_temp') 
        self.label_humidity = self.findChild(QtWidgets.QLabel, 'label_humidity') 
        self.label_time = self.findChild(QtWidgets.QLabel, 'label_time')
        self.label_battery = self.findChild(QtWidgets.QLabel, 'label_battery')
        self.show() # Show the GUI

def TCP_thread():
    global ui, tcp_soc
    ui.label_battery.setText("No Clients Connected !!!!")
    tcp_soc.listen(10)
    con, addr = tcp_soc.accept()
    print ('Connection Address is: ' , addr)
    ui.label_battery.setText("Connected to clent at IP addr: {}".format(addr))
    while True:
        s_data = con.recv(BUF_SIZE)
#        print("Hereeeeeeeee.....")
#        data = 25978
        if not s_data:
            break
        print ("Received data {}" .format(s_data))
        data = s_data.decode('utf-8')
        data = int(data)
        message_type = data >> 14
        print(message_type)
        lpn_id = (data >> 13) & 1
        print(lpn_id)
        if(message_type == 0):
            print("LED Message")
            ui.label_battery.setText("Low Battery on Node {}".format(lpn_id))
        elif(message_type == 2):
            # insert power data into the table
            decimal = int((data & 8191) / 10)
            floa = (data & 8191) % 10
            power = str(decimal) + '.' + str(floa)
            print("Power " + power)
            mycursor.execute("INSERT INTO {} (id, type, data) VALUES ({}, '{}', {})".format(TABLE, lpn_id, "power", power))
            mydb.commit()
            ui.label_time.setText("{}".format(power))

        elif(message_type == 1):
            # insert current data into the table
            decimal = (data & 8191) >> 8
            floa = data & 255
            current = str(decimal) + '.' + str(floa)
            print("Current " + current)
            mycursor.execute("INSERT INTO {} (id, type, data) VALUES ({}, '{}', {})".format(TABLE, lpn_id, "current", current))
            mydb.commit()
            ui.label_humidity.setText("{}".format(current))

        elif(message_type == 3):
            # insert voltage data into the table
            decimal = (data & 8191) >> 5     #0x1fff
            floa = data & 31
            voltage = str(decimal) + '.' + str(floa)
            print("Voltage: " + voltage)
            mycursor.execute("INSERT INTO {} (id, type, data) VALUES ({}, '{}', {})".format(TABLE, lpn_id, "voltage", voltage))
            mydb.commit()
            ui.label_temp.setText("{}".format(voltage))
   
    
if __name__ == '__main__':
    print("Heerere")
    signal(SIGINT, handler)
    app = QtWidgets.QApplication(sys.argv)
    ui = Ui()
    """Multithreading"""
    thread1 = threading.Thread(target = TCP_thread)
    thread1.start()

    sys.exit(app.exec_())
    thread1.join()
        
    
        
