"""
This script is used to update the given excel format
"""
import argparse
import csv

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Excel Test')
    # Set the excel table file
    parser.add_argument('-fr', '--fr', default='./excel_format_update_old.csv',
                        help='the default input excel file')

    parser.add_argument('-fw', '--fw', default='./update_excel_format_update.csv',
                        help='the updated output excel file')
    args = parser.parse_args()

    L = []            # the list to store the loaded data from input excel file
    newL = []         # Updated list to store the data to output the updated excel file
    with open(args.fr, encoding='utf-8', newline='') as fr:     # open the input excel file
        f_csvR = csv.reader(fr)
        for row in enumerate(f_csvR):
            L.append(row)                   # store all the data in row to the list L
    # print(L)                              # print the items in L
    # print(len(L))                         # print how many items there, which is all the lines of the input excel file

    L = L[1:]                               # remove the first line of the excel header
    print(L)                                # print the items in L
    print(len(L))                           # print how many items there, which is all the lines of the input excel file

    # Set the output excel file header

    csv_headers = ['Frequency','PowerLevel = -100', 'PowerLevel = -99', 'PowerLevel = -98', 'PowerLevel = -97']
    with open(args.fw, 'w+', encoding='utf-8', newline='') as fw:   # open the output excel file
        f_csvW = csv.writer(fw)
        f_csvW.writerow(csv_headers)
        no4lines = int(len(L) / 4)           # get every four lines data from input as one row to output file
        print(no4lines)                      # calculate how many four lines of the original file
        col = 0
        j = 471                              # start frequency
        for i in range(0, no4lines):
            newL.clear()                     # only input four lines into newL, then clear it, to get next four lines
            for number in range(0,4):
                newL.append(L[col][1][2])    # get the FPGA status data from one line
                col = col + 1                # go to the next line to get data until four is reached
                # print(newL)
            if number == 3:
                newL.insert(0, j)            # insert the current frequency into the beginning
                print(newL)
                j = j + 1                    # next frequency
                f_csvW.writerow(newL)        # write the updated data to output file

