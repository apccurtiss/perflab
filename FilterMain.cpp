#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"
#include <stdint.h>
#include "rtdsc.h"
#include <cstring>
using namespace std;

typedef int v4si __attribute__ ((mode(V4SI)));
union f4vector {
  v4si v;
  int f[4];
};

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
  
  short w = input -> width - 2;
  short h = input -> height - 2; 
  f4vector a, b, c, d, x, y, z;
  x.f[0] = filter->data[0];
  x.f[1] = filter->data[1];
  x.f[2] = filter->data[2];
  y.f[0] = filter->data[3];
  y.f[1] = filter->data[4];
  y.f[2] = filter->data[5];
  z.f[0] = filter->data[6];
  z.f[1] = filter->data[7];
  z.f[2] = filter->data[8];
  
  int divisor = filter->getDivisor() >> 2;
  for(int plane = 0; plane < 3; plane++) {
    for(int col = 0; col < w; col++) {
      a.f[0] = (input->color[col][plane][0]);
      a.f[1] = (input->color[col+1][plane][0]);
      a.f[2] = (input->color[col+2][plane][0]);
      b.f[0] = (input->color[col][plane][1]);
      b.f[1] = (input->color[col+1][plane][1]);
      b.f[2] = (input->color[col+2][plane][1]);
      for(int row = 0; row < h; row+=3) {
	int value;	
        c.f[0] = (input->color[col][plane][row+2]);
        c.f[1] = (input->color[col+1][plane][row+2]);
        c.f[2] = (input->color[col+2][plane][row+2]);
	v4si i = a.v * x.v;
	v4si j = b.v * y.v;
	v4si k = c.v * z.v;
	d.v = i + j + k;
	value = d.f[0] + d.f[1] + d.f[2];
	value = value >> divisor;
	value = value&(~(value>>0x1f));
	value = value>255?255:value;
	output -> color[col+1][plane][row+1] = value;

	a.f[0] = (input->color[col][plane][row+3]);
	a.f[1] = (input->color[col+1][plane][row+3]);
	a.f[2] = (input->color[col+2][plane][row+3]);	
	i = a.v * z.v;
	j = b.v * x.v;
	k = c.v * y.v;
	d.v = i + j + k;
	value = d.f[0] + d.f[1] + d.f[2];
	value = value >> divisor;
	value = value&(~(value>>0x1f));
	value = value>255?255:value;
	output -> color[col+1][plane][row+2] = value;

	b.f[0] = (input->color[col][plane][row+4]);
	b.f[1] = (input->color[col+1][plane][row+4]);
	b.f[2] = (input->color[col+2][plane][row+4]);	
	i = a.v * y.v;
	j = b.v * z.v;
	k = c.v * x.v;
	d.v = i + j + k;
	value = d.f[0] + d.f[1] + d.f[2];
	value = value >> divisor;
	value = value&(~(value>>0x1f));
	value = value>255?255:value;
	output -> color[col+1][plane][row+3] = value;
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

