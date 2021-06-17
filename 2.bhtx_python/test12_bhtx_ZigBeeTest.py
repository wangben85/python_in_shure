"""
Run the ZigBee Showlink test with the command 'zbed' when rfmute off and rfmute on
"""

import os
import time
from bhtx import *
import argparse
import sys
import csv

# VariantA all the Channels
VariantA_Channel11 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 481 force",
    "zigbee off",
    "telecfreq 11",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel12 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 482 force",
    "zigbee off",
    "telecfreq 12",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel13 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 483 force",
    "zigbee off",
    "telecfreq 13",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel14 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 484 force",
    "zigbee off",
    "telecfreq 14",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel15 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 485 force",
    "zigbee off",
    "telecfreq 15",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel16 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 486 force",
    "zigbee off",
    "telecfreq 16",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel17 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 487 force",
    "zigbee off",
    "telecfreq 17",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel18 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 488 force",
    "zigbee off",
    "telecfreq 18",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel19 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 489 force",
    "su dev dev"
    "zigbee off",
    "telecfreq 19",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel20 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 490 force",
    "zigbee off",
    "telecfreq 20",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel21 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 491 force",
    "zigbee off",
    "telecfreq 21",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel22 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 492 force",
    "zigbee off",
    "telecfreq 22",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel23 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 493 force",
    "zigbee off",
    "telecfreq 23",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel24 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 494 force",
    "zigbee off",
    "telecfreq 24",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel25 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 495 force",
    "zigbee off",
    "telecfreq 25",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantA_Channel26 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 496 force",
    "zigbee off",
    "telecfreq 26",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

# VariantB all the Channels
VariantB_Channel11 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 801.67 force",
    "zigbee off",
    "telecfreq 11",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantB_Channel12 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 803.33 force",
    "zigbee off",
    "telecfreq 12",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantB_Channel13 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 805 force",
    "zigbee off",
    "telecfreq 13",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantB_Channel14 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 806.67 force",
    "zigbee off",
    "telecfreq 14",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantB_Channel15 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 808.33 force",
    "zigbee off",
    "telecfreq 15",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantB_Channel16 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 810 force",
    "zigbee off",
    "telecfreq 16",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

# Variant C all the Channels
VariantC_Channel11 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 801.67 force",
    "zigbee off",
    "telecfreq 11",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel12 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 803.33 force",
    "zigbee off",
    "telecfreq 12",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel13 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 805 force",
    "zigbee off",
    "telecfreq 13",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel14 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 806.67 force",
    "zigbee off",
    "telecfreq 14",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel15 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 808.33 force",
    "zigbee off",
    "telecfreq 15",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel16 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 810 force",
    "zigbee off",
    "telecfreq 16",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel17 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 811.67 force",
    "zigbee off",
    "telecfreq 17",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel18 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 813.33 force",
    "zigbee off",
    "telecfreq 18",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel19 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 815 force",
    "su dev dev"
    "zigbee off",
    "telecfreq 19",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel20 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 816.67 force",
    "zigbee off",
    "telecfreq 20",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel21 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 818.33 force",
    "zigbee off",
    "telecfreq 21",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel22 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 820 force",
    "zigbee off",
    "telecfreq 22",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel23 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 821.67 force",
    "zigbee off",
    "telecfreq 23",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel24 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 823.33 force",
    "zigbee off",
    "telecfreq 24",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel25 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 825 force",
    "zigbee off",
    "telecfreq 25",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]

VariantC_Channel26 = [
    "su dev dev",
    "rfmute off",
    "llfreq 1 826.67 force",
    "zigbee off",
    "telecfreq 26",
    "repeat 10 100 zbed",
    "rfmute on",
    "repeat 10 100 zbed",
]
# quick channel test
cmd_count_test = [
    VariantA_Channel11,
    VariantA_Channel12,
    VariantA_Channel13,
]

# RF variant A
cmd_count_variantA = [
    VariantA_Channel11,
    VariantA_Channel12,
    VariantA_Channel13,
    VariantA_Channel14,
    VariantA_Channel15,
    VariantA_Channel16,
    VariantA_Channel17,
    VariantA_Channel18,
    VariantA_Channel19,
    VariantA_Channel20,
    VariantA_Channel21,
    VariantA_Channel22,
    VariantA_Channel23,
    VariantA_Channel24,
    VariantA_Channel25,
    VariantA_Channel26,
]

# RF variant B
cmd_count_variantB = [
    VariantB_Channel11,
    VariantB_Channel12,
    VariantB_Channel13,
    VariantB_Channel14,
    VariantB_Channel15,
    VariantB_Channel16,
]

# RF variant C
cmd_count_variantC = [
    VariantC_Channel11,
    VariantC_Channel12,
    VariantC_Channel13,
    VariantC_Channel14,
    VariantC_Channel15,
    VariantC_Channel16,
    VariantC_Channel17,
    VariantC_Channel18,
    VariantC_Channel19,
    VariantC_Channel20,
    VariantC_Channel21,
    VariantC_Channel22,
    VariantC_Channel23,
    VariantC_Channel24,
    VariantC_Channel25,
    VariantC_Channel26,
]


