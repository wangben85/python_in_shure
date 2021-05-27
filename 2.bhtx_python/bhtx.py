from cliconn import CliConn
from enum import Enum
import re


class TxType(Enum):
    BASIC_BP    = 1
    ENHANCED_BP = 2
    BASIC_PLUGON = 3
    BASIC_HH    = 4
    ENHANCED_HH = 6
    FD_HH       = 7
    MICRO_BP    = 8
    UNKNOWN     = -1

class RfVariant(Enum):
    A       = 0
    B       = 1
    C       = 2
    UNKNOWN = -1

class CalTable(Enum):
    PRIMARY = 0
    SECONDARY = 1
    COMBINED = 2

class RfPowerControlMode(Enum):
    MUTE       = 0
    STANDBY    = 1
    LOW        = 2
    NORM       = 3
    HIGH       = 4
    CC_MUTE    = 5  #CC_MODES are only used for FD_HH transmitter
    CC_STANDBY = 6
    CC_LOW     = 7
    CC_NORM    = 8
    CC_HIGH    = 9
    UNKNOWN    = 10

class RfPower(Enum):
    LOW = 2
    NORM = 0
    HIGH = 1
    UNKNOWN = 3

    def __str__(self):
        return {2:'low', 0:'norm', 1:'hi', 3:'unknown'}[self.value]

class ModemMode(Enum):
    STD = 0
    HD = 1
    UNKNOWN = 2

    def __str__(self):
        return {0:'std', 1:'hd', 2:'UNKNOWN'}[self.value]

PRIMARY_CARRIER = 1
SECONDARY_CARRIER = 2

LOG_FILE_PATH = 'Cli_Log_File.txt'   # it is the CLI command LOG file path name

class BhTx(object):

    DEFAULT_TIMEOUT = 3.0
    instance = None

    def __init__(self, hostname, port, baud, ir):
        self.ir = ir
        self.cli = CliConn(hostname, port, baud)
        self.cli.connect()

    @classmethod
    def get_instance(cls):
        if cls.instance is None:
            raise Exception('Instance must be created with "connect"')

        return cls.instance

    @classmethod
    def connect(cls, hostname=None, port=None, baud=115200, ir=True):
        if cls.instance is not None:
            del cls.instance

        # cls.instance = cls(hostname, port, baud, ir)
        # They are the same
        cls.instance = BhTx(hostname, port, baud, ir)
        cls.instance.unlock()

    def unlock(self):
        if self.send_cmd("su dev dev"):
            print("Transmitter unlocked")

    def send_cmd(self, cmd, timeout=None, expect_resp=True):
        """
        Send CLI command with specified timeout.

        A successful command will result in a prompt with no
        [Failed] message appearing.

        Returns pair where first element is boolean of command
        success and second element is response text.
        """
        if timeout is None:
            timeout = self.DEFAULT_TIMEOUT

        print("CLI> " + cmd)   # print out the CLI command to be sent

        self.cli.send(cmd)

        # Wait for command response by waiting for prompt. If [Failed] in
        # the response then the command failed
        if expect_resp:
            resp = self.cli.expect(['CLI> '], timeout)
            success = ((resp[0] != -1) and
                (None == re.search('\[Failed\]', resp[2], re.MULTILINE)))

            # IR dongle echos cmd back, so strip it from response
            # For some reason, we're having to do a double trim...
            if self.cli.is_serial():
                resp[2] = re.sub('^\s*{}\s*'.format(cmd), '', resp[2])
                if self.ir:
                    resp[2] = re.sub('^\s*{}\s*'.format(cmd), '', resp[2])

            # Split on newlines and remove last terminating prompt
            lines = resp[2].splitlines()
            lines = lines[:-1]
            if len(lines):
                print("\n".join(lines))
        else:
            success = True
            lines = ['']

        return [success, lines]

    def send_cmd_log(self, cmd, timeout=None, expect_resp=True, expect_record=True):
        """
        Send CLI command with specified timeout and record the feedback into log file.

        A successful command will result in a prompt with no
        [Failed] message appearing.

        Returns pair where first element is boolean of command success and second element is response text.
        """
        if timeout is None:
            timeout = self.DEFAULT_TIMEOUT

        print("CLI> " + cmd)   # print out the CLI command to be sent

        self.cli.send(cmd)

        # Wait for command response by waiting for prompt. If [Failed] in
        # the response then the command failed
        if expect_resp:
            resp = self.cli.expect(['CLI> '], timeout)
            success = ((resp[0] != -1) and
                (None == re.search('\[Failed\]', resp[2], re.MULTILINE)))

            # IR dongle echos cmd back, so strip it from response
            # For some reason, we're having to do a double trim...
            if self.cli.is_serial():
                resp[2] = re.sub('^\s*{}\s*'.format(cmd), '', resp[2])
                if self.ir:
                    resp[2] = re.sub('^\s*{}\s*'.format(cmd), '', resp[2])

            # Split on newlines and remove last terminating prompt
            lines = resp[2].splitlines()
            lines = lines[:-1]
            if len(lines):
                print("\n".join(lines))
        else:
            success = True
            lines = ['']

        # if expect_record = true, means we need to record the CLI command feedback, write them to Log file
        if expect_record:
            fLog = open(LOG_FILE_PATH, 'a+')  # 'a+' means it will overwrite original file
            fLog.write(cmd)   # write command to Log file first
            fLog.write('\n')
            fLog.write(' '.join(lines))  # write command response to Log file later
            fLog.write('\n')
            fLog.write('\n')
            fLog.close()

        return [success, lines]

    def set_freq(self, carrier, freq):
        """
        Set frequency via llfreq, so it's not impacted by band reqs
        """
        return self.send_cmd("llfreq {} {:0.3f}".format(carrier, freq / 1000.0))

    def set_power(self, power):
        """
        Set power tx power.
        """
        return self.send_cmd("rfpow {}".format(power))
   
    def get_power(self):
        """
        Return transmitter rf output power mode
        """
        ret = RfPower.UNKNOWN
        resp = self.send_cmd('rfpow')
        if resp:
             ret = resp[1][0]   # working
        return ret

    def set_rfcar2(self, enable):
        """
        Set secondary RF carrier enable
        """
        return self.send_cmd("rfcar2 {}".format("on" if enable else "off"))

    def get_type(self):
        """
        Return transmitter type
        """
        ret = TxType.UNKNOWN
        resp = self.send_cmd('txtype')
        if resp:
            ret = TxType(int(resp[1][0].split(' ')[0]))  # working
            #ret = resp[1][0].split(' ')[0]  # working
        return ret

    def get_band(self):
        """
        Return transmitter type
        """
        ret = ""
        resp = self.send_cmd('band')
        if resp:
            ret = resp[1][0].split(' ')[0]
        return ret

    def get_variant(self):
        """
        Return transmitter variant
        """
        ret = RfVariant.UNKNOWN
        resp = self.send_cmd('rfvariant')
        if resp:
            ret = RfVariant[resp[1][0].split(' ')[0]]
        return ret

    def set_modem(self, modem):
        """
        Set transmitter modem mode
        """
        return self.send_cmd("modem {}".format(modem))

    def get_modem(self):
        """
        Return transmitter modem mode
        """
        ret = ModemMode.UNKNOWN
        resp = self.send_cmd('modem')
        if resp:
            #ret = ModemMode[resp[1][0]]
            ret = resp[1][0]
        return ret

    def parse_cal_line(self, line):
        """
        Parse line from calibration table
        """
        pat = '\d+:\s+(?P<lbl>\w+)?\s+(?P<values>.*)'

        pwr = RfPower.UNKNOWN
        calvals = {}

        m = re.match(pat, line)
        if m:
            re_dict = m.groupdict()
            lbl = re_dict['lbl']
            if   lbl == 'low':  pwr = RfPower.LOW
            elif lbl == 'norm': pwr = RfPower.NORM
            elif lbl == 'hi':   pwr = RfPower.HIGH
            if pwr != RfPower.UNKNOWN:
                for fidx, pair in enumerate(re_dict['values'].split(' ')):
                    cal_val = [int(scale, 16) for scale in pair.split(',')]
                    calvals[fidx] = cal_val

        return [pwr, calvals]

    def get_cal(self, table):
        """
        Return calibration for specified table
        """
        cmd = ''
        if table == CalTable.COMBINED:
            cmd = 'scrfcal'
        elif table == CalTable.PRIMARY:
        #elif table == 0:
            cmd = 'rfcal 1'
        elif table == CalTable.SECONDARY:
            cmd = 'rfcal 2'

        cal = {}
        resp = self.send_cmd(cmd)
        if resp:
            lines = resp[1][2:]
            for line in lines:
                ret = self.parse_cal_line(line)
                cal[ret[0]] = ret[1]

        return cal

    def get_current_rf_cal(self, carrier):
        """
        Return current rf calibration for specified carrier as pair of integer
        scales (asic, fpga).
        """
        cal = []
        for dev in ['asic', 'fpga']:
            resp = self.send_cmd("rfscale {} {}".format(dev, carrier))
            if resp:
                cal.append(int(resp[1][0], 16))
        return cal

    def fpga_peek(self, addr):
        """
        Return fpga register contents
        """
        ret = 0
        resp = self.send_cmd("fpgapeek 0x{:02X}".format(addr))
        if resp:
            ret = int(resp[1][0].split(' ')[1], 16)
        return ret

