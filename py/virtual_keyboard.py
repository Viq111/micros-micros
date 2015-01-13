# -*- coding:Utf-8 -*-
# Micros Micros Package
prog_name = "Micros Micros - Virtual Keyboard"
# version:
version = 1
# By Viq - Vianney Tran
# License: Creative Commons Attribution-ShareAlike 3.0 (CC BY-SA 3.0) 
# (http://creativecommons.org/licenses/by-sa/3.0/)

##############
### IMPORT ###
##############
import os, time, SerialBridge
import win32com.client

###############
### GLOBALS ###
###############

MIN = -5000
MAX = 7000
DELTA = 50

###################
### DEFINITIONS ###
###################

def getValue(ser):
    "Get the last value on the serial"
    while True:
        # Read serial until we get a correct value
        line = ser.readline()
        try:
            value = int(line.split(" ")[-1])
            if int(line.split(" ")[1]) == 1:
                value = - value
            if value > MIN and value < MAX:
                return value
        except:
            print "Warning: Wrong line, " + str(line)

def getMoyValue(ser, nb_ack = 1):
    "Get a mean value that should be mainly correct"
    values = []
    # First get the first values
    for i in range(nb_ack):
        values.append(getValue(ser))
    while True:
        # While not all values are near the mean,
        moy = sum(values) * 1.0 / nb_ack
        ok = True
        for v in values:
            if abs(moy-v) > DELTA:
                ok = False
        if ok:
            # If all value are near the mean, return the mean
            return moy
        else:
            # New sampling
            max_dev = values[0]
            # Remove extreme value
            for v in values:
                if abs(moy-v) > abs(moy-max_dev):
                    max_dev = v
            values.remove(max_dev)
            # Add the new value
            values.append(getValue(ser))


    
def simulateKeypress(to_write="", then_enter = False):
        "Emulate Keyboard"
        if to_write == "":  then_enter = True
        shell = win32com.client.Dispatch("WScript.Shell")
        for key in to_write:
            shell.SendKeys(str(key))
            time.sleep(0.01)
        if then_enter:
            shell.SendKeys('{ENTER}')
            time.sleep(0.01)
        return True

##################
###  __MAIN__  ###
##################

if __name__ == "__main__":
    print "> Welcome to " + str(prog_name) + " (r" + str(version) + ")"
    print "> By Viq (under CC BY-SA 3.0 license)"
    print "> Loading program ..."
    ser = SerialBridge.getFirst(57600)
    # Map keys to the near value
    print "> Calibrate each key then press return to start... "
    values = []
    key = raw_input("-> Virtual Key:")
    while key:
        value = getMoyValue(ser, 3)
        print "--> Key " + str(key) + " is associated with value " + str(value)
        values.append((value, key))
        key = raw_input("-> Virtual Key:")
    values.sort()
    print "> Running..."
    try:
        while True:
            val = getValue(ser)
            nearest_value = min(range(len(values)), key=lambda i: abs(values[i][0]-val))
            key = values[nearest_value][1]
            print "-> Value " + str(val) + " -> " + key
            simulateKeypress(key)
    except KeyboardInterrupt:
        print "> Bye!"
