"""
This python script is going to import the IAR project map file and calculate current project memory cost size
"""
import argparse

#fname = "BHTX_App.map"
#fname = "BHTX_AD3_App.map"                           # select the map file needs to calculate the memory size

parser = argparse.ArgumentParser(description='Run the test to calculate memory code size')
# Set default map file name to BHTX_App.map
parser.add_argument('-f', '--file', default='./BHTX_App.map', help='Specify the memory cose map file')

args = parser.parse_args()
fname = args.file                                     # we can also specify other map file name in the command line by '-f'
                                                      # e.g.  python code_memory_cost -f BHTX_AD3_App.map

with open(fname, 'rb') as f:
    first_line = f.readline()                         # read one line as the first line, not it is not 'readlines' here
    offset = -50                                      # offset initial value set to '-50'
    while True:
        f.seek(offset, 2)                             # '2' argument means file offset seek from the end of file
        lines = f.readlines()                         # read all the existing lines until the file ending
        if len(lines) >= 7:                           # keep the last 7 lines
            last_no7_line_readonly_code = lines[-7]   # readonly code memory
            last_no6_line_readwrite_code = lines[-6]  # readwrite code memory
            last_no5_line_readonly_data = lines[-5]   # readonly data memory
            last_no4_line_readwrite_data = lines[-4]  # readwrite data memory
            break
        offset *= 2                                   # modify the offset values to seek all the 7 lines in the file

# 'split' and 'replace' here are to split the lines and remove the space
readonly_code = last_no7_line_readonly_code.decode().split('bytes')[0].replace(' ','')
readwrite_code = last_no6_line_readwrite_code.decode().split('bytes')[0].replace(' ','')
readonly_data = last_no5_line_readonly_data.decode().split('bytes')[0].replace(' ','')
readwrite_data = last_no4_line_readwrite_data.decode().split('bytes')[0].replace(' ','')

flash_memory = (int(readonly_code) + int(readwrite_code) + int(readonly_data)) / 1024
ram_memory =  int(readwrite_data) / 1024

print('file ' + fname + ' last number 7 line is :' + last_no7_line_readonly_code.decode() )
print('file ' + fname + ' last number 6 line is :' + last_no6_line_readwrite_code.decode() )
print('file ' + fname + ' last number 5 line is :' + last_no5_line_readonly_data.decode() )
print('file ' + fname + ' last number 4 line is :' + last_no4_line_readwrite_data.decode() )

print('Project Flash size Boot loader costs 96K bytes!')

print('Project Flash size App costs ' + str(flash_memory) + '/416K bytes!')
print('Project RAM size App costs ' + str(ram_memory) + '/64K bytes!')

