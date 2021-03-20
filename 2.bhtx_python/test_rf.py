import unittest
import re
from cliconn import CliConn
from bhtx import *

class test(unittest.TestCase): # included into the unit test list
#class test(object): # remove it from the unit test list

    bhtx = None

    @classmethod
    def setUpClass(cls):
        cls.bhtx = BhTx.get_instance()

    def setUp(self):
        """
        Set expected device defaults before starting any test
        """
        self.assertTrue(self.bhtx.set_modem(ModemMode.STD))
        self.assertTrue(self.bhtx.set_power(RfPower.NORM))
        self.assertTrue(self.bhtx.send_cmd('rfmute off'))
        self.assertTrue(self.bhtx.set_rfcar2(False))

    def get_rf_power_control_mode(self):
        """
        Return contents of rf power control register
        """
        return self.bhtx.fpga_peek(0x182)

    def is_asic_enabled(self, carrier):  # check if the primary or secondary RF ASIC is enabled through FPAG peek
        """
        Return true if asic for specified carrier is enabled
        """
        en1 = 1 << 31
        en2 = 1 << 30
        reg = self.bhtx.fpga_peek(0x181)
        msk = en1 if carrier == PRIMARY_CARRIER else en2
        return reg & msk

    cal_powers = [RfPower.LOW, RfPower.NORM, RfPower.HIGH]
    cal_freqs = {}
    cal_freqs[RfVariant['A']] = (480000, 510000, 550000, 590000, 620000)
    cal_freqs[RfVariant['B']] = (610000, 650000, 700000, 750000, 800000)
    cal_freqs[RfVariant['C']] = (770000, 800000, 850000, 900000, 930000)

    def expectedPowerCtrlMode(self, txtype, power=RfPower.NORM, scEn=True, rfmute=False):
        """
        Return expected register value for rf control mode
        """
        ret = 0

        mode = RfPowerControlMode.UNKNOWN
        if txtype == TxType.FD_HH and not scEn:
            if rfmute:
                mode = RfPowerControlMode.CC_MUTE
            else:
                if   power == RfPower.LOW:  mode = RfPowerControlMode.CC_LOW
                elif power == RfPower.NORM: mode = RfPowerControlMode.CC_NORM
                elif power == RfPower.HIGH: mode = RfPowerControlMode.CC_HIGH
        else:
            if rfmute:
                mode = RfPowerControlMode.MUTE
            else:
                if   power == RfPower.LOW:  mode = RfPowerControlMode.LOW
                elif power == RfPower.NORM: mode = RfPowerControlMode.NORM
                elif power == RfPower.HIGH: mode = RfPowerControlMode.HIGH
        self.assertNotEqual(mode, RfPowerControlMode.UNKNOWN)

        if txtype == TxType.ENHANCED_BP:
            if   mode == RfPowerControlMode.LOW:      ret = 0x61011050
            elif mode == RfPowerControlMode.NORM:     ret = 0x63011050
            elif mode == RfPowerControlMode.HIGH:     ret = 0x63011010
            elif mode == RfPowerControlMode.STANDBY:  ret = 0x60011070
            elif mode == RfPowerControlMode.MUTE:     ret = 0x60001070
        elif txtype == TxType.MICRO_BP:
            if   mode == RfPowerControlMode.LOW:      ret = 0x20001010
            elif mode == RfPowerControlMode.NORM:     ret = 0x20001010
            elif mode == RfPowerControlMode.HIGH:     ret = 0x80001010
            elif mode == RfPowerControlMode.STANDBY:  ret = 0x20001010
            elif mode == RfPowerControlMode.MUTE:     ret = 0x00001010
        elif txtype == TxType.ENHANCED_HH:
            if   mode == RfPowerControlMode.LOW:      ret = 0x60411050
            elif mode == RfPowerControlMode.NORM:     ret = 0x60C11050
            elif mode == RfPowerControlMode.HIGH:     ret = 0x60C11010
            elif mode == RfPowerControlMode.STANDBY:  ret = 0x60011070
            elif mode == RfPowerControlMode.MUTE:     ret = 0x60001070
        elif txtype == TxType.FD_HH:
            if   mode == RfPowerControlMode.LOW:         ret = 0xDD411010
            elif mode == RfPowerControlMode.NORM:        ret = 0xDFC11050
            elif mode == RfPowerControlMode.HIGH:        ret = 0xDFC11010
            elif mode == RfPowerControlMode.MUTE:        ret = 0xDC001070
            elif mode == RfPowerControlMode.STANDBY:     ret = 0xDC011070
            elif mode == RfPowerControlMode.CC_LOW:      ret = 0x75411070
            elif mode == RfPowerControlMode.CC_NORM:     ret = 0x77C11070
            elif mode == RfPowerControlMode.CC_HIGH:     ret = 0x77C11010
            elif mode == RfPowerControlMode.CC_MUTE:     ret = 0x74001070
            elif mode == RfPowerControlMode.CC_STANDBY:  ret = 0x74011070
        elif txtype in [TxType.BASIC_BP, TxType.BASIC_HH, TxType.BASIC_PLUGON]:   # Basic transmitter only
            if   mode == RfPowerControlMode.LOW:      ret = 0x60001010
            elif mode == RfPowerControlMode.NORM:     ret = 0x60001010
            elif mode == RfPowerControlMode.HIGH:     ret = 0xC0011010
            elif mode == RfPowerControlMode.MUTE:     ret = 0x40001010
            elif mode == RfPowerControlMode.STANDBY:  ret = 0x40001010
        return ret

    def calTableTest(self, table):
        """
        Test helper which tests applying all values in specified calibration
        table
        """

        cal = self.bhtx.get_cal(table)  # restore the calibration data
        txtype = self.bhtx.get_type()
        var = self.bhtx.get_variant()
        band = self.bhtx.get_band()
        scEn = txtype == TxType.FD_HH and table != CalTable.COMBINED
        carrier = PRIMARY_CARRIER if table != CalTable.SECONDARY else SECONDARY_CARRIER

        for pwr in self.cal_powers:

            self.assertTrue(self.bhtx.send_cmd("llrfpow {}".format(pwr))) # low level call to set the RF power level, set FPGA/ASIC, do not use marketing limits

            # Check power control mode
            act = self.get_rf_power_control_mode()  # Return contents of rf power control register by FPGA register 0x182
            exp = self.expectedPowerCtrlMode(txtype, pwr, scEn)  # set the expected values in 0x182
            self.assertEqual(act, exp)

            # Check cal for each frequency index
            for idx, freq in enumerate(self.cal_freqs[var]):
                self.assertTrue(self.bhtx.set_freq(carrier, freq))
                cur_cal = self.bhtx.get_current_rf_cal(carrier) # Return current rf calibration for specified carrier as pair of integer scales (asic, fpga).
                self.assertEqual(cur_cal, cal[pwr][idx])

    def isCalValid(self, txtype):  # Check if Calibration table is valid, data could be read

        resp = []
        resp.append(self.bhtx.send_cmd('rfcal 1'))
        if txtype == TxType.FD_HH:
            resp.append(self.bhtx.send_cmd('rfcal 2'))
            resp.append(self.bhtx.send_cmd('scrfcal'))

        ret = True
        for r in resp:
            for line in r[1]:
                m = re.search("RF calibration not valid", line)
                if m:
                    ret = False
                    break;

        return ret

    def testSingleCarrierCal(self):  # test case 1

        txtype = self.bhtx.get_type()

        if txtype is TxType.FD_HH:
            self.skipTest("FD uses dual carrier test")
        if not self.isCalValid(txtype):  # calibration data must be valid
            self.skipTest("Cal must be valid to do this test")

        # Ensure carrier enabled
        carrier = PRIMARY_CARRIER
        if txtype == TxType.ENHANCED_HH:
            carrier = SECONDARY_CARRIER
        self.assertTrue(self.is_asic_enabled(carrier))

        # Test calibration
        self.calTableTest(CalTable.PRIMARY)

    def testDualCarrierCal(self): # test case 2, it will be skipped by AD3

        txtype = self.bhtx.get_type()

        if txtype is not TxType.FD_HH:
            self.skipTest("Only used on FD")
        if not self.isCalValid(txtype):
            self.skipTest("Cal must be valid to do this test")

        # Test dual carrier
        self.assertTrue(self.bhtx.set_rfcar2(True))
        self.assertTrue(self.is_asic_enabled(PRIMARY_CARRIER))
        self.assertTrue(self.is_asic_enabled(SECONDARY_CARRIER))
        self.calTableTest(CalTable.PRIMARY)
        self.calTableTest(CalTable.SECONDARY)

        # Test single carrier
        self.assertTrue(self.bhtx.set_rfcar2(False))
        self.assertTrue(self.is_asic_enabled(PRIMARY_CARRIER))
        self.assertFalse(self.is_asic_enabled(SECONDARY_CARRIER))
        self.calTableTest(CalTable.COMBINED)

    def getDefaultCal(self, txtype, pwr, scEn):

        cal = {}
        cal[TxType.BASIC_BP]    = {'low':(0x0B, 0x60), 'norm':(0x03, 0x60), 'hi':(0x0D, 0x60)}
        cal[TxType.BASIC_PLUGON]= {'low':(0x0B, 0x60), 'norm':(0x03, 0x60), 'hi':(0x0D, 0x60)}
        cal[TxType.BASIC_HH]    = {'low':(0x0B, 0x60), 'norm':(0x0F, 0x60), 'hi':(0x0A, 0x60)}
        cal[TxType.ENHANCED_BP] = {'low':(0x0B, 0x60), 'norm':(0x0F, 0x60), 'hi':(0x0A, 0x60)}
        cal[TxType.ENHANCED_HH] = {'low':(0x0B, 0x60), 'norm':(0x0D, 0x60), 'hi':(0x09, 0x60)}
        cal[TxType.FD_HH]       = {'low':(0x07, 0x60), 'norm':(0x0A, 0x60), 'hi':(0x08, 0x60)}
        cal[TxType.MICRO_BP]    = {'low':(0x0C, 0x60), 'norm':(0x06, 0x60), 'hi':(0x03, 0x60)}
        cc_cal = {}
        cc_cal[TxType.FD_HH]    = {'low':(0x10, 0x60), 'norm':(0x0B, 0x60), 'hi':(0x05, 0x60)}

        if txtype == TxType.FD_HH and not scEn:
            return cc_cal[txtype][str(pwr)]
        else:
            return cal[txtype][str(pwr)]

    def testDefaultCal(self):  # test case 3, it will be skipped if the calibration table is valid

        txtype = self.bhtx.get_type()
        if self.isCalValid(txtype):  # calibration data must be invalid to test default calibration
            self.skipTest("Cal must be invalid to do this test")

        # Handle FD differently becuase needs to test multiple carriers and combined
        # carrier mode
        if txtype == TxType.FD_HH:
            # Second Carrier is enable, so Test dual carrier first
            self.assertTrue(self.bhtx.set_rfcar2(True))
            for pwr in self.cal_powers:
                self.assertTrue(self.bhtx.send_cmd("llrfpow {}".format(pwr)))
                self.assertSequenceEqual(self.getDefaultCal(txtype, pwr, True),
                    self.bhtx.get_current_rf_cal(PRIMARY_CARRIER))
                self.assertSequenceEqual(self.getDefaultCal(txtype, pwr, True),
                    self.bhtx.get_current_rf_cal(SECONDARY_CARRIER))

            # Second Carrier is disable, so Test combined carrier
            self.assertTrue(self.bhtx.set_rfcar2(False))
            for pwr in self.cal_powers:
                self.assertTrue(self.bhtx.send_cmd("llrfpow {}".format(pwr)))
                self.assertSequenceEqual(self.getDefaultCal(txtype, pwr, False),
                    self.bhtx.get_current_rf_cal(PRIMARY_CARRIER))

        # All other devices only have single carrier
        else:
            for pwr in self.cal_powers:
                self.assertTrue(self.bhtx.send_cmd("llrfpow {}".format(pwr)))
                self.assertSequenceEqual(self.getDefaultCal(txtype, pwr, False), # expected data to be compare
                    self.bhtx.get_current_rf_cal(PRIMARY_CARRIER))   # get default calibration data from hardware(asic, FPGA)

    def testAdjustedCal(self):  # test case 4, adjusted calibration data test, only apply to high 20mW

        txtype = self.bhtx.get_type()

        if not self.isCalValid(txtype):
            self.skipTest("Cal must be valid to do this test")

        if txtype == TxType.MICRO_BP:
            self.skipTest("Micro has no adjusted powers")

        # If TxType is FDHH, then Adjusted power is only available in combined carrier mode 
        if txtype == TxType.FD_HH:
            self.bhtx.set_rfcar2(False)
            cal = self.bhtx.get_cal(CalTable.COMBINED)
        else:
            cal = self.bhtx.get_cal(CalTable.PRIMARY)

        # Adjustment only concerns 20 mW power. Use llrfpow so as to not worry about
        # band/maxpow restrictions
        self.assertTrue(self.bhtx.send_cmd("llrfpow 20"))

        # Check cal for each frequency index
        var = self.bhtx.get_variant()
        for idx, freq in enumerate(self.cal_freqs[var]):
            self.assertTrue(self.bhtx.set_freq(PRIMARY_CARRIER, freq))
            asic, fpga = self.bhtx.get_current_rf_cal(PRIMARY_CARRIER)

            # Reverse cal adjustment and make sure it matches whats in the table
            if txtype in [TxType.BASIC_BP, TxType.BASIC_HH, TxType.BASIC_PLUGON]:  asic -= 2
            elif txtype in [TxType.ENHANCED_BP, TxType.ENHANCED_HH]:  asic -= 3
            elif txtype == TxType.FD_HH:                              asic -= 4

            self.assertEqual([asic, fpga], cal[RfPower.HIGH][idx])

    def testMute(self):

        txtype = self.bhtx.get_type()

        # Ensure we have normal power
        self.assertTrue(self.bhtx.set_power(RfPower.NORM))

        if txtype == TxType.FD_HH:
            for scEn in [True, False]:
                self.assertTrue(self.bhtx.set_rfcar2(scEn))

                # Muted
                self.assertTrue(self.bhtx.send_cmd('rfmute on'))
                self.assertEqual(self.get_rf_power_control_mode(),
                        self.expectedPowerCtrlMode(txtype, scEn=scEn, rfmute=True))
                self.assertFalse(self.is_asic_enabled(PRIMARY_CARRIER))
                self.assertFalse(self.is_asic_enabled(SECONDARY_CARRIER))
                asic, fpga = self.bhtx.get_current_rf_cal(PRIMARY_CARRIER)
                self.assertEqual(fpga, 0)
                asic, fpga = self.bhtx.get_current_rf_cal(SECONDARY_CARRIER)
                self.assertEqual(fpga, 0)

                # Unmuted
