import comm

class Servo():
    def __init__(self):
        pass
        
    def move_to_pos(self, pos):
        pass
        
    def get_pos(self):
        return 0
        
    def read_param(self, addr):
        return 0
        
    def write_param(self, addr, value):
        pass

class Adapter():
    def __init__(self):
        pass
    
    def get_supply(self):
        return 0
        
    def get_current(self):
        return 0