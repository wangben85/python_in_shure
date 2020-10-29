"""
Convert from *.dat file to *.bin file
"""
import os

import struct

file_dat = 'ble_led3_init.dat'
file_bin = 'ble_led3_init.bin'

fn_dat = open(file_dat, "rb")
fn_bin = open(file_bin, "ab+")

size = os.path.getsize(file_dat)
print(size)

L = []
j = 0
for i in range(size):
    data = fn_dat.read(1)
    value = struct.unpack('B', data)
    print(hex(value[0]))
    a = struct.pack('B', value[0])
    fn_bin.write(a)
    j = j + 1
    if ( j == 16 ):
       j = 0
       print('\n')
fn_dat.close()
fn_bin.close()
