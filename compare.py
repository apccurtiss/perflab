import filecmp


files = ["edge", "emboss", "hline", "motionblur", "raised", "sharpen"]

for f in files:
  
  if filecmp.cmp("filtered-" + f + "-blocks-small.bmp", "originals/filtered-" + f + "-blocks-small.bmp"):
    print '\033[92m' + f + " filter matches!" + '\033[0m'
  else:
    print '\033[93m' + f + " filter does not match!" + '\033[0m'
