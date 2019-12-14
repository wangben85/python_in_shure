"""
This test is to set the RF calibration data to AD3 variant A
"""
from bhtx import *
import sys
import time
import enum
import csv
import argparse

if __name__ == "__main__":
    """
    This script is to set the RF calibration data in AD3
    """
    parser = argparse.ArgumentParser(description='Run BHTX integration tests')
    parser.add_argument('-n','--hostname', help='Hostname for CLI connection')
    # Set default COM=6
    parser.add_argument('-p','--port', default = 'COM6', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b','--baud', default = 115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True, help='Specify that serial connection is NOT IR')
   
    #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Specify the calibration data csv file name
    #variantA_rf_cali_data.csv is the file contain the calibration data
    #default_clear_rf_data.csv is the file remove all the calibration data, back to default(0xFF)
    parser.add_argument('-f','--file', default = 'variantA_rf_cali_data.csv', help='Specify the calibration data csv file')
    
    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    # e.g.   python rf_cal_set.py -p com6,  set the calibration table
    # e.g.   python rf_cal_set.py -p com6 -f default_clear_rf_data.csv,  remove all the calibration data
    # note:  No input, hostname = None,  baud = 115200
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    tx = BhTx.get_instance()

    tx.unlock() # switch to dev privilege mode
    tx.send_cmd("ls") # to check the mode switch success
    
    tx.get_cal(CalTable.PRIMARY)   # load current calibration value from the device
    
    # example of setting of RF calibration
    # rfcal 1 low,0 0x0b,0x59
    # argv[0] = 1: For primary or secondary 
    # argv[1] = low,0 : There are two subargs, subarg[0] = 'low' for power level
    #                                          subarg[1] = '0' for frequency idx
    # note: The ',' argument between <power>,<fidx> will be omit
    # argv[2] = 0xb,0x59 : There are two subargs, subarg[0] = '0xb' for asic value
    #                                             subarg[1] = '0x59' for FPGA value 
    # note: The ',' argument between <asic>,<fpga> will be omit
    with open(args.file,encoding = 'utf-8') as csvfile:
       reader = csv.reader(csvfile)
       for rows in enumerate(reader):
          row = rows
          #print(row)  # print each line of the calibration table
          #print(row[1][0])  #arg[1]
          #print(row[1][1])  #arg[2]
          tx.send_cmd("rfcal 1 {} {}".format(row[1][0],row[1][1]))
          time.sleep(0.1)   # delay 100ms

    tx.send_cmd("rfcal finish")
    tx.get_cal(CalTable.PRIMARY)   # reload the calibration value after setting
