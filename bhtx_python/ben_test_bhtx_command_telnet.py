"""
Periodically run one or some CLI commands for auto test
This Case is set the default host to telnet over AD4
"""

import os
import time
from bhtx import BhTx
import argparse

if __name__ == "__main__":

    # @todo Arg parsing here could use some improvement...
    parser = argparse.ArgumentParser(description='Run BHTX integration tests')
    # Set default host is 192.168.1.101
    parser.add_argument('-n', '--hostname',default='192.168.1.101', help='Hostname for CLI connection')
    # Set default COM=8024
    parser.add_argument('-p', '--port', default='8024', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b', '--baud', default=115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True,
                        help='Specify that serial connection is NOT IR')
    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    # e.g    python ben_test_bhtx.py -p com6                   # use the serial port
    # e.g.   python ben_test_bhtx.py -n 192.168.1.103 -p 8024  # use the telnet 8024 port
    # note:  No input, hostname = None,  baud = 115200
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    # Print system info for this test run
    bhtx = BhTx.get_instance()
    bhtx.send_cmd("verapp")
    bhtx.send_cmd("su dev dev")

    for i in range(1, 100):
        bhtx.send_cmd("factoryReset")
        bhtx.send_cmd("reboot", expect_resp=False)  # reboot the device
        time.sleep(4)  # delay 4 seconds
        bhtx.send_cmd("su mfr Rj1146uHCpsCHakrBKQ")
        bhtx.send_cmd("serialnum")
        bhtx.send_cmd("txtype")
        bhtx.send_cmd("ateuid")
        bhtx.send_cmd("verpkg")
        bhtx.send_cmd("verboot")
        bhtx.send_cmd("verapp")
        bhtx.send_cmd("verfpga")
        bhtx.send_cmd("rebootfpga 2")
        print('test ran %d times\n' % i)
        if i == 100:
            print('Finished! test ran %d times\n' % i)