#                self.assertTrue(self.bhtx.send_cmd('rfmute off'))
                #self.assertEqual(self.get_rf_power_control_mode(),
                        #self.expectedPowerCtrlMode(txtype, power=RfPower.NORM, scEn=scEn))
                #self.assertTrue(self.is_asic_enabled(PRIMARY_CARRIER))
                #asic, fpga = self.bhtx.get_current_rf_cal(PRIMARY_CARRIER)
                #self.assertNotEqual(fpga, 0)
                #if scEn:
                    #self.assertTrue(self.is_asic_enabled(SECONDARY_CARRIER))
                    #asic, fpga = self.bhtx.get_current_rf_cal(SECONDARY_CARRIER)
                    #self.assertNotEqual(fpga, 0)

        # All other devices only have single carrier
        else:

            # Muted
            self.assertTrue(self.bhtx.send_cmd('rfmute on'))
            self.assertEqual(self.get_rf_power_control_mode(),
                    self.expectedPowerCtrlMode(txtype, rfmute=True))
            self.assertFalse(self.is_asic_enabled(PRIMARY_CARRIER))
            asic, fpga = self.bhtx.get_current_rf_cal(PRIMARY_CARRIER)
            self.assertEqual(fpga, 0)

            # Unmuted
#            self.assertTrue(self.bhtx.send_cmd('rfmute off'))
            #self.assertEqual(self.get_rf_power_control_mode(),
                     #self.expectedPowerCtrlMode(txtype, power=RfPower.NORM, rfmute=False))
            #self.assertTrue(self.is_asic_enabled(PRIMARY_CARRIER))
            #asic, fpga = self.bhtx.get_current_rf_cal(PRIMARY_CARRIER)
            #self.assertNotEqual(fpga, 0)

    def testHdPowerLimit(self):

        self.assertTrue(self.bhtx.set_modem(ModemMode.HD))

        # Ensure only low power can be set. Note, we cannot use the set_power
        # method as it uses low-level power set. We want to use app level so
        # that restrictions are observed.
        self.assertTrue(self.bhtx.send_cmd("rfpow low")[0])
        self.assertFalse(self.bhtx.send_cmd("rfpow norm")[0])
        self.assertFalse(self.bhtx.send_cmd("rfpow hi")[0])




