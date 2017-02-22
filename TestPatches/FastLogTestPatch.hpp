#include "TestPatch.hpp"
#include "FastLog.h"
#include "../Tools/IcsiLogTable.h"

class FastLogTestPatch : public TestPatch {
public:
  FastLogTestPatch(){
    debugMessage("");
    {
      TEST("FastLog");
      FastLog fastLog(icsi_log_table, icsi_log_precision);
      //CHECK_EQUAL(empty.getSize(), 0);
      //CHECK_CLOSE(tempc2[n].re, (cfa[n].re*tempc[n].re - cfa[n].im*tempc[n].im) >> 17, 4);
      float maxPerc = 0;
      int NN = 100000;
      for(int n = 1; n < NN ; n++){
        float in = n/(float(NN))*100;
        float approx = fastLog.log(in);
        float exact = logf(in);
        float err = fabsf(approx - exact);
        float perc = fabsf(err/exact) * 100;
        maxPerc = maxPerc > perc ? maxPerc : perc;
        {
          // float fastlog2result = fastlog2f(in);
          float fastlog2result = log2f(in);
          float fastlogresult1 = fastLog.log(in) / fastLog.log(2);
          float fastlogresult2 = fastLog.log(2, in);
          float threshold = 0.000137; // tested with NN=10000000
	  CHECK_CLOSE(fastlog2result, fastlogresult1, threshold);
	  CHECK_CLOSE(fastlog2result, fastlogresult2, threshold);
        }
      }
    }
    {
      //TEST("Default ctor");
      //CHECK_EQUAL(empty.getSize(), 0);
      //CHECK_CLOSE(tempc2[n].re, (cfa[n].re*tempc[n].re - cfa[n].im*tempc[n].im) >> 17, 4);
    }
    debugMessage("end of tests!");
  }
};
