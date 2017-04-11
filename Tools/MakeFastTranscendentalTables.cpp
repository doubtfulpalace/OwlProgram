#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include "../LibSource/FastLog.h"
#include "../LibSource/FastPow.h"

#define FASTPOW_PRECISION 14

char fast_log_table_file[] = "FastLogTable.h";
char fast_pow_table_file[] = "FastPowTables.h";

int main(int argc, char** argv) {
  uint32_t log_precision = FASTPOW_PRECISION;
  int log_table_size = (1 << log_precision);
  float* log_lookup_table = (float*)malloc(sizeof(float) * log_table_size);
  {
    FastLog::fill_icsi_log_table(log_lookup_table, log_precision);
    FILE* file = fopen(fast_log_table_file, "w+");
    if(file < 0)
    {
    fprintf(stderr, "Error while opening file: %s, aborting\n", strerror(errno));
    exit(1);
    }
    fprintf(file, "/* ICSI log table, log_precision %d, size %d */\n", log_precision, log_table_size);
    fprintf(file, "const unsigned int fast_log_precision = %u;\n", log_precision);
    fprintf(file, "const unsigned int fast_log_table_size = %u;\n", log_table_size);
    fprintf(file, "const float fast_log_table[%d] = {\n", log_table_size);
    int32_t i;
    for(i=0; i < log_table_size; ++i)
      fprintf(file, "%a, ", log_lookup_table[i]);
    fprintf(file, "\n};\n");
    
    FastLog fastLog(log_lookup_table, log_precision);
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
      float app = fastLog.log(f);
      if(fabsf(acc) > 0.001)
      {
        //printf("icsi: %.4f, std: %.4f, err: %.4f%%\n", app, acc, (app - acc) / acc * 100);
        float e = fabsf((app - acc) / acc);
        err += e;
        m = e > m ? e : m;
      }
    }
    printf("Log: relative error over range [%.f,%.f]: average %f%%, max: %f%%\n", start, stop, err * 100 / numTest, m * 100);
  }

  {
    float* tableH = (float*)malloc(sizeof(float) * FastPow::tableHLength);
    FastPow::fillTable(tableH, 9, FastPow::tableHExtent, false);
    float* tableL = (float*)malloc(sizeof(float) * FastPow::tableLLength);
    FastPow::fillTable(tableL, 18, FastPow::tableLExtent, true);
    FILE* file = fopen(fast_pow_table_file, "w+");
    if(file < 0)
    {
      fprintf(stderr, "Error while opening file: %s, aborting\n", strerror(errno));
      exit(1);
    }
    char letters[] = "hl";
    int sizes[] = {FastPow::tableHLength, FastPow::tableLLength};
    float* arrs[2] = {tableH, tableL};
    for(int n = 0; n < 2; ++n){
      float* arr = arrs[n];
      char letter = letters[n];
      int size = sizes[n];
      fprintf(file, "/* FastPow table %s, default, size %d */\n", letter == 'h' ? "High" : "Low", size);
      fprintf(file, "const float fast_pow_%c_table_size = %d;\n", letter, size);
      fprintf(file, "const float fast_pow_%c_table[%d] = {\n", letter, size);
      int32_t i;
      for(i=0; i < size; ++i)
        fprintf(file, "%a, ", arr[i]);
      fprintf(file, "\n};\n");
    }
    FastPow fastPow(tableH, tableL, log_lookup_table, log_precision);
    float err = 0;
    float m = 0;
    int numTest = 0;
    float start = 0.01;
    float stop = 100;
	float startExp = -10;
	float stopExp = 10;
    float inc = 0.01;
    for(float f = start; f < stop; f += inc)
	{
		for(float g = startExp; g < stopExp; g += inc)
		{
		  ++numTest;
		  float acc = powf(f, g);
		  float app = fastPow.pow(f, g);
		  if(fabsf(acc) > 0.001)
		  {
			//printf("log: %.4f, std: %.4f, err: %.4f%%\n", app, acc, (app - acc) / acc * 100);
			float e = fabsf((app - acc) / acc);
			err += e;
			m = e > m ? e : m;
		  }
		}
	}
    printf("pow: relative error over range powf([%f,%f], [%f, %f]): average %f%%, max: %f%%\n", start, stop, startExp, stopExp, err * 100 / numTest, m * 100);
  }
  return 0;
}