def getRfVariant(cmd_count):
    if cmd_count == cmd_count_variantA:
        return 'A'
    elif cmd_count == cmd_count_variantB:
        return 'B'
    else:
        return 'C'


if __name__ == "__main__":

    # @todo Arg parsing here could use some improvement...
    parser = argparse.ArgumentParser(description='Run Black hawk integration tests')
    # set default host
    #parser.add_argument('-n', '--hostname', default='192.168.1.101', help='Hostname for CLI connection')
    parser.add_argument('-n', '--hostname', help='Hostname for CLI connection')
    # Set default COM
    parser.add_argument('-p', '--port', default='8024', help='Port for CLI connection')
    # Set default baud rate
    parser.add_argument('-b', '--baud', default=115200, help='Baud for CLI connection')
    # Set default rf power mode
    parser.add_argument('-m', '--rfmode', default='norm', help='Rf power mode: hi or norm')
    # Set default rf variant
    parser.add_argument('-v', '--rfvar', default='A', help='Rf Variant: A , B or C')
    # Set default IR = True
    parser.add_argument('--noir', dest='ir', action='store_const', const=False, default=True,
                        help='Specify that serial connection is NOT IR')
    # Set data store file
    parser.add_argument('-f', '--file', default='./ZigBee.csv', help='Specify test result output data file')

    args = parser.parse_args()

    # Establish connection to transmitter
    # CLI command line to input the parameters
    # e.g    python test12_bhtx_ZigBeeTest.py -p com6                            # use the serial port
    # e.g.   python test12_bhtx_ZigBeeTest.py -n 192.168.1.103 -p 8024           # use the telnet 8024 port
    # e.g.   python test12_bhtx_ZigBeeTest.py -m hi                              # set rf power mode is hi
    # e.g.   python test12_bhtx_ZigBeeTest.py -v A                               # set rf variant to A
    # note:  No input, hostname = None,  baud = 115200
    BhTx.connect(args.hostname, args.port, args.baud, args.ir)

    # Print system info for this test run
    bhtx = BhTx.get_instance()

    # set the rf power mode
    print(args.rfmode)
    if args.rfmode == 'hi':
        bhtx.send_cmd("rfpow hi")
    elif args.rfmode == 'norm':
        bhtx.send_cmd("rfpow norm")
    else:
        print('RF power mode set is error')
        sys.exit(1)

    # set the rf variant
    print(args.rfvar)
    if args.rfvar == 'A' or args.rfvar == 'a':
        cmd_count = cmd_count_variantA
    elif args.rfvar == 'B' or args.rfvar == 'b':
        cmd_count = cmd_count_variantB
    elif args.rfvar == 'C' or args.rfvar == 'c':
        cmd_count = cmd_count_variantC
    else:
        print('RF variant set is error')
        sys.exit(1)

    # Set the csv file header
    print(cmd_count)
    print(len(cmd_count))
    with open(args.file, 'w+', encoding='utf-8', newline='') as csvfile:
        f_csv = csv.writer(csvfile)
        f_csv.writerow(['rf power mode is %s' % args.rfmode])  # write rf power mode to csv
        variantTemp = getRfVariant(cmd_count)
        f_csv.writerow(['RF variant is %s' % variantTemp])     # write the RF variant to csv
        for j in range(len(cmd_count)):
            for i in range(len(cmd_count[j])):
                if i == 2:                                     # get the current RF frequency
                    bhtx.send_cmd(cmd_count[j][i])
                    cmd_str_split = cmd_count[j][i].split()
                    rffreq = cmd_str_split[2]                  # put the frequency to variable rffreq
                    # print('need to print RF freq')
                    # print(cmd_str_split[2])
                    # f_csv.writerow(['UHF RF Freq is %s Mhz' % cmd_str_split[2]])   # write the RF frequency to csv
                elif i == 4:                                   # get the ZigBee test channel
                    bhtx.send_cmd(cmd_count[j][i])
                    cmd_str_split = cmd_count[j][i].split()
                    # print('need to print ZigBee test channel')
                    # print(cmd_str_split[1])
                    zigbeechannel = cmd_str_split[1]           # put the ZigBee test channel to variable zigbeechannel
                    # f_csv.writerow(['ZigBee test channel is %s ' % cmd_str_split[1]])   # write the ZigBee channel to csv
                elif i < 5 or i == 6:
                    bhtx.send_cmd(cmd_count[j][i])             # send other commands, no care about the response
                elif i == 5:                                   # send the command to get the zbed when rfmute off
                    rfmuteoff_data = bhtx.send_cmd(cmd_count[j][i])
                elif i == 7:                                   # send the command to get the zbed when rfmute on
                    rfmuteon_data = bhtx.send_cmd(cmd_count[j][i])

            # write the header to csv
            f_csv.writerow(['UHF RF freq is %s Mhz and ZigBee test channel is %s' % (rffreq, zigbeechannel)])
            f_csv.writerow(['rf mute off zbed value'] + rfmuteoff_data[1])  # write the rfmute off data to csv
            f_csv.writerow(['rf mute on zbed value'] + rfmuteon_data[1])    # write the rfmute on data to csv
            f_csv.writerow('')  # write the blank line
