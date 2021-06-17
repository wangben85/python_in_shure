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
    parser = argparse.ArgumentParser(description='Run BHTX tests')
    parser.add_argument('-n', '--hostname', help='Hostname for CLI connection')
    # Set default COM=5
    parser.add_argument('-p', '--port', default='COM10', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b', '--baud', default=115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True)
   
    # Specify the BLE image file name
    # image 1, app and init
    #parser.add_argument('-f', '--file', default='./BLE_Images/ble_app_data.bin')
    #parser.add_argument('-f', '--file', default='./BLE_Images/ble_app_init.dat')
    # image2, app and init
    #parser.add_argument('-f', '--file', default='./BLE_Images/ble_app_uart_slave_data.bin')
    parser.add_argument('-f', '--file', default='./BLE_Images/ble_app_uart_slave_init.dat')

    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    tx = BhTx.get_instance()

    tx.unlock()  # switch to dev privilege mode

    ExternalFlashSectorSize = 4 * 1024     # external flash sector size is 4K Bytes
    #test on AD3
    #image1 App size is 15K, init packet size is 3K( 0x284800 - 0x283c00)
    #ExternalFlashStartAddr  = 0x00280000   # Ble image file location start addr, ble_app_data
    #ExternalFlashStartAddr  = 0x00283c00   # Ble image file location start addr, ble_app_init
    #image2 size is 30K, init packet size is 3K ( 0x28c000 - 0x28cc00)
    #ExternalFlashStartAddr  = 0x00284800   # Ble image file location start addr, ble_app_uart_data
    ExternalFlashStartAddr  = 0x0028c000   # Ble image file location start addr, ble_app_uart_init

    #test on SLXD3
    #image1 App size is 15K, init packet size is 3K( 0x1A4800 - 0x1A3c00)
    #ExternalFlashStartAddr  = 0x001A0000   # Ble image file location start addr, ble_app_data
    #ExternalFlashStartAddr  = 0x001A3c00   # Ble image file location start addr, ble_app_init
    #image2 size is 30K, init packet size is 3K ( 0x1Ac000 - 0x1Acc00)
    #ExternalFlashStartAddr  = 0x001A4800   # Ble image file location start addr, ble_app_uart_data
    #ExternalFlashStartAddr  = 0x001Ac000   # Ble image file location start addr, ble_app_uart_init

    #
    FlashAddrIndex = ExternalFlashStartAddr   # Address index starts from ExternalFlashStartAddr
    
    print("If run this script on ADX3, make sure the ShowLink network is disable");  
    ret = tx.send_cmd("zigbee")    # get the respnse of ZigBee command 
    status = ret[1][0]             # get the ZigBee status
    if ( status == 'on'):
      print("ZigBee is ON")
      print("Disable ZigBee First")
      tx.send_cmd("zigbee off")    # set the ZigBee Off
      time.sleep(1)                # delay 1 second
    else:
      print("ZigBee is OFF")
    
    #debug purpose
    #peek the default value in flash
    tx.send_cmd("flashpuke 0x{:x} 16".format(ExternalFlashStartAddr))
    #erase flash
    tx.send_cmd("flasherase 0x{:x}".format(ExternalFlashStartAddr))
    #peek flash after erase
    tx.send_cmd("flashpuke 0x{:x} 16".format(ExternalFlashStartAddr))


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

    tx.send_cmd("reboot", expect_resp=False)     # after all the BLE image file, reboot the device

