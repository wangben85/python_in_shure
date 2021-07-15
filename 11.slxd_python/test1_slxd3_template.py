"""
This is the SLXD3 python script test template
python test_slxd.py -p com6                   # use the IR serial port
python test_slxd.py -n 192.168.1.101 -p 8024  # use the telnet 8024 port over AD4Q unit
"""
from bhtx import BhTx
import argparse

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

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    # Print system info for this test run
    bhtx = BhTx.get_instance()
    bhtx.send_cmd("verapp")


