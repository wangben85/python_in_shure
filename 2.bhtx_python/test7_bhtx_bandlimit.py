"""
This test is to set the band limit frequency for AD3 variant A as a example
"""
from bhtx import *
import sys
import time
import enum
import csv
import argparse

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(description='Run BHTX integration tests')
    # Set default host is 192.168.1.104
    parser.add_argument('-n', '--hostname',default='192.168.1.104', help='Hostname for CLI connection')
    # Set default COM=8024
    parser.add_argument('-p', '--port', default='8024', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b','--baud', default = 115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True, help='Specify that serial connection is NOT IR')
   
    #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Specify the band limit csv file name
    #tx_variantA_bandlimit.csv is the file contain the test bandlimit frequency for variant A
    parser.add_argument('-f','--file', default = './bandlimit/tx_variantA_bandlimit.csv', help='Specify the bandlimit csv file')
    
    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    tx = BhTx.get_instance()

    tx.unlock() # switch to dev privilege mode
    tx.send_cmd("ls") # to check the mode switch success
    
    with open(args.file,encoding = 'utf-8') as csvfile:
       reader = csv.reader(csvfile)
       for rows in enumerate(reader):
          row = rows
          #print(row)  # print each line of the bandlimit table
          print(row[1][0])  #arg[1] = indx
          print(row[1][1])  #arg[2] = limit freq
          tx.send_cmd("bandlimit {} {}".format(row[1][0],row[1][1]))
          time.sleep(0.1)   # delay 100ms
    
    tx.send_cmd("bandlimit") # to check if the bandlimit set correct
    tx.send_cmd("blenable on", expect_resp=False)  # enable bandlimit to take effect
    time.sleep(4)  # delay 4 seconds to wait the system reboot ok
    tx.send_cmd("freq 1 470.125") # set freq in the band limit range, ok
    time.sleep(0.5)               # delay 500ms
    tx.send_cmd("freq 1 500")     # set freq in the band limit range, ok
    time.sleep(0.5)               # delay 500ms
    tx.send_cmd("freq 1 501")     # set freq out of the band limit range, fail
    time.sleep(0.5)               # delay 500ms
    tx.send_cmd("freq 1 501.250") # set freq out of the band limit range, fail
    time.sleep(0.5)               # delay 500ms
    tx.send_cmd("freq 1 502.125") # set freq out of the band limit range, fail
    tx.send_cmd("bandlimit reset")# reset the band limit after the test
    tx.send_cmd("bandlimit")      # check reset the band limit ok or not
