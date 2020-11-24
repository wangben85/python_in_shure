"""
This command is to set the ZigBee device both Coarse and Fine power calibration values
Note:
1.We need to set the ZigBee power mode first, different power mode have different calibration values, e.g. ZigBee power
  mode low and Mid ...
2.When we have ZigBee power mode changed, the last time device power setting could not be saved, because it will reload
  a default value restored in EEPROM

bool cZigBeeRadio::SetRadioPowerMode( ZigbeePowerMode_t mode )
{
      if ( m_ZigbeePowerMode != mode )
      {
         // Load cal for this power mode and program power for current
         // channel
         LoadZigbeeCal( mode );
         SetRadioPowerLevel( GetChannelPowerCalibration( PhyPlmeGetCurrentChannelRequest(0) ) ); // only PAN 0
         SetRadioFinePowerLevel( GetChannelFinePowerCalibration( PhyPlmeGetCurrentChannelRequest(0) ) ); // only PAN 0
}
"""

import os
import time
from bhtx import BhTx
import argparse

if __name__ == "__main__":

    # @todo Arg parsing here could use some improvement...
    parser = argparse.ArgumentParser(description='Run BHTX integration tests')
    # Set default host is 192.168.1.102
    parser.add_argument('-n', '--hostname',default='192.168.1.102', help='Hostname for CLI connection')
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
    tx = BhTx.get_instance()
    tx.unlock()     # unlock to dev privilege mode
    tx.send_cmd("ls")   # verify the privilege mode switch successfully

    # Coarse Power Calibration value range is 3 ~ 31, it will configure the register Modem_PA_PWR
    zbpwr_Coarselow = '15'
    #zbpwr_Coarselow = '16'
    zbpwr_Coarsemid = '17'
    #zbpwr_Coarsemid = '18'

    # Fine Power Calibration value range is 32 ~ 47, it will configure the register Modem_PA_CAL
    zbfinepwr_Coarselow = '32'
    #zbfinepwr_Coarselow = '33'
    zbfinepwr_Coarsemid = '34'
    #zbfinepwr_Coarsemid = '35'

    print('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!')
    print('First, make sure to set the ZigBee power mode to LOW !')
    print('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!')
    # power low
    tx.send_cmd("zbpwrmode")                     # get the current ZigBee power mode
    tx.send_cmd("zbpwrmode low")                 # set ZigBee power mode to low
    tx.send_cmd("zbpwrmode")                     # check low mode

    tx.send_cmd("zbpwr")                         # get the current Coarse power calibration data at current power mode
    tx.send_cmd("zbpwr %s" % zbpwr_Coarselow)    # set the Coarse power calibration values
    time.sleep(1)                                # delay 1 seconds
    tx.send_cmd("zbpwr")                         # get the current Coarse power calibration data after setting

    tx.send_cmd("zbfinepwr")                      # get the current Coarse power calibration data at current power mode
    tx.send_cmd("zbfinepwr %s" % zbfinepwr_Coarselow)    # set the Coarse power calibration values
    time.sleep(1)                                        # delay 1 seconds
    tx.send_cmd("zbfinepwr")                             # get the current Coarse power calibration data after setting

    # power mid
    print('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!')
    print('Now we should switch the ZigBee power mode to MID !')
    print('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!')
    tx.send_cmd("zbpwrmode")                     # get the current ZigBee power mode
    tx.send_cmd("zbpwrmode mid")                 # set ZigBee power mode to low
    tx.send_cmd("zbpwrmode")                     # check mid mode

    tx.send_cmd("zbpwr")                         # get the current Fine power calibration data at current power mode
    tx.send_cmd("zbpwr %s" % zbpwr_Coarsemid)    # set the Fine power calibration values
    time.sleep(1)                                # delay 1 seconds
    tx.send_cmd("zbpwr")                         # get the current Fine power calibration data after setting

    tx.send_cmd("zbfinepwr")                        # get the current Fine power calibration data at current power mode
    tx.send_cmd("zbfinepwr %s" % zbfinepwr_Coarsemid)    # set the Fine power calibration values
    time.sleep(1)                                        # delay 1 seconds
    tx.send_cmd("zbfinepwr")                             # get the current Fine power calibration data after setting

