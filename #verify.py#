# Alex Curtiss, Audrey Randall, Izaak Weiss

import filecmp
import sys
from os import listdir
from os.path import exists, isfile, join

# Filepath to correctly filtered images
filepath = "original_images/"

class colors:
  RED = '\033[91m'
  GREEN = '\033[92m'
  YELLOW = '\033[93m'
  DEFAULT = '\033[0m'

if not exists(filepath):
  print '\033[91m' + sys.argv[0] + ": filepath for original images ( " + filepath + " ) does not exist" + '\033[0m'
  raise SystemExit

# Filtered images that exist in the current folder
new_filters = [ f for f in listdir('.') if isfile(f) and f.endswith(".bmp") and f.startswith("filtered-") ]

# Original images that are used as a reference
original_filters = [ f for f in listdir(filepath) ]


if not new_filters:
  print colors.RED + sys.argv[0] + ": cannot find any images to test: run the filter program first" + colors.DEFAULT
  raise SystemExit

if not original_filters:
  print colors.RED + sys.argv[0] + ": cannot find original images on path ( " + filepath + " )" + colors.DEFAULT
  raise SystemExit

for f in new_filters:
  if (f in original_filters):
    # Comparison occurs if original filter exists
    if filecmp.cmp(f, filepath + f):
      print colors.GREEN + sys.argv[0] + ": " + f + " matches original" + colors.DEFAULT
    else:
      print colors.RED + sys.argv[0] + ": " + f + " does not match original!" + colors.DEFAULT
  else:
    # Error if original filter does not exist
    print colors.YELLOW + sys.argv[0] + ": cannot find original filter to verify against file: " + f + colors.DEFAULT
