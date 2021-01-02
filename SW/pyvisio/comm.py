import sys
import glob
import serial

class Communicator():
    def __init__(self):
        pass
        
    def connect(self, port):
        try:
            self.port = serial.Serial(port, baudrate=115200, bytesize=8, parity='N', stopbits=1, timeout=0.1)
            self.send('ID')
            line = self.read()
            if line == 'MLS':
                return True
            else:
                return False
        except (OSError, serial.SerialException):
            return False
            
    def disconnect(self):
        self.port.close()
        
    def send(self, line):
        print(f'>> {line}')
        line = '\x1b' + line + '\r'
        self.port.write(bytearray(line, encoding='utf-8'))
        
    def read(self):
        line = self.port.readline().strip().decode('utf-8') 
        print(f'<< {line}')
        return line
    
    def get_ports(self):
        """ Lists serial port names

            :raises EnvironmentError:
                On unsupported or unknown platforms
            :returns:
                A list of the serial ports available on the system
        """
        if sys.platform.startswith('win'):
            ports = ['COM%s' % (i + 1) for i in range(256)]
        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            # this excludes your current terminal "/dev/tty"
            ports = glob.glob('/dev/tty[A-Za-z]*')
        elif sys.platform.startswith('darwin'):
            ports = glob.glob('/dev/tty.*')
        else:
            raise EnvironmentError('Unsupported platform')

        result = []
        for port in ports:
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, serial.SerialException):
                pass
        return result
