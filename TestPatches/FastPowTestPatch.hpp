#include "TestPatch.hpp"
#include "FastPow.h"
#include "../Tools/FastLogTable.h"
#include "../Tools/FastPowTables.h"

class FastPowTestPatch : public TestPatch {
public:
  FastPowTestPatch(){
    debugMessage("");
    {
      TEST("FastPow");
      printf("testing fastPow\n");
      FastPow fastPow;
      // FloatArray tableH_ = FloatArray::create(FastPow::tableHLength);
      // FloatArray tableL_ = FloatArray::create(FastPow::tableLLength);
      // FastPow::fillTableH(tableH_);
      // FastPow::fillTableL(tableL_);
      FloatArray tableH = FloatArray((float*)fast_pow_h_table, fast_pow_h_table_size);
      FloatArray tableL = FloatArray((float*)fast_pow_l_table, fast_pow_l_table_size);
      // for(int n = 0; n < tableH_.getSize(); ++n){
      //   CHECK_EQUAL(tableH[n], tableH_[n]);
      //   CHECK_EQUAL(tableL[n], tableL_[n]);
      // }
      FloatArray logTable = FloatArray((float*)fast_log_table, 1 << fast_log_precision);
      fastPow.setTables(tableH, tableL, logTable);
      float maxPerc = 0;
      float threshold = 0.001; // maximum relative error accepted
      for(int n = 1; n < 1000; n++){
        float base = rand()/(float)RAND_MAX * 10;
        float exponent = n*10/1000.f;
		//fastPow can be used in three ways which should
		//yield identical results
		// .setBase and .getPow
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
		// .pow(base, exponent)
        float powBaseExp = fastPow.pow(base, exponent);
        CHECK(powBaseExp == approx);
		float iLog = fastPow.computeIlog(base);
		// .powIlog(ilog, exponent)
		float powIlog = fastPow.powIlog(iLog, exponent);
        CHECK_EQUAL(powIlog, approx);
      }
    }
  }
};

