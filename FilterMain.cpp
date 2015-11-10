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

  int divisor = (filter->getDivisor())>>2; 
  int size = filter->getSize();
  int value = 0;
  int dim = filter->dim;
  int* data = filter->data;
  /*
  int data[9] = {filter->data[0],
		 filter->data[1],
		 filter->data[3],
		 filter->data[4],
		 filter->data[5],
		 filter->data[6],
		 filter->data[7],
		 filter->data[8]};
  */
  int w = input->width - 2;
  int h = input->height - 2;
  
  int block_size = 64;

  for(int col = 0; col < w; col+=block_size) {
    for(int plane = 0; plane < 3; plane++) {
        for(int row = 0; row < h ; row +=block_size) {

       	   for(int colc = col; colc < block_size; colc++) {
	      for(int rowc = row; rowc < block_size; rowc++)  {
		int i1 = input->color[col][plane][row]* data[0];
	        int i2 = input->color[col][plane][row+1]* data[3];
	        int i3 = input->color[col][plane][row+2]* data[6];      
        
	        i1 = i1 + input->color[col+1][plane][row]* data[1];
	        i2 = i2 + input->color[col+1][plane][row+1]* data[4];
	        i3 = i3 + input->color[col+1][plane][row+2]* data[7];

	        i1 = i1 + input->color[col+2][plane][row]* data[2];
	        i2 = i2 + input->color[col+2][plane][row+1]* data[5];
	        i3 = i3 + input->color[col+2][plane][row+2]* data[8];

	      }
	   }
	//value = 0;
	/*for (int j = 0; j < size; j++) {
	  for (int i = 0; i < size; i++) {
	    value = value + input->color[col + j][plane][row + i]
	      * data[i * dim + j];
	  }
	}*/
	
	/*
	int i1 = input->color[col][plane][row]* data[0];
	int i2 = input->color[col][plane][row+1]* data[3];
	int i3 = input->color[col][plane][row+2]* data[6];	
	
	i1 = i1 + input->color[col+1][plane][row]* data[1];
        i2 = i2 + input->color[col+1][plane][row+1]* data[4];
        i3 = i3 + input->color[col+1][plane][row+2]* data[7];

	i1 = i1 + input->color[col+2][plane][row]* data[2];
        i2 = i2 + input->color[col+2][plane][row+1]* data[5];
        i3 = i3 + input->color[col+2][plane][row+2]* data[8];
	*/
/*	int i1 = input->color[col][plane][row]* data[0];
	int i1 = input->color[col][plane][row]* data[0];
	int i2 = input->color[col+1][plane][row]* data[1];
	int a = i1;
	int i3 = input->color[col+2][plane][row]* data[2];
	int b = i2;
	int i4 = input->color[col][plane][row+1]* data[3];
        a += i3;
	i1 = input->color[col+1][plane][row+1]* data[4];
	b += i4;
	i2 = input->color[col+2][plane][row+1]* data[5];
        a += i1;
	i3 = input->color[col][plane][row+2]* data[6];	
        b += i2;
	i4 = input->color[col+1][plane][row+2]* data[7];
	a += i3;
	i1 = input->color[col+2][plane][row+2]* data[8];
	b += i4;
	a += i1;
	value = a + b;*/
	value = value>>divisor;
	/*
	if ( value < 0) { value = 0; }
	*/
	if ( value  > 255 ) { value = 255; }
	//value = (value < 0)? 0 : value;
	value = value & ~(value >> 0x1f);
	//value = (value > 255)? 255 : value;
	//*/
	output -> color[col+1][plane][row + 1] = value;
      }
    }
  }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}
