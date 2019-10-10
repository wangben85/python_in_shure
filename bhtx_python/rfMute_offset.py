from bhtx import *
import sys
import time
import enum
import csv

class HdqCmds(enum.IntEnum):
    TTECP_ATRATE = 0x04,
    VOLTAGE      = 0x08,
    POWER        = 0x24,
    TTECP        = 0x26,
    CHARGE       = 0x2C

    def __str__(self):
        return {0x04 : 'TTECP_ATRATE',
                0x08 : 'VOLTAGE',
                0x24 : 'POWER',
                0x26 : 'TTECP',
                0x2C : 'CHARGE'}[self.value]

error_count = 0
def send_cmd(tx, cmd, expect_resp=True):
    global error_count

    resp = ''
    try:
        resp = tx.send_cmd(cmd, expect_resp)
    except Exception as e:
        error_count += 1
        if error_count >= 3:
            sys.exit('Exceeded 3 read errors')
    else:
        error_count = 0

    return resp

def get_hdq_param(tx, cmd):

    global error_count

    ret = send_cmd(tx, "hdqpeek 0x{:02X}".format(cmd))
    if ret:
        try:
            ret = int(ret[1][0].split(' ')[1], 16)  # change from 0x value to mW,mA
        except Exception as e:
            ret = 0
        else:
            if cmd == HdqCmds.POWER:
                if ret > 0x7FFF:
                    ret -= 0x10000

    return ret

def get_power(tx):
    power = []
    for i in range(0,5):
        p = get_hdq_param(tx, HdqCmds.POWER)
        if p:
            power.append(p)
        time.sleep(1)

    return sum(power, 0.0) / len(power)

if __name__ == "__main__":

    BhTx.connect(None, 'COM6', 115200, True)
    tx = BhTx.get_instance()

    # Get type as this changes are polling behavior due to different batteries
    txtype = tx.get_type()

    send_cmd(tx, 'su dev dev')
    send_cmd(tx, 'asserts off')

    cmds = [ 'rfpow low', 'rfpow norm', 'rfpow hi' ]

    state = ['rfcar2 off']  # For transmitter except FD HH, the state is only "rfcar2 off"
    if TxType.FD_HH == txtype:
        state = [ 'rfcar2 on', 'rfcar2 off' ]

    # Mute, wait five seconds, record power
    send_cmd(tx, 'rfmute on')
    time.sleep(5)
    mute_power = get_power(tx)  # the power consumption in rfmute state
    send_cmd(tx, 'rfmute off')

    filename = 'muteoffsets-{}.csv'.format(txtype)
    with open(filename, 'w') as csvfile:
        fieldnames = ['cmd', 'pow', 'offset']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()  # write the csv file table header "cmd", "pow","offset"

        # Write line for each power
        for s in state:
            send_cmd(tx, s)

            for c in cmds:
                send_cmd(tx, c)
                time.sleep(5)
                p = get_power(tx)  # the power consumption in different power state, e.g. low, norm, hi

                # Write to logfile
                data = {}
                data['cmd'] = '{}:{}'.format(s, c) # 's' = state, 'c' = cli command
                data['pow'] = p
                data['offset'] = mute_power - p
                writer.writerow(data)
        
        # comment out the LCD test for AD3
        # Get LCD backlight offset too
        # send_cmd(tx, 'led lcd on')
        #time.sleep(5)
        #lcd_power = get_power(tx)
        #send_cmd(tx, 'led lcd off')
        #time.sleep(5)
        #p = get_power(tx)

        # Write to logfile
        #data = {}
        #data['cmd'] = 'led lcd on'
        #data['pow'] = lcd_power
        #data['offset'] = lcd_power - p
        #writer.writerow(data)


