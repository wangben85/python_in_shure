"""
This test case is execute CLI commmand "help all" at privilege mode to get all the supported CLI commands and put them stored in csv file
We have to create the folder AD3_Supported_CLI first !!
"""

import os
import time
from bhtx import BhTx
import argparse
import csv

if __name__ == "__main__":

    # @todo Arg parsing here could use some improvement...
    parser = argparse.ArgumentParser(description='Run BHTX integration tests')
    parser.add_argument('-n','--hostname', help='Hostname for CLI connection')
    # Set default COM=6
    parser.add_argument('-p','--port', default = 'COM6', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b','--baud', default = 115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True, help='Specify that serial connection is NOT IR')

    parser.add_argument('-f','--file', default = './AD3_Supported_CLI/AD3_CLI_Command_Lists_All.csv', help='AD3 all the CLI Commands List')

    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    # e.g    python ben_test_bhtx.py -p com6                   # use the serial port
    # e.g.   python ben_test_bhtx.py -n 192.168.1.103 -p 8024  # use the telnet 8024 port
    # note:  No input, hostname = None,  baud = 115200
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    # Print system info for this test run
    bhtx = BhTx.get_instance()

    bhtx.unlock() # switch to dev privilege mode, put it into dev mode will get all the CLI commands list instead of guest mode
    bhtx.send_cmd("ls") # to check the mode switch success
    
    #set the csv file header 
    csv_headers = ['CLI Command','Description']

    with open(args.file,'w+',encoding = 'utf-8', newline='') as csvfile:
      f_csv = csv.writer(csvfile)
      f_csv.writerow(csv_headers)             # write header first
      lists = bhtx.send_cmd("help all")       # send_cmd return value is [success, lines]
      #print(lists[1])                        # print all the CLI commands
      for row in lists[1]:                    # only care about lists[1] because which is the CLI response string ,not the boolean of success or not
         #print(row)                          # print command lists and description
         cmd_tb = row.split(' ', 1)           # split the response into two parts: command(cmd_tb[0]) and description(cmd_tb[1])
         cmd_tb[1] = cmd_tb[1].lstrip()       # remove the description part's space in the beginning
         print(cmd_tb)
         f_csv.writerow(cmd_tb)               # write to the csv file

