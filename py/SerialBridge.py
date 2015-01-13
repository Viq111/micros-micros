# pyScillo Package
# SerialBridge version:
version = 2
# By Viq
# License: Creative Commons Attribution-ShareAlike 3.0 (CC BY-SA 3.0) 
# (http://creativecommons.org/licenses/by-sa/3.0/)

##############
### IMPORT ###
##############
import serial, os, time

#####################
### OS DEFINITION ###
#####################

# getFirst to be modified if a new OS is added
# Currently supported: Windows (Tested on XP, Seven) and Linux (Tested on Debian)
if os.name == "nt": # Windows version
    system = "win"
    realtime = time.clock
    sleep = time.sleep
elif os.name == "posix": # Linux
    system = "linux"
    realtime = time.time
    sleep = time.sleep
else:
    raise SystemError, "OS is not supported"

###############
### CLASSES ###
###############

class SimpleSerial():
    "Easier class to deal with serial"
    
    def __init__(self,serial_port,baudrate=9600):
        "Creating serial objext"
        self.ser = serial.Serial(serial_port,baudrate)
        
    def readline(self):
        "Return last serial line without \\r\\n"
        data = self.ser.readline()
        return data[:-2]

class FPS():
    "Count FPS (or data/sec)"
    def __init__(self, prevent_dizzyness=False):
        "Creation de l'objet"
        # prevent_dizzyness is used to refresh the returned counter
        # only once per seconds max
        self.moy_over = 10 # Average FPS calculated over this number
        self.last_time = realtime()
        self.last_results = [-1]
        self.last_return = -100
        self.last_time_returned = realtime()
        self.prevent_dizzyness = prevent_dizzyness
        
    def update(self):
        "Update FPS"
        if (realtime()-self.last_time) == 0: # It's too quick to refresh
            return False
        
        if len(self.last_results) >= self.moy_over:
            del(self.last_results[0])

        self.last_results.append(int(1/(realtime()-self.last_time) * 100.0)/100.0) # 2 digits after .
        self.last_time = realtime()
        return True
    
    def get(self):
        "Give the FPS counter"

        if self.prevent_dizzyness:
            if realtime() - self.last_time_returned < 1:
                return self.last_return
        
        moy = sum(self.last_results) * 1.0 / len(self.last_results)
        moy = int(moy*100.0)/100.0
        
        if self.prevent_dizzyness:
            self.last_return = moy
            self.last_time_returned = realtime()
        return moy
        
    def get_last(self):
        "Give the instant FPS counter"
        return self.last_results[-1]

    def is_stable(self):
        "Return True if he thinks the FPS counter is stable"
        # ToDo: Enhance this function (detection)
        
        if (len(self.last_results) == self.moy_over) and (abs(self.get() - self.last_return) < ((self.get()/10.0)+2)):
            # There is a error < 10 percents
            return True
        else:
            return False

def isAvailable(serial_port, baudrate = 9600):
    "Ask OS if the serial port is open"
    try:
        serial.Serial(serial_port,baudrate)
    except:
        return False
    else:
        return True

def getFirst(baudrate=9600):
    "Find the first serial, and return a Simple Serial Object"
    # Working on XP, Seven, Debia, names are based on experience
    searchList = []
    if system == "win":
        for i in range(2,20):
            searchList.append("COM"+str(i))
    elif system == "linux":
        for i in range(0,7):
            searchList.append("/dev/ttyACM"+str(i))
    for ser_port in searchList:
        if isAvailable(ser_port):
            return SimpleSerial(ser_port,baudrate)
    raise IOError, "No available port Found !"
