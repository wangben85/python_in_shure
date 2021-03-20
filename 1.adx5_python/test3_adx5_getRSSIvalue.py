"""
This script is to start front panel scan and restore the scan data into csv file
"""
import unittest
import os
import time
from bhtx import BhTx
import argparse
import csv

if __name__ == "__main__":

    # @todo Arg parsing here could use some improvement...
    parser = argparse.ArgumentParser(description='Run BHTX integration tests')
    # Don''t Set default host
    parser.add_argument('-n', '--hostname', help='Hostname for CLI connection')
    # Set default COM = 8023
    parser.add_argument('-p', '--port', default='com4', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b','--baud', default=115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True,
                        help='Specify that serial connection is NOT IR')
    # Set the rssi table file
    parser.add_argument('-f', '--file', default='./adx5_rssi_table/adx5_rssi.csv',
                        help='ADX5 front panel scan RSSI value table')

    # We can not specify the start,stop and step freq but use the default ones
    # Or
    # We can specify them in the command line e.g. python test3_adx5_getRSSIvalue.py --start 606000 --stop 606100 --step 25

    # Set default start frequency, default is 606000 Hz
    parser.add_argument('--start', default='606000', help='fp scan start frequency')
    # Set default stop frequency, default is 6061100 Hz
    parser.add_argument('--stop', default='606100', help='fp scan stop frequency')
    # Set default step frequency, default is 25 Hz
    parser.add_argument('--step', default='25', help='fp scan step frequency')


    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    # Print system info for this test run
    bhtx = BhTx.get_instance()

    # test command 'send_cmd_special_resp', read feedback timeout
    # bhtx.send_cmd_special_resp("getversion a",special_resp='CLI> ')

    # test command 'send_cmd_special_resp', read feedback ok
    # bhtx.send_cmd_special_resp("getversion a",special_resp='---> ')

    # test command start fp scan channel in range : start(start 606000) stop(606050) step(25)
    #bhtx.send_cmd_special_resp("sScanStart 1 606000 606100 25", special_resp='100%', timeout=1000)
    #bhtx.send_cmd_special_resp("sScanStart 2 606000 606100 25", special_resp='100%', timeout=1000)
    # use the command line parameters as the start, stop and step frequencies
    bhtx.send_cmd_special_resp("sScanStart 1" + " " + args.start + " " + args.stop + " " + args.step, special_resp='100%', timeout=1000)
    bhtx.send_cmd_special_resp("sScanStart 2" + " " + args.start + " " + args.stop + " " + args.step, special_resp='100%', timeout=1000)

    # test command start fp scan channel 1 all the frequency range as variant, leave enough timeout to make scan finish
    # bhtx.send_cmd_special_resp("sScanStart 1", special_resp='100%', timeout=1000)

    # sleep 5 seconds
    time.sleep(5)

    # test command dump the scan data in range: start(start 606000) stop(606050) step(25)
    #bhtx.send_cmd("sScanDump 606000 606100 25", timeout=1000)
    # use the command line parameters as the start, stop and step frequencies to dump the scan data
    lists = bhtx.send_cmd("sScanDump"  + " " + args.start + " " + args.stop + " " + args.step, timeout=1000)

    # test command dump all the scan data, leave enough timeout to let scan dump finish
    # bhtx.send_cmd("sScanDump", timeout=1000)

    # sleep 5 seconds
    time.sleep(5)

    #set the csv file header
    csv_headers = ['Freq', 'Val_1A', 'Val_1B', 'Val_2A', 'Val_2B']

    with open(args.file,'w+',encoding = 'utf-8', newline='') as csvfile:
      f_csv = csv.writer(csvfile)
      f_csv.writerow(csv_headers)               # write header first
      #print(lists[1])                          # print scan dump useful information
      rssi_value_only_list = lists[1][3:]       # filter to get the rssi value only
      #print(rssi_value_only_list)              # print the rssi value
      for row in rssi_value_only_list:
           #print(row)                          # print every row of the rssi value
           cmd_tb = row.split('\t',4)           # split the response into 5 parts, 'Freq', 'Val_1A', 'Val_1B', 'Val_2A', 'Val_2B'
           print(cmd_tb)
           f_csv.writerow(cmd_tb)               # write to the csv file


