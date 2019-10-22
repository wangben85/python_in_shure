import os
import time
from bhtx import BhTx
import argparse

cli_commands_list = [
    "verapp",
    "verfpga",
    "verboot",
]

"""
Periodically run one or some CLI commands for auto test, and also record the CLI logs into txt file
"""

if __name__ == "__main__":

    # @todo Arg parsing here could use some improvement...
    parser = argparse.ArgumentParser(description='Run BHTX integration tests')
    parser.add_argument('-n','--hostname', help='Hostname for CLI connection')
    # Set default COM=6
    parser.add_argument('-p','--port', default = 'COM5', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b','--baud', default = 115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True, help='Specify that serial connection is NOT IR')
    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    # e.g    python ben_test_bhtx.py -p com6                   # use the serial port
    # e.g.   python ben_test_bhtx.py -n 192.168.1.103 -p 8024  # use the telnet 8024 port
    # note:  No input, hostname = None,  baud = 115200
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    # Print system info for this test run
    bhtx = BhTx.get_instance()

    # send command and record the feedback into log file
    fLog = open('./LogFile.txt', 'w')

    for i in range(1,6):
      fLog = open('./LogFile.txt', 'a')
      fLog.write("This is the %d Log\n" %i)
      fLog.close()
      for j in range(len(cli_commands_list)):
        bhtx.send_cmd_log(cli_commands_list[j])
      fLog = open('./LogFile.txt', 'a')
      fLog.write("\n\n")
      fLog.close()

