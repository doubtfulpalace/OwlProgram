#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FASTPOW_PRECISION 14

/* Creates the ICSILog lookup table.
*/
void fill_icsi_log_table(float* lookup_table, uint32_t precision){
  uint32_t n = precision;
  float numlog;
  int32_t *const exp_ptr = ((int32_t*)&numlog);
  int32_t x = *exp_ptr; //x is the float treated as an integer
  x = 0x3F800000; //set the exponent to 0 so numlog=1.0
  *exp_ptr = x;
  int32_t incr = 1 << (23-n); //amount to increase the mantissa
  int32_t size = 1<<precision;
  int32_t i;
  for(i=0;i<size;++i){
    lookup_table[i] = log2(numlog); //save the log value
    x += incr;
    *exp_ptr = x; //update the float value
  }
}


/* void setTable(float* const pTable, const uint32_t precision, */
/* 	      const uint32_t extent, const bool isRound){ */
/*   // step along table elements and x-axis positions */
/*   float zeroToOne = !isRound ? */
/*     0.0f : (1.0f / (static_cast<float>(1 << precision) * 2.0f)); */
/*   for(int32_t i = 0;  i < (1 << extent);  ++i ){ */
/*     // make y-axis value for table element */
/*     pTable[i] = ::powf( 2.0f, zeroToOne ); */
/*     zeroToOne += 1.0f / static_cast<float>(1 << precision); */
/*   } */
/* } */

int main(int argc, char** argv) {
  uint32_t precision = FASTPOW_PRECISION;
  float* lookup_table;
  int table_size = (1 << precision);
  lookup_table = (float*)malloc(sizeof(float)*table_size);
  fill_icsi_log_table(lookup_table, table_size);
  printf("/* ICSI log table, precision %d, size %d */\n", precision, table_size);
  printf("const float icsi_table[%d] = {\n", table_size);
  int32_t i;
  for(i=0; i<table_size; ++i)
    printf("%f, ", lookup_table[i]);
  printf(" };\n");

  return 0;
}
