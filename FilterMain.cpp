#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"
#include <stdint.h>
#include "rtdsc.h"

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
  short data[9] = {filter->data[0],
                 filter->data[1],
                 filter->data[2],
                 filter->data[3],
                 filter->data[4],
                 filter->data[5],
                 filter->data[6],
                 filter->data[7],
                 filter->data[8]};

  for(int col = 0; col < w; col++) {
    for(int row = 0; row < h; row++) {
      for(int plane = 0; plane < 3; plane++) {
	f4vector a, b, c, d, e, f, g;
	a.f[0] = (input->color[col][plane][row]);
	a.f[1] = (input->color[col][plane][row+1]);
	a.f[2] = (input->color[col][plane][row+2]);
	a.f[3] = (input->color[col+1][plane][row]);
	b.f[0] = (input->color[col+1][plane][row+1]);
	b.f[1] = (input->color[col+1][plane][row+2]);
	b.f[2] = (input->color[col+2][plane][row]);
	b.f[3] = (input->color[col+2][plane][row+1]);

	c.f[0] = (data[0]);
	c.f[1] = (data[3]);
	c.f[2] = (data[6]);
	c.f[3] = (data[1]);
	d.f[0] = (data[4]);
	d.f[1] = (data[7]);
	d.f[2] = (data[2]);
	d.f[3] = (data[5]);

//	v4si x = __builtin_mulv4si(a.v, c.v);
//	v4si y = __builtin_mulv4si(b.v, d.v);
	v4si x = a.v*c.v;
	v4si y = b.v*d.v;
//	e.v = __builtin_addv4si(x,y);
	e.v = x+y;
	int value = e.f[0] + e.f[1] + e.f[2] + e.f[3] + input->color[col+2][plane][row+2]* data[8];
	/*int i1 = input->color[col][plane][row]* data[0];
        int i2 = input->color[col][plane][row+1]* data[3];
        int i3 = input->color[col][plane][row+2]* data[6];
        i1 = i1 + input->color[col+1][plane][row  ]* data[1];
        i2 = i2 + input->color[col+1][plane][row+1]* data[4];
        i3 = i3 + input->color[col+1][plane][row+2]* data[7];
        i1 = i1 + input->color[col+2][plane][row ]* data[2];
        i2 = i2 + input->color[col+2][plane][row+1]* data[5];
        i3 = i3 + input->color[col+2][plane][row+2]* data[8];
        int value = i1 + i2 + i3;
	*/
	value = value / filter -> getDivisor();
	if ( value  < 0 ) { value = 0; }
	if ( value  > 255 ) { value = 255; }
	output -> color[col+1][plane][row+1] = value;
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

