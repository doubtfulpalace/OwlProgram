#include "FastTrans.h"
#include <time.h>

int main(){
  if(0)
  {
    printf("testing fastPow\n");
    FastPow fastPow;
    int precision = 14;
    fastPow.setup(precision);
    float maxPerc = 0;
    for(int n = 0; n < 1000; n++){
      float base = rand()/(float)RAND_MAX *10 ;
      float exponent = n*10/1000.f;
      fastPow.setBase(base);
      float approx = fastPow.getPow(exponent);
      float exact = powf(base, exponent);
      float err = fabsf(approx - exact);
      float perc = err/exact * 100;
      maxPerc = maxPerc > perc ? maxPerc : perc;
      printf("%.3f^%.3f = %.3f %.3f %.3f%%\n", base, exponent, exact, approx, perc);
    }
    printf("maxError: %f%%\n", maxPerc);

    clock_t begin, end;
    double time_spent;
    int N = 100000000;
    float exponent = 0.3;
    
    begin = clock();
    for(int n = 0; n < N; n++){
      fastPow.setBase(n);
      exponent = fastPow.getPow(-exponent);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Took %fs\n", time_spent);
    
    fastPow.setBase(1.8);
    begin = clock();
    for(int n = 0; n < N; n++){
      exponent = fastPow.getPow(-exponent);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Took %fs\n %f\n", time_spent, exponent);
  }
  if(1)
  {
    printf("testing fastLog\n");
    FastLog fastLog;
    int precision = 11;
    fastLog.setup(precision);
    float maxPerc = 0;
    int NN = 10000;
    for(int n = 0; n < NN ; n++){
      float in = n/(float(NN))*100;
      float approx = fastLog.log(in);
      float exact = logf(in);
      float err = fabsf(approx - exact);
      float perc = fabsf(err/exact) * 100;
      maxPerc = maxPerc > perc ? maxPerc : perc;
      //  printf("log(%f) = %f %f %f%%\n", in,  exact, approx, perc);
    }
    printf("maxError: %f%%\n", maxPerc);
    clock_t begin, end;
    double time_spent;
    int N = 100000000;
    float in = 10;
    
    begin = clock();
    for(int n = 0; n < N; n++){
      in = fastLog.log(in*10);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Took %fs\n", time_spent);
    
    begin = clock();
    for(int n = 0; n < N; n++){
      in = logf(in);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Took %fs\n %f\n", time_spent, in);
  }
  return 0;
}

