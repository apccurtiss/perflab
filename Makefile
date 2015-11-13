##
##

CXX	=g++
CXXFLAGS= -g -Ofast -m64 -ftree-vectorize -mssse3

goals: judge
	echo "Done"

filter: FilterMain.cpp Filter.cpp cs1300bmp.cc
	$(CXX) $(CXXFLAGS) -o filter FilterMain.cpp Filter.cpp cs1300bmp.cc
##
## Parameters for the test run
##
FILTERS = gauss.filter vline.filter hline.filter emboss.filter
#IMAGES = boats.bmp blocks-small.bmp
IMAGES = blocks-small.bmp
TRIALS = 1 2 

test: filter
	-./Judge -p filter -n 1 -i blocks-small.bmp
	-python compare.py
perf:
	-scp -q * alcu5535@perf-02.cs.colorado.edu:perflab
judge: filter
	#-./Judge -p ./filter -i boats.bmp
	-./Judge -p ./filter -i blocks-small.bmp
	-python compare.py
clean:
	-rm filter
	-rm filtered-*.bmp
