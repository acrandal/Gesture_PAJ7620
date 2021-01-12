#!/usr/bin/env python3

import serial


if __name__ == "__main__":

    try:
        ser = serial.Serial('/dev/ttyUSB0', 115200)
        while(True):
            line  = ser.readline()
            line = line.decode('utf-8')
            line = line.rstrip()
            print(line)
    except KeyboardInterrupt:
        print("Interrupted - quitting")
    finally:
        ser.close()
