# encoding: utf-8
import os
import sys

cscope_files_path = "C:\\Work_Python\\cscope_file_remove\\source.files"
cscope_files_list = []

def files_list_append(obj):
    """
    endswith() define the suffix files to be find
    """
    if obj.endswith(".cpp") or \
       obj.endswith(".c") or \
       obj.endswith(".h") or\
       obj.endswith(".hpp"):
       print(obj)
       cscope_files_list.append(obj + '\n')


def search_dir(dir_path):
    dir_files = os.listdir(dir_path)  # list all the files and folders of current folder
    for file in dir_files:
        file_path = os.path.join(dir_path, file)  # absolute path of the file
        if os.path.isfile(file_path):  # if it is the file
            files_list_append(file_path)
        elif os.path.isdir(file_path): # if it is the folder
            search_dir(file_path)  # call itself

def file_write(content):
    with open(cscope_files_path, 'w') as g:
        g.write(content)
        g.flush()
        g.close()

if __name__ == '__main__':
    dir_path = sys.path[0]    # current sys path
    search_dir(dir_path)      # search dir
    content = ''.join(cscope_files_list)   # convert list to str for file write
    file_write(content)
