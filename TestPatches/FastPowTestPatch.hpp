#include "TestPatch.hpp"
#include "FastPow.h"
#include "../Tools/IcsiLogTable.h"

class FastPowTestPatch : public TestPatch {
public:
  FastPowTestPatch(){
    debugMessage("");
    {
      TEST("FastPow");
      printf("testing fastPow\n");
      FastPow fastPow;
      FloatArray tableH = FloatArray::create(1 << FastPow::tableHLength);
      FloatArray tableL = FloatArray::create(1 << FastPow::tableLLength);
      FastPow::fillTableH(tableH);
      FastPow::fillTableL(tableL);
      FloatArray logTable = FloatArray((float*)icsi_log_table, 1 << icsi_log_precision);
      fastPow.setTables(tableH, tableL, logTable);
      float maxPerc = 0;
      float threshold = 0.001; // maximum relative error accepted
      for(int n = 1; n < 1000; n++){
        float base = rand()/(float)RAND_MAX * 10;
        float exponent = n*10/1000.f;
        fastPow.setBase(base);
        float approx = fastPow.getPow(exponent);
        float exact = powf(base, exponent);
        float err = fabsf(approx - exact);
        float perc = err/exact * 100;
        maxPerc = maxPerc > perc ? maxPerc : perc;
        //printf("%.3f^%.3f = %.3f %.3f %.3f%%\n", base, exponent, exact, approx, perc);
        if(fabsf(exact) > 0.001){
          float relErr = err / exact;
          CHECK_CLOSE(relErr, 0.f, threshold);
        } else {
          CHECK_CLOSE(approx, exact, 0.001f);
        }
        float powBaseExp = fastPow.pow(base, exponent);
        CHECK(powBaseExp == approx);
        //float fastpowfresult = fastpowf(base, exponent);
        //if(approx != fastpowfresult){
          //printf("fastpowresult != fastpowf(base, exponent), approx: %f, powresult: %f, base %f, exponent %f", approx, fastpowfresult, base, exponent);
          //exit(1);
        //}
        //float fastexpfresult = fastexpf(exponent);
        //fastpowfresult = fastpowf(exp(1), exponent);
        //if(fastpowfresult != fastexpfresult){
          //printf("fastexpfresult != fastpowfresult(base, exponent), approx: %f, powresult: %f, exponent %f", fastexpfresult, fastpowfresult, exponent);
          //exit(1);
        //}
        //float fastpowf2fresult = fastPow.pow(base, exponent);
        //fastpowfresult = fastpowf(2, exponent);
        //if(fastpowfresult != fastpowf2fresult){
          //printf("fastpowf2fresult != fastpowfresult(base, exponent), approx: %f, powresult: %f, exponent %f", 
          //fastpowf2fresult, fastpowfresult, exponent);
        //}
        //exact = powf(base, exponent);
        //printf("pow2f %f, fastpow2f %f (%f%%)\n", exact, fastpowf2fresult, fabsf(exact-fastpowf2fresult)/exact*100);
        //TODO check for maxError of fastpow2f
      }
    }
    {
      //TEST("Default ctor");
      //CHECK_EQUAL(empty.getSize(), 0);
      //CHECK_CLOSE(tempc2[n].re, (cfa[n].re*tempc[n].re - cfa[n].im*tempc[n].im) >> 17, 4);
    }
  }
};

