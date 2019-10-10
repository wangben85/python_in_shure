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
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    # Print system info for this test run
    bhtx = BhTx.get_instance()
    # bhtx.send_cmd("systeminfo")   #comment here

    # Use standard discovery mechanism to locate tests. Then run them
    loader = unittest.defaultTestLoader
    tests = loader.discover(start_dir=os.path.dirname(__file__))
    runner = unittest.TextTestRunner(stream=os.sys.stdout, buffer=True, verbosity=2)
    result = runner.run(tests)
    if result.errors or result.failures:
        os.sys.exit(1)            # if no unit tests fail, process finished with exit code 0
                                  # if any test cases fail, process will terminated and report error

