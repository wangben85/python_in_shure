"""
This test is to set the band limit frequency for ADX5 variant B R52 band as a example
"""
from bhtx import *
import sys
import time
import enum
import csv
import argparse

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(description='Run BHTX integration tests')
    # Don't Set default host 
    parser.add_argument('-n', '--hostname', help='Hostname for CLI connection')
    # Set default COM=4
    parser.add_argument('-p', '--port', default='com4', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b','--baud', default = 115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True, help='Specify that serial connection is NOT IR')
   
    #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Specify the band limit csv file name
    #rx_variantB_R52_bandlimit.csv is the file contain the test bandlimit frequency for variant A
    parser.add_argument('-f','--file', default = './bandlimit/rx_variantB_R52_bandlimit.csv', help='Specify the bandlimit csv file')
    
    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    tx = BhTx.get_instance()

    # band limit test starts
    tx.send_cmd("setbandlimit reset") # reset the bandlimit whatever it used to be
    
    tx.send_cmd("getbandlimit") # get the default bandlimit first
    
    tx.send_cmd("setbandlimit R52") # set the bandlimit for band R52
    
    with open(args.file,encoding = 'utf-8') as csvfile:
       reader = csv.reader(csvfile)
       for rows in enumerate(reader):
          row = rows
          #print(row)  # print each line of the bandlimit table
          print(row[1][0])  #arg[1] = indx
          print(row[1][1])  #arg[2] = limit freq
          tx.send_cmd("setbandlimit {} {}".format(row[1][0],row[1][1]))
          time.sleep(0.1)   # delay 100ms
    
    tx.send_cmd("getbandlimit") # check bandlimit after setting
    
    tx.send_cmd("getfreq 1")                  # get default channel 1 freq
    time.sleep(0.5)                           # delay 500ms
    tx.send_cmd("setfreq 1 800000")           # set freq in the band limit range, ok
    time.sleep(0.5)                           # delay 500ms
    tx.send_cmd("setfreq 1 801000 blenable")     # more than two arguments, so set channel 1 freq with respect of band limit, will return fail
    tx.send_cmd("setfreq 1 801000")           # only two arguments, set channel 1 freq not respect of band limit, will return ok
    tx.send_cmd("setfreq 1 801100 blenable")     # more than two arguments, so set channel 1 freq with respect of band limit, will return fail
    tx.send_cmd("setfreq 1 801100")           # only two arguments, set channel 1 freq not respect of band limit, will return ok
    tx.send_cmd("setfreq 1 80200")            # set freq in the band limit range, ok
  
