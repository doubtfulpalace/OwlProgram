#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define FASTPOW_PRECISION 14

float* gLookupTable;
float icsiLog(float arg, int n)
{
  float* lookup_table = gLookupTable;
  if(arg <= 0) 
    return 0;
  int *const exp_ptr = ((int*)&arg);
  int x = *exp_ptr; //x is treated as integer
  const int log_2 = ((x >> 23) & 255) - 127;//exponent
  x &= 0x7FFFFF; //mantissa
  x = x >> (23-n); //quantize mantissa
  arg = lookup_table[x]; //lookup precomputed arg
  arg = ((arg + log_2)* 0.69314718);
  return arg; //natural logarithm
}
/* Creates the ICSILog lookup table.
*/
void fill_icsi_log_table(float* lookup_table, uint32_t precision){
  float numlog;
  int32_t *const exp_ptr = ((int32_t*)&numlog);
  int32_t x = *exp_ptr; //x is the float treated as an integer
  x = 0x3F800000; //set the exponent to 0 so numlog=1.0
  *exp_ptr = x;
  int32_t incr = 1 << (23 - precision); //amount to increase the mantissa
  int32_t size = 1 << precision;
  int32_t i;
  for(i=0;i<size;++i){
    lookup_table[i] = log2(numlog); //save the log value
     x += incr;
    *exp_ptr = x; //update the float value
  }
}

int main(int argc, char** argv) {
  uint32_t precision = FASTPOW_PRECISION;
  float* lookup_table;
  int table_size = (1 << precision);
  lookup_table = (float*)malloc(sizeof(float)*table_size);
  fill_icsi_log_table(lookup_table, precision);
  FILE* file = fopen("IcsiLogTable.h", "w+");
  if(file < 0)
  {
    fprintf(stderr, "Error while opening file: %s, aborting\n", strerror(errno));
    exit(1);
  }
  fprintf(file, "/* ICSI log table, precision %d, size %d */\n", precision, table_size);
  fprintf(file, "const unsigned int icsi_log_precision = %u;\n", precision);
  fprintf(file, "const float icsi_log_table[%d] = {\n", table_size);
  int32_t i;
  for(i=0; i<table_size; ++i)
    fprintf(file, "%.4f, ", lookup_table[i]);
  fprintf(file, "\n};\n");
  
  gLookupTable = lookup_table;
  float err = 0;
  float m = 0;
  int numTest = 0;
  float start = 0.01;
  float stop = 100;
  float inc = 0.01;
  for(float f = start; f < stop; f += inc)
  {
    ++numTest;
    float acc = logf(f);
    float app = icsiLog(f, precision);
    if(fabsf(acc) > 0.001)
    {
      //printf("icsi: %.4f, std: %.4f, err: %.4f%%\n", app, acc, (app - acc) / acc * 100);
      float e = fabsf((app - acc) / acc);
      err += e;
      m = e > m ? e : m;
    }
  }
  printf("relative error over range [%.f,%.f]: average %f%%, max: %f%%\n", start, stop, err * 100 / numTest, m * 100);
  return 0;
}
