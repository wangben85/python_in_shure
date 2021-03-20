import sys
import time
import random
from enum import Enum

ext_file = 'mathout.txt'
class calcType(Enum):
    addOnly = 0
    subOnly = 1
    addSubBoth = 2

##############################################################
# test case definitions to change below three variables

intMax = 20   # the result will not larger then
examNum = 100  # the exam numbers
calcTypeSelect = calcType.addSubBoth

##############################################################

# list restore the result
R = []

def randint():
    return random.randint(0, intMax)


def do_add(x, y):
    if x + y < 20:
        # print('%2d + %2d = ' % (x, y))
        R.append('%2d + %2d = ' % (x, y))
    #    print the result, debug
    #    time.sleep(1)
    #    print('The result is')
    #    print('%2d + %2d = %2d ' % (x, y, x+y))
    else:
        do_add(randint(), randint())


def do_sub(x, y):
    if x - y > 0:
        # print('%2d - %2d = ' % (x, y))
        R.append('%2d - %2d = ' % (x, y))
    # print the result, debug
    # time.sleep(1)
    # print('The result is')
    # print('%2d - %2d = %2d ' % (x, y, x-y))
    else:
        do_sub(randint(), randint())


def do_calc_add_sub(type):
    if type == calcType.addSubBoth:                                # random plus and minus
        calc_type = random.randint(0, 1)
    elif type == calcType.addOnly:
        calc_type = 0
    else:
        calc_type = 1

    if calc_type == 0:                               # plus
        return do_add(randint(), randint())
    else:                                            # minus
        return do_sub(randint(), randint())


def do_math_random(type):
    for i in range(examNum):
        do_calc_add_sub(type)


def print_result_write_file():
    index = 0
    fout = open(ext_file, 'w')
    for item in R:
        if (index + 3) <= len(R):
            print('%10s %20s %10s %20s %10s' % (R[index], '', R[index + 1], '', R[index + 2]))
            fout = open(ext_file, 'a')
            fout.write('%10s %20s %10s %20s %10s' % (R[index], '', R[index + 1], '', R[index + 2]))
            fout.write('\n\n')
            index = index + 3
        elif (index + 2) == len(R):
            print('%10s %20s %10s ' % (R[index], '', R[index + 1] ))
            fout = open(ext_file, 'a')
            fout.write('%10s %20s %10s' % (R[index], '', R[index + 1]))
            fout.write('\n\n')
            index = index + 2
        elif (index + 1) == len(R):
            print('%10s' % (R[index]))
            fout = open(ext_file, 'a')
            fout.write('%10s' % (R[index]))
            fout.write('\n\n')
            index = index + 1

if __name__ == '__main__':
    do_math_random(calcTypeSelect)
    #print(R)
    print_result_write_file()
