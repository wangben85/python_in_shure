from PIL import Image
import numpy as np
import csv
threshold = 10000    # threshold for print
# source image(bmp)file
image_path = "C:\\Users\\wangb\\Desktop\\test_image\\test1.bmp"
#image_path = "C:\\Users\\wangb\\Desktop\\test_image\\test2.bmp"
#image_path = "C:\\Users\\wangb\\Desktop\\test_image\\updating.bmp"

img = Image.open(image_path)    # open image
img2_array = np.array(img)
img3_array = img2_array
width,height = img.size
np.set_printoptions(threshold=np.inf)   # complete print all the matrix when the result is smaller than threshold
print(height)     # 48
print(width)      # 64

#for i in range(0, height):
#    print(i)
#    for j in range(0, width):
        # if( i == 20 ):
        #  print(~img2_array[i][j])  # get the array matrix and copy them to Image_Updating2.xlsx
        # if ( img2_array[i][j] > 0 ):
        #        print(True)
        # else:
        #        print(False)

    #print(img2_array[i]>0 ? True:False)  # get the array matrix and copy them to Image_Updating2.xlsx


# This is for image_path = "C:\\Users\\wangb\\Desktop\\test_image\\updating.bmp"
#with open("arraytable.csv", 'w+', encoding='utf-8', newline='') as csvfile:
    #t_csv = csv.writer(csvfile)
    #for i in range(0, height):
       #print(i)
       #t_csv.writerow(~img2_array[i])

# This is for image_path = "C:\\Users\\wangb\\Desktop\\test_image\\updating.bmp"
# This is for image_path = "C:\\Users\\wangb\\Desktop\\test_image\\test1.bmp"
with open("arraytable.csv", 'w+', encoding='utf-8', newline='') as csvfile:
    t_csv = csv.writer(csvfile)
    for i in range(0, height):
       print(i)
       t_csv.writerow(img2_array[i])