#Class init also works below
"""
BhTx(None,'COM6',115200,True)
BhTx.connect(None, 'COM6', 115200, True)
bhtx = BhTx.get_instance()
bhtx.send_cmd("verfpga")
######################################################

##############Overal commands test####################
testbh = BhTx(None,'COM6',115200,True)
#testbh.send_cmd("ls")
testbh.unlock()
#testbh.send_cmd("ls")
#CLI commands test
testbh.set_freq(1,480000)   # set the carrier 1 RF frequency to 480Mhz
testbh.set_power('hi')      # set RF power to hi level
testbh.get_type()           # get the Tx type
testbh.get_band()           # get the Tx band
testbh.get_variant()        # get the Tx RF variant:
testbh.set_power('norm')     # set RF power to norm level
testbh.get_cal(CalTable.PRIMARY)   # load the calibration value
testbh.get_current_rf_cal(1)
testbh.fpga_peek(0x0)
"""
"""
##########Modem test
testbh = BhTx(None,'COM6',115200,True)
testbh.unlock()
testbh.get_modem()          # get the RF modem mode , STD or HD
testbh.set_modem(ModemMode.HD)     # set the RF modem mode , STD or HD
testbh.get_modem()          # get the RF modem mode , STD or HD
testbh.set_modem(ModemMode.STD)     # set the RF modem mode , STD or HD
testbh.get_modem()          # get the RF modem mode , STD or HD
"""
"""
##########Rfpower test
testbh = BhTx(None,'COM6',115200,True)
testbh.unlock()
testbh.send_cmd("oledvcc on 255")
testbh.set_power(RfPower.NORM)          # get the RF power mode , low ,norm or high
testbh.get_power()                      # get the RF power mode , low ,norm or high
testbh.set_power(RfPower.HIGH)          # get the RF power mode , low ,norm or high
testbh.get_power()                      # get the RF power mode , low ,norm or high
testbh.set_power(RfPower.LOW)           # get the RF power mode , low ,norm or high
testbh.get_power()                      # get the RF power mode , low ,norm or high
"""
