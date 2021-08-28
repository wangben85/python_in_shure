"""
This test is to program FreqTable file to external flash with python script

Note: If we are using this script on high loading device Before run this script,
      FPGA interrupt and battery polling needs to be disabled first
"""
import time
import datetime
import argparse
import os
import struct
from bhtx import *

if __name__ == "__main__":
    # @todo Arg parsing here could use some improvement...
    parser = argparse.ArgumentParser(description='Run SLXD3 integration tests')
    # Set default host is 192.168.1.102
    parser.add_argument('-n', '--hostname',default='192.168.1.102', help='Hostname for CLI connection')
    #parser.add_argument('-n', '--hostname', help='Hostname for CLI connection')
    # Set default COM=8024
    parser.add_argument('-p', '--port', default='8024', help='Port for CLI connection')
    #parser.add_argument('-p', '--port', default='COM10', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b', '--baud', default=115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True,
                        help='Specify that serial connection is NOT IR')

    
    args = parser.parse_args()

    # start time print
    startTime = datetime.datetime.now()
    print('The script start time is') 
    print(startTime) 

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    tx = BhTx.get_instance()

    tx.unlock()  # switch to dev privilege mode

    while True:
      ret = tx.send_cmd("tone on")            # stop the fpga interrupt polling 
      time.sleep(5)                          # delay 1 second
      ret = tx.send_cmd("tone off")      # stop the batterypolling 
      time.sleep(5)                          # delay 1 second
      ret = tx.send_cmd("verapp")      # stop the batterypolling 

      # current time print
      currentTime = datetime.datetime.now()
      print('The current time is') 
      print(currentTime) 
      
      time.sleep(3)                          # delay 1 second


