import time
import sys
import serial
import re
import telnetlibCallback3 as telnetlib

class CliConn(object):

    DEFAULT_TIMEOUT = 3.0

    def __init__(self, hostname=None, port=None, baud=115200):
        self.hostname = hostname
        self.port = port
        self.baud = baud
        self.buf = b''
        self.ser = None
        self.tn = None

    def __del__(self):
        if self.ser is not None:
            print("close serial port")
            self.ser.close()

    def is_serial(self):
        return self.hostname is None

    def connect(self):  #Serial init and Connect to it
        to = self.DEFAULT_TIMEOUT
        if self.hostname is None:  #Physcial Serial
            self.ser = serial.Serial(self.port,self.baud, bytesize=serial.EIGHTBITS,
                                     parity=serial.PARITY_NONE,
                                     stopbits=serial.STOPBITS_ONE,
                                     timeout=to)
        else:  #Telnet Serial
            self.tn = telnetlib.Telnet(self.hostname, self.port, timeout=to)
            self.tn.read_until(b"ogin: ", timeout=to)
            self.tn.write('blackhawk'.encode('ascii') + b"\r\n")
            self.tn.read_until(b"assword: ", timeout=to)
            self.tn.write('blackhawk'.encode('ascii') + b"\r\n")
            self.tn.read_until(b"CLI> ", timeout=to)

        name = self.port if self.hostname is None else self.hostname
        print("Connected @ {}".format(name))

    def send(self, cmd, newline='\r'):
        if self.hostname is None:
            self.send_serial(cmd,newline)
        else:
            self.send_telnet(cmd,newline)

    def send_serial(self, cmd, newline='\r'):
        cmd += newline
        self.ser.write(cmd.encode('ascii'))

    def send_telnet(self, cmd, newline='\r'):
        cmd += newline
        self.tn.write(cmd.encode('ascii'))

    def expect(self, lst, timeout=None):

        if isinstance(lst, str):
            lst = list(lst)

        if timeout is None:
            timeout = self.DEFAULT_TIMEOUT

        if self.hostname is None:
            ret = list(self.expect_serial(lst,timeout))
        else:
            ret = list(self.expect_telnet(lst,timeout))

        ret[2] = ret[2].decode('ascii')
        return ret

    def expect_telnet(self, lst, timeout=None):
        if timeout is None:
            timeout = self.DEFAULT_TIMEOUT

        lst = [s.encode('ascii') for s in lst]
        return self.tn.expect(lst, timeout)

    def expect_serial(self, lst, timeout=None):
        if timeout is None:
            timeout = self.DEFAULT_TIMEOUT

        # Read until we match expected string or get timeout
        deadline = time.time() + timeout
        while True:
            try:
                buf = self.ser.read()
            except:
                pass
            else:
                self.buf += buf

            # Try to find pattern
            for i, pat in enumerate(lst):
                pat = pat.encode('ascii')
                m = re.search(pat, self.buf, re.MULTILINE)
                if m:
                    ret = (i, m, self.buf[:m.end()])  # it is the CLI command return value as the return value
                                                      # ret[0], ret[1], ret[2]
                    self.buf = b''
                    return ret

            if time.time() > deadline:
                raise Exception("Read timeout!")
                # return (-1, None, self.buf[:])

        self.buf = b''
        raise Exception("Unexpected failure!")
        # return (-1, None, '')

