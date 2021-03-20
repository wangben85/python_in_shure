# encoding: utf-8
import os
import sys

fwupdate_files_path = "C:\\Users\\Ben\\Desktop\\fwupdate_test.txt"
fwupdate_files_removespaces_path = "C:\\Users\\Ben\\Desktop\\fwupdate_test_removespaces.txt"
cscope_files_list = []

if __name__ == '__main__':
    fname = fwupdate_files_path
    ffilter = fwupdate_files_removespaces_path
    with open(fname, 'r') as f:   # read open
       with open(ffilter, 'w') as g:  # write
          for line in f.readlines():  # read each line from the input file
            line = ''.join(line.split())  # remove all the spaces
            g.write(line)    # write the files after remove all the spaces

