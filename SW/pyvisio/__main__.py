from .gui import Ui_MainWindow
from pubsub import pub
from PySide2.QtWidgets import *
from PySide2.QtCore import QTimer
import sys
from .comm import Communicator
import json
from parse import *
import os
import matplotlib.pyplot as plt
import numpy as np


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
        self.setupUi(self) 

class Controller():
    DEFAULT_INTERVAL = 0.1
    JSON_FILE_NAME = 'pyvisio.json'

    def __init__(self):
        self.comm = Communicator()
        self.active = False
        # set up the first frame which displays the current Model value
        self.view1 = MainWindow(None)
        for port in self.comm.get_ports():
            self.view1.add_port(port)
        dir = os.path.dirname(__file__)
        print(dir)
        json_name = os.path.join(dir, self.JSON_FILE_NAME)
        self.servo = None
        with open(json_name) as json_file:
            self.data = json.load(json_file)
            for p in self.data['servo']:
                if self.servo is None:
                    self.servo = p["name"]
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
        pub.subscribe(self.write_param, 'write_param') 
        pub.subscribe(self.do_graph, 'do_graph')
    
    def change_pos(self, position):
        print(f'New pos: {position}')
        self.comm.send(f'DP{position}')
        self.comm.read()
        
    def do_connection(self, port):
        print(f'Connecting to {port}')
        if self.comm.connect(port) == True:
            self.select_servo(self.servo)
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
        pub.sendMessage("remove_params") 
        print('Selected: ' + str(servo))
        self.servo = servo
        for p in self.data['servo']:
            if p["name"] == servo:
                self.comm.send(f'SI{p["iface"]}')
                self.comm.read()
                if "baudrate" in p:
                    if isinstance(p["baudrate"], list):
                        baudrate = ':'.join([str(i) for i in p["baudrate"]])
                    else:
                        baudrate = str(p["baudrate"])
                    self.comm.send(f'SB{baudrate}')
                    self.comm.read()
                for param in p["params"]:
                    pub.sendMessage("add_param", name=param, param=p["params"][param])
                break
                
    def get_pos(self):
        if self.active:
            self.comm.send('GP')
            try:
                pos = parse("P:{}", self.comm.read())[0]
            except:
                pos = '--.-'
            pub.sendMessage("show_pos", pos=pos)
            
    def write_param(self, name, value):
        for p in self.data['servo']:
            if p["name"] == servo:
                for param in p["params"]:
                    if param == name:
                        addr = p["params"][param]["addr"][2:]
                        self.comm.send(f'WB{addr}:{value}')
                        self.comm.read()
                        break 
                    
    def read_param(self, name):
        pass    
        
    def do_graph(self):
        t_step = 0.0001
        max_freq = 500
        y_min_scale = 0
        dpi = 80
        offset = 300
        end = -100
        i = 0
        arr = []
        while i < 2048 // 8:
            self.comm.send(f'GB{i}')
            str = self.comm.read()
            n = 2
            arr.extend([int(str[i:i+n], 16) for i in range(0, len(str), n)])
            i += 1
        x_axis = list(range(len(arr[offset:end])))
        y = np.array(arr[offset:end])
        t = np.linspace(0, t_step * len(x_axis), len(x_axis), endpoint=True)
        #plt.plot(t, y)
        #plt.xlabel('Time ($s$)')
        #plt.ylabel('Amplitude ($Unit$)')
        #plt.show()

        hann = np.hanning(len(y))
        #plt.plot(t, y*hann)
        #plt.xlabel('Time ($s$)')
        #plt.ylabel('Amplitude ($Unit$)')
        #plt.show()

        Y = np.fft.fft(y)
        N = len(Y) // 2 + 1
        dt = t[1] - t[0]
        fa = 1.0 / dt # scan frequency
        X = np.linspace(0, fa/2, N, endpoint=True)
        out = 2.0/N*np.abs(Y[:N])
        num_of_bars = (int)(max_freq // (fa / 2 / N))
        #print(N, fa, num_of_bars)
        fig = plt.figure(dpi = dpi, figsize = (800 / dpi, 400 / dpi) )
        axes = plt.gca()
        max_y = np.max(out[5:])
        max_y = 1
        axes.set_ylim([y_min_scale, max_y])
        plt.plot(X[5:num_of_bars], out[5:num_of_bars])
        #plt.bar(X[:num_of_bars], out[:num_of_bars], 2)
        plt.xlabel('Frequency ($Hz$)')
        plt.ylabel('Amplitude ($Unit$)')
        #pre, ext = os.path.splitext(f_name)
        #fig.savefig(pre + '.png')
        plt.show()        
        #plt.plot(x_axis, arr) #Построение графика
        #plt.xlabel(r'$x$') #Метка по оси x в формате TeX
        #plt.ylabel(r'$f(x)$') #Метка по оси y в формате TeX
        #plt.grid(True) #Сетка
        #plt.show() #Показать график               

def main():
    print('pubsub API version', pub.VERSION_API)
    app = QApplication(sys.argv)
    c = Controller()
    app.exec_()

if __name__ == "__main__":
    main()
