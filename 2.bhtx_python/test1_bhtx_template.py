"""
This is the AD3 python script test template
python ben_test_bhtx.py -p com6                   # use the IR serial port
python ben_test_bhtx.py -n 192.168.1.101 -p 8024  # use the telnet 8024 port over AD4Q unit
"""
import unittest
import os
from bhtx import BhTx
import argparse

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
    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    # e.g    python ben_test_bhtx.py -p com6                   # use the IR serial port
    # e.g.   python ben_test_bhtx.py -n 192.168.1.101 -p 8024  # use the telnet 8024 port over AD4Q unit
    # note:  No input, hostname = None,  baud = 115200
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    # Print system info for this test run
    bhtx = BhTx.get_instance()
    bhtx.send_cmd("verfpga")
    bhtx.send_cmd("verapp")


