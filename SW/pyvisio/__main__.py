from gui.gui import Ui_MainWindow
from pubsub import pub
from PySide2.QtWidgets import *
from PySide2.QtCore import QTimer
import sys
from comm import Communicator
import json
from parse import *


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
        self.setupUi(self) 

class Controller():
    DEFAULT_INTERVAL = 0.1

    def __init__(self):
        self.comm = Communicator()
        self.active = False
        # set up the first frame which displays the current Model value
        self.view1 = MainWindow(None)
        for port in self.comm.get_ports():
            self.view1.add_port(port)
        with open('pyvisio.json') as json_file:
            self.data = json.load(json_file)
            for p in self.data['servo']:
                self.view1.add_servo(p["name"])
        self.view1.set_minmax(-45, 45)
        self.view1.show()
        self.meas_timer = QTimer()
        self.meas_timer.setInterval(self.DEFAULT_INTERVAL * 1e3)
        self.meas_timer.timeout.connect(self.do_measure)          
        self.pos_timer = QTimer()
        self.pos_timer.setInterval(self.DEFAULT_INTERVAL * 1e3)
        self.pos_timer.timeout.connect(self.get_pos) 
        self.pos_timer.start()        
        pub.subscribe(self.change_pos, 'position_changing')
        pub.subscribe(self.do_connection, 'do_connect')
        pub.subscribe(self.do_disconnection, 'do_disconnect')
        pub.subscribe(self.do_power_on, 'power_on')
        pub.subscribe(self.do_power_off, 'power_off') 
        pub.subscribe(self.start_measure, 'start_measure') 
        pub.subscribe(self.stop_measure, 'stop_measure')     
        pub.subscribe(self.select_servo, 'select_servo')         
    
    def change_pos(self, position):
        print(f'New pos: {position}')
        self.comm.send(f'DP{position}')
        self.comm.read()
        
    def do_connection(self, port):
        print(f'Connecting to {port}')
        if self.comm.connect(port) == True:
            pub.sendMessage("connected")
        else:
            pub.sendMessage("conn_error")
        
    def do_disconnection(self):
        self.stop_measure()    
        self.comm.disconnect()
        print('Disconnected')
        self.active = False
        pub.sendMessage("disconnected")
        
    def do_power_on(self):
        self.comm.send('PWR1')
        self.comm.read()
        self.active = True
        print('Power On')
        
    def do_power_off(self):
        self.comm.send('PWR0')
        self.comm.read()
        self.active = False
        print('Power Off')
        
    def do_measure(self):
        d = {}
        self.comm.send('GUS')
        d['U'] = parse("US:{}", self.comm.read())[0]
        self.comm.send('GCI')
        d['I'] = parse("I:{}", self.comm.read())[0]
        self.comm.send('GT')
        d['T'] = parse("T:{}", self.comm.read())[0]
        pub.sendMessage("measure", data=d)
        
    def start_measure(self):
        self.meas_timer.start()
        
    def stop_measure(self):
        self.meas_timer.stop()        
        
    def select_servo(self, servo):
        print('Selected: ' + str(servo))
        for p in self.data['servo']:
            if p["name"] == servo:
                self.comm.send(f'SI{p["iface"]}')
                self.comm.read()
                if "baudrate" in p:
                    if isinstance(p["baudrate"], list):
                        baudrate = ':'.join(p["baudrate"])
                    else:
                        baudrate = str(p["baudrate"])
                    self.comm.send(f'SB{baudrate}')
                    self.comm.read()
                break
                
    def get_pos(self):
        if self.active:
            self.comm.send('GP')
            try:
                pos = parse("P:{}", self.comm.read())[0]
            except:
                pos = '--.-'
            pub.sendMessage("show_pos", pos=pos)

def main():
    print('pubsub API version', pub.VERSION_API)
    app = QApplication(sys.argv)
    c = Controller()
    app.exec_()

if __name__ == "__main__":
    main()
