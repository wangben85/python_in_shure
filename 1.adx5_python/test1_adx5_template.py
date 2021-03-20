"""
This is the ADX5 python script test template
python test1_adx5_template.py -p comX                   # use the physical serial port and switch to another comX instead of default COM4
python test1_adx5_template.py -n 192.168.1.103 -p 8023  # use the ADX5 telnet 8023 port 
"""
import unittest
import os
from bhtx import BhTx
import argparse

if __name__ == "__main__":

    # @todo Arg parsing here could use some improvement...
    parser = argparse.ArgumentParser(description='Run BHTX integration tests')
    # Don''t Set default host
    parser.add_argument('-n', '--hostname',help='Hostname for CLI connection')
    # Set default COM = 8023
    parser.add_argument('-p', '--port', default='com4', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b','--baud', default = 115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True, help='Specify that serial connection is NOT IR')
    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    # Print system info for this test run
    bhtx = BhTx.get_instance()
    bhtx.send_cmd("systeminfo")  # print out ADX5 systeminfo


