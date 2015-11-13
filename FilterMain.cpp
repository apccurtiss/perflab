#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"

using namespace std;

#include "rtdsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  }
}


double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output)
{

  long long cycStart, cycStop;

  cycStart = rdtscll();

  output -> width = input -> width;
  output -> height = input -> height;
  
  unsigned const static short int width = input -> width - 1;
  unsigned const static short int height = input -> height - 1;
  unsigned const static short divisor = filter->getDivisor();
  static const char data[3][3] = {
		{filter -> data[0], filter -> data[1], filter -> data[2]},
		{filter -> data[3], filter -> data[4], filter -> data[5]},
		{filter -> data[6], filter -> data[7], filter -> data[8]}};
   
#define BLOCK_SIZE 32
#define MIN(x,y) ((x < y)? x : y)
for (int colBlock = 1; colBlock < width; colBlock = colBlock + BLOCK_SIZE) {
for (int rowBlock = 1; rowBlock < height; rowBlock = rowBlock + BLOCK_SIZE) {
  int rowEnd = MIN(rowBlock + BLOCK_SIZE, height);
  int colEnd = MIN(colBlock + BLOCK_SIZE, width);
  for(int col = colBlock; col < colEnd; col = col + 1) {	
    for(int row = rowBlock; row < rowEnd; row = row + 1) {
       for(int color = 0; color < 3; color = color + 1) {


        int a = input -> color[col-1][row-1][color] * data[0][0];
        a += input -> color[col-1][row][color] * data[1][0];
        a += input -> color[col-1][row+1][color] * data[2][0];
        int b = input -> color[col][row-1][color] * data[0][1];
        b += input -> color[col][row][color] * data[1][1]; 
        b += input -> color[col][row+1][color] * data[2][1];
	int c = input -> color[col+1][row-1][color] * data[0][2];
	c += input -> color[col+1][row][color] * data[1][2];
	c += input -> color[col+1][row+1][color] * data[2][2];
	
	int d = a + b + c;
	if(divisor==16)
	  d = d >> 4;
		
	output -> color[col][row][color] = d<0?0:d>255?255:d;
	
	/*
	a = input -> color[col-1][row-1][1] * data[0][0];
	a += input -> color[col-1][row][1] * data[1][0];
	a += input -> color[col-1][row+1][1] * data[2][0];
        b = input -> color[col][row-1][1] * data[0][1];
        b += input -> color[col][row][1] * data[1][1]; 
        b += input -> color[col][row+1][1] * data[2][1];
	c = input -> color[col+1][row-1][1] * data[0][2];
	c += input -> color[col+1][row][1] * data[1][2];
	c += input -> color[col+1][row+1][1] * data[2][2];
	
	d = a + b + c;
	if(divisor==16)
	  d = d >> 4;
		
	output -> color[col][row][1] = d<0?0:d>255?255:d;

	a = input -> color[col-1][row-1][2] * data[0][0];
	a += input -> color[col-1][row][2] * data[1][0];
	a += input -> color[col-1][row+1][2] * data[2][0];
        b = input -> color[col][row-1][2] * data[0][1];
        b += input -> color[col][row][2] * data[1][1]; 
        b += input -> color[col][row+1][2] * data[2][1];
	c = input -> color[col+1][row-1][2] * data[0][2];
	c += input -> color[col+1][row][2] * data[1][2];
	c += input -> color[col+1][row+1][2] * data[2][2];
	
	d = a + b + c;
	if(divisor==16)
	  d = d >> 4;
		
	output -> color[col][row][2] = d<0?0:d>255?255:d;
	*/
      }
    }
  }
}}
  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}
