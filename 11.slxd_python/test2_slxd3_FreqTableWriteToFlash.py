"""
This test is to program FreqTable file to external flash with python script

Note: If we are using this script on high loading device Before run this script,
      FPGA interrupt and battery polling needs to be disabled first
"""
import time
import datetime
import argparse
import os
import struct
from bhtx import *

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

    # Specify the FreqTable file name
    parser.add_argument('-f', '--file', default='./FreqTable/SLXD_FTable0_0_1_11.bin')
    
    args = parser.parse_args()

    # start the time calculating
    startTime = datetime.datetime.now()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    tx = BhTx.get_instance()

    tx.unlock()  # switch to dev privilege mode

    #ExternalFlashStartAddr = 0x00280000    # debug only , Backup FPGA image location start addr in external flash
    ExternalFlashSectorSize = 4 * 1024     # external flash sector size is 4K Bytes
    ExternalFlashStartAddr  = 0x00160000   # FreqBand file location start addr in external flash
    FlashAddrIndex = ExternalFlashStartAddr   # Address index starts from ExternalFlashStartAddr
    
    print("If run this script on SLXD, make sure to stop the FPGA and batt polling");  
    ret = tx.send_cmd("stoppolling on")    # stop the fpga interrupt polling 
    time.sleep(1)                          # delay 1 second
    ret = tx.send_cmd("battpoll off")      # stop the batterypolling 
    time.sleep(1)                          # delay 1 second

    binfile = open(args.file, 'rb')           # open the FreqTable file
    size = os.path.getsize(args.file)         # get the FreqTable file size
    print("FreqBand binfile size is {}".format(size))  # print the FreqTable file size

    numofSector = int(size / ExternalFlashSectorSize) + 1    # calculate how many sectors does FreqBand file occupies in external flash
    print("sector number is {}".format(numofSector))         # print the sector numbers
    for j in range(numofSector):
        tx.send_cmd("flasherase 0x{:x}".format(ExternalFlashStartAddr))    # erase all sectors the FreqBand file occupies
        time.sleep(1)                                                      # delay 1 second
        ExternalFlashStartAddr = ExternalFlashStartAddr + ExternalFlashSectorSize

    for i in range(size):
        data = binfile.read(1)                   # get each byte from the FreqBand file
        value = struct.unpack('B',data)          # convert to integer
            #print(value[0])                     # convert to hex and print
            #print(str(value[0]))                # convert to string and print
        tx.send_cmd("flashpoke 0x{:x} {}".format(FlashAddrIndex, str(value[0])))
        FlashAddrIndex = FlashAddrIndex + 1      # Address index increaments


    #end time calculating
    endTime = datetime.datetime.now()

    #print time
    print( 'Time for this script is ' )
    print( endTime - startTime )

    time.sleep(3)                                # delay 3 second
    tx.send_cmd("reset", expect_resp=False)      # after all the FreqBand file, reboot the device

