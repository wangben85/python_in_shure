"""
Convert from *.dat file to *.bin file
"""
import os

import struct

file_dat = 'ble_app_blinky_LED3_init.dat'
file_bin = 'ble_app_blinky_LED3_init.bin'

fn_dat = open(file_dat, "rb")
fn_bin = open(file_bin, "ab+")

size = os.path.getsize(file_dat)
print(size)

data0 = struct.pack('B', 0)
L = []
j = 0
for i in range(size):
    data = fn_dat.read(1)
    value = struct.unpack('B', data)
    print(hex(value[0]))
    a = struct.pack('B', value[0])
    fn_bin.write(a)
    j = j + 1
    if ( j == 15 ):
       j = 0
       fn_bin.write(data0)
       print('\n')
#fn_bin.write(data0)    # need to check whether it is need or not

fn_dat.close()
fn_bin.close()
