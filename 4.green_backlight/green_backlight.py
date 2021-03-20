"""
This python script is to config the system environment 'regedit' to make the window backlight to green to protect users' eye
!!!!!!In order to full access the regedit with the right of 'KEY_ALL_ACCESS', pleas launch pyCharm with administrator!!!!!!!
Another option is to run cmd.exe as the adminitrator then run "python green_backlight"
"""
import win32api
import win32con
import os
import sys
import time

# test key of IE
test_key = win32api.RegOpenKey(win32con.HKEY_LOCAL_MACHINE, 'SOFTWARE\\Microsoft\\Internet Explorer', 0, win32con.KEY_ALL_ACCESS)
IEversion = win32api.RegQueryValueEx(test_key, 'Version')
print(IEversion)  # print the IE version

# key of window color under HKEY_CURRENT_USER\Control Panel\Colors
# specify the key in regedit
window_color_key = win32api.RegOpenKey(win32con.HKEY_CURRENT_USER,'Control Panel\\Colors', 0, win32con.KEY_ALL_ACCESS)
# query the orignal key value
window_color_before = win32api.RegQueryValueEx(window_color_key, 'Window')
# print the key value
print(window_color_before)
# set the key value to the desired value
win32api.RegSetValueEx(window_color_key, 'Window', 0, win32con.REG_SZ, '204 232 208')
# query the value after set
window_color_after = win32api.RegQueryValueEx(window_color_key, 'Window')
# print the key value
print(window_color_after)

# key of window color under HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\DefaultColors\Standard
# specify the key in regedit
window_standard_color_key = win32api.RegOpenKey(win32con.HKEY_LOCAL_MACHINE, 'SOFTWARE\\Microsoft\\Windows\\Current'
                                                'Version\\Themes\\DefaultColors\\Standard', 0, win32con.KEY_ALL_ACCESS)
# query the orignal key value
window_standard_color_before = win32api.RegQueryValueEx(window_standard_color_key, 'Window')
# print the key value
print(window_standard_color_before)
# set the key value to the desired value
# temp_value = 0xffffff    # this setting is to put window back light to white by default
temp_value = 0xcaeace    # this setting is to put window back light to green
win32api.RegSetValueEx(window_standard_color_key, 'Window', 0, win32con.REG_DWORD, temp_value)
# query the value after set
window_standard_color_after = win32api.RegQueryValueEx(window_standard_color_key, 'Window')
# print the key value
print(window_standard_color_after)

check_input = input("Are you sure want to restart your PC to take effect!(y/n?)\n")
if check_input == 'n':
   print('Do nothing\n')
   exit()
else:
   print('PC is going restart in 3 seconds\n')
   time.sleep(3)
   print('PC begins restarting\n')
   os.system("shutdown /r /t 1")
