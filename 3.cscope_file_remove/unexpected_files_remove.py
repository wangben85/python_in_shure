import shutil, sys, os
del_array = [
  "mocks",
  "fake",
  "fsl_",
  "tests"
]

# check items in array if exist in line
def isInArray (array, line):
  for item in array:
    if item in line:
      return True
  return False

# main 
if __name__ == '__main__':
  argv = sys.argv
  argc = len(argv)
#  if argc < 2:    
    #print "Usage: %s <file>" %(os.path.basename(argv[0]))
    #exit()
  #fname = argv[1]    # file name as the input argument
  fname = "test1.files"  # input files
  fresult = fname + ".result"  # output files
  with open(fname, 'r') as f:
    with open(fresult, 'w') as g:
      for line in f.readlines():  # read each line from the input file
        if not isInArray(del_array, line):
           g.write(line)
