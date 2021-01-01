from pyvisio.gui.gui import Ui_MainWindow
from pubsub import pub
from PySide2.QtWidgets import *
import sys
from pyvisio.comm import Communicator

'''class MainWin(gui_main.MainFrame): 
    def __init__(self,parent): 
        gui_main.MainFrame.__init__(self,parent)'''

class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
        self.setupUi(self)

class Controller:
    def __init__(self):
        self.comm = Communicator()
        # set up the first frame which displays the current Model value
        self.view1 = MainWindow(None)
        for port in self.comm.get_ports():
            self.view1.add_port(port)
        self.view1.set_minmax(-45, 45)
        self.view1.show()
        #pub.subscribe(self.changePosition, 'position_changing')
    
    def changePosition(self, position):
        print(f'New pos: {position}')

def main():
    print('pubsub API version', pub.VERSION_API)
    app = QApplication(sys.argv)
    c = Controller()
    app.exec_()

if __name__ == "__main__":
    main()
