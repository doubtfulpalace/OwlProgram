#include "FastTrans.h"
#include <time.h>
#include "basicmaths.cpp"


int main(){
  if(1)
  {
    printf("testing fastPow\n");
    FastPow fastPow;
    int precision = FASTPOW_PRECISION;
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
      //  printf("%.3f^%.3f = %.3f %.3f %.3f%%\n", base, exponent, exact, approx, perc);
      float powresult = fastPow.pow(base, exponent);
      float fastpowresult = approx;
      if(fastpowresult != powresult){
        printf("fastpowresult != fastPow.pow(base, exponent), approx: %f, powresult: %f, base %f, exponent %f", approx, powresult, base, exponent);
        exit(1);
      }
      float fastpowfresult = fastpowf(base, exponent);
      if(approx != fastpowfresult){
        printf("fastpowresult != fastpowf(base, exponent), approx: %f, powresult: %f, base %f, exponent %f", approx, fastpowfresult, base, exponent);
        exit(1);
      }
      float fastexpfresult = fastexpf(exponent);
      fastpowfresult = fastpowf(exp(1), exponent);
      if(fastpowfresult != fastexpfresult){
        printf("fastexpfresult != fastpowfresult(base, exponent), approx: %f, powresult: %f, exponent %f", fastexpfresult, fastpowfresult, exponent);
        exit(1);
      }
      float fastpowf2fresult = fastpow2f(exponent);
      fastpowfresult = fastpowf(2, exponent);
      if(fastpowfresult != fastpowf2fresult){
        printf("fastpowf2fresult != fastpowfresult(base, exponent), approx: %f, powresult: %f, exponent %f", 
          fastpowf2fresult, fastpowfresult, exponent);
        exit(1);
      }
      exact = powf(2, exponent);
      //  printf("pow2f %f, fastpow2f %f (%f%%)\n", exact, fastpowf2fresult, fabsf(exact-fastpowf2fresult)/exact*100);
      //  exit(1);
      //TODO check for maxError of fastpow2f
    }
    printf("maxError of fastpow: %f%%\n", maxPerc);

    clock_t begin, end;
    double time_spent;
    int N = 10000000;
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
    int precision = FASTPOW_PRECISION;
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
      
      //test fastlog2f
      {
        float fastlog2result = fastlog2f(in);
        float fastlogresult1 = fastLog.log(in) / fastLog.log(2);
        float fastlogresult2 = fastLog.log(2, in);
        float threshold = 0.000001;
        if(fabsf(fastlog2result - fastlogresult1)>threshold || fabsf(fastlog2result-fastlogresult2)>threshold){
          printf("fastlog2 mismatch %d: %f %f %f\n", n, fastlog2result, fastlogresult1, fastlogresult2);
          exit(1);
        }
      }
    }
    printf("maxError of fastLog: %f%%\n", maxPerc);
    clock_t begin, end;
    double time_spent;
    int N = 10000000;
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


