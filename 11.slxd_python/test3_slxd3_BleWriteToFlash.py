"""
This test is to program BLE image file to external flash with python script

Note: If we are using this script in high loading device(e.g. ADX3 with ShowLink) , Before run this script,
      ShowLink needs to be disabled first
"""
import time
import argparse
import os
import struct
from bhtx import *

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run SLXD3 tests')
    # Set default host is 192.168.1.102
    parser.add_argument('-n', '--hostname',default='192.168.1.102', help='Hostname for CLI connection')
    #parser.add_argument('-n', '--hostname', help='Hostname for CLI connection')
    # Set default COM=8024
    parser.add_argument('-p', '--port', default='8024', help='Port for CLI connection')
    #parser.add_argument('-p', '--port', default='COM10', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b', '--baud', default=115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True)
   
    # Specify the BLE image file name
    # image 1, app and init
    #parser.add_argument('-f', '--file', default='./BLE_Images/slxd3_ble_app_simple1.bin')
    parser.add_argument('-f', '--file', default='./BLE_Images/slxd3_ble_app_simple1.dat')

    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    tx = BhTx.get_instance()

    tx.unlock()  # switch to dev privilege mode

    ExternalFlashSectorSize = 4 * 1024     # external flash sector size is 4K Bytes

    #test on SLXD3
    #image1 App size is 80K, init packet size is 3K
    #ExternalFlashStartAddr  = 0x001A0000   # Ble image file location start addr, ble_app_data
    ExternalFlashStartAddr  = 0x001B4000   # Ble image file location start addr, ble_app_init

    #
    FlashAddrIndex = ExternalFlashStartAddr   # Address index starts from ExternalFlashStartAddr
    
    print("If run this script on SLXD, make sure to stop the FPGA and batt polling");  
    ret = tx.send_cmd("fpgaint off")        # stop the fpga interrupt polling 
    time.sleep(1)                          # delay 1 second
    ret = tx.send_cmd("battpoll off")      # stop the batterypolling 
    time.sleep(1)                          # delay 1 second


    binfile = open(args.file, 'rb')           # open the BLE image file
    size = os.path.getsize(args.file)         # get the BLE image file size
    print("BLE image file size is {}".format(size))  # print the BLE image file size

    numofSector = int(size / ExternalFlashSectorSize) + 1    # calculate how many sectors does BLE image file occupies in external flash
    print("sector number is {}".format(numofSector))         # print the sector numbers
    for j in range(numofSector):
        tx.send_cmd("flasherase 0x{:x}".format(ExternalFlashStartAddr))    # erase all sectors the BLE image file occupies
        ExternalFlashStartAddr = ExternalFlashStartAddr + ExternalFlashSectorSize

    for i in range(size):
        data = binfile.read(1)                   # get each byte from the BLE image file
        value = struct.unpack('B',data)          # convert to integer
            #print(value[0])                     # convert to hex and print
            #print(str(value[0]))                # convert to string and print
        tx.send_cmd("flashpoke 0x{:x} {}".format(FlashAddrIndex, str(value[0])))
        FlashAddrIndex = FlashAddrIndex + 1      # Address index increaments

    tx.send_cmd("reset", expect_resp=False)     # after all the BLE image file, reboot the device

