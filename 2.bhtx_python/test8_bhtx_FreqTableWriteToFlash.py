"""
This test is to program FreqTable file to external flash with python script
"""
import argparse
import os
import struct
from bhtx import *

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run BHTX tests')
    parser.add_argument('-n', '--hostname', help='Hostname for CLI connection')
    # Set default COM=5
    parser.add_argument('-p', '--port', default='COM5', help='Port for CLI connection')
    # Set default baudrate=115200
    parser.add_argument('-b', '--baud', default=115200, help='Baud for CLI connection')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True)
   
    # Specify the FreqTable file name
    # e.g. ./FreqTable/BH_FreqTable_0_2_0_17.bin is band Table file version BH_FreqTable_0_2_0_17
    # e.g. ./FreqTable/BH_FreqTable_0_2_0_16.bin is band Table file version BH_FreqTable_0_2_0_16
    parser.add_argument('-f', '--file', default='./FreqTable/BH_FreqTable_0_2_0_16.bin')

    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    tx = BhTx.get_instance()

    tx.unlock()  # switch to dev privilege mode

    #ExternalFlashStartAddr = 0x00280000    # debgu only , Backup FPGA image location start addr in external flash
    ExternalFlashSectorSize = 4 * 1024     # external flash sector size is 4K Bytes
    ExternalFlashStartAddr  = 0x00100000   # FreqBand file location start addr in external flash
    FlashAddrIndex = ExternalFlashStartAddr   # Address index starts from ExternalFlashStartAddr

    # debug purpose
    # peek the default value in flash
    #tx.send_cmd("flashpuke 0x{:x} 16".format(ExternalFlashStartAddr))
    # erase flash
    #tx.send_cmd("flasherase 0x{:x}".format(ExternalFlashStartAddr))
    # peek flash after erase
    #tx.send_cmd("flashpuke 0x{:x} 16".format(ExternalFlashStartAddr))

    binfile = open(args.file, 'rb')           # open the FreqTable file
    size = os.path.getsize(args.file)         # get the FreqTable file size
    print("FreqBand binfile size is {}".format(size))  # print the FreqTable file size

    numofSector = int(size / ExternalFlashSectorSize) + 1    # calculate how many sectors does FreqBand file occupies in external flash
    print("sector number is {}".format(numofSector))         # print the sector numbers
    for j in range(numofSector):
        tx.send_cmd("flasherase 0x{:x}".format(ExternalFlashStartAddr))    # erase all sectors the FreqBand file occupies
        ExternalFlashStartAddr = ExternalFlashStartAddr + ExternalFlashSectorSize

    for i in range(size):
        data = binfile.read(1)                  # get each byte from the FreqBand file
        value = struct.unpack('B',data)         # convert to integer
            # print(value[0])                   # convert to hex and print
            # print(str(value[0]))              # convert to string and print
        tx.send_cmd("flashpoke 0x{:x} {}".format(FlashAddrIndex, str(value[0])))
        FlashAddrIndex = FlashAddrIndex + 1     # Address index increaments

    tx.send_cmd("reboot", expect_resp=False)    # after all the FreqBand file, reboot the device

