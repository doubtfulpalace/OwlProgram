/**
 * This class computes pow with a fast algorithm described in 
 * http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html
 * It internally allocates two tables of size 2^9(sizeof(float)) each  and a 
 * FastLog object of the given precision.
 * 
*/
#include "FastLog.h"

class FastPow {
private:
  /**
   * @_ilog2      one over log, to required radix, of two
  */
  float* tableH_m;
  float* tableL_m;
  static constexpr float _2p23 = 8388608.0f;
  float _ilog2;
  FastLog fastLog;

public:
  /**
   * Initialize powFast lookup table.
   */
  static void fillTable(float* const pTable, const unsigned int precision,
     const unsigned int extent, const bool isRound)
  {
    // step along table elements and x-axis positions
    float zeroToOne = !isRound ?
      0.0f : (1.0f / (static_cast<float>(1 << precision) * 2.0f));
    for(int i = 0;  i < (1 << extent);  ++i )
    {
      // make y-axis value for table element
      pTable[i] = ::powf( 2.0f, zeroToOne );
      zeroToOne += 1.0f / static_cast<float>(1 << precision);
    }
  }
  static constexpr int tableHExtent = 9;
  static constexpr int tableLExtent = 9;
  static constexpr int tableHLength = 1 << tableHExtent;
  static constexpr int tableLLength = 1 << tableLExtent;
  FastPow() :
    tableH_m(NULL),
    tableL_m(NULL)
  {}

  ~FastPow(){}
  
  static int fillTableH(FloatArray table){
    if(table.getSize() < 9)
      return -1;
    fillTable(table, 9, tableHExtent, false);
    return 1;
  }

  static int fillTableL(FloatArray table){
    if(table.getSize() < 9)
      return -1;
    fillTable(table, 18, tableLExtent, true);
    return 1;
  }

  void setTables(float* tableH, float* tableL, FloatArray logTable){
    tableH_m = tableH;
    tableL_m = tableL;
    fastLog.setTable(logTable);
  }

  /**
   * Get pow (fast!).
   *
   * @param value power to raise radix to
   *
   * @return radix^exponent
   */
  float getPow(const float val)
  {
    const float _2p23 = 8388608.0f;

    // build float bits
    const int i = static_cast<int>( (val * (_2p23 * _ilog2)) + (127.0f * _2p23) );

    // replace mantissa with combined lookups
    const float t  = tableH_m[(i >> 14) & (tableHLength - 1)] * tableL_m[(i >> 5) & (tableLLength - 1)];
    const int   it = (i & 0xFF800000) |
        (*reinterpret_cast<const int*>( &t ) & 0x7FFFFF);

    // convert bits to float
    return *reinterpret_cast<const float*>( &it );
  }

  /**
   * Sets the radix.
   *
   * @param base radix
   */
  void setBase(const float base){
    _ilog2 = computeIlog(base);
  }
  
  /**
   * Compute fast pow.
   *
   * @param base radix
   * @param exponent exponent
   *
   * @return radix^exponent
   */
  float pow(float base, float exponent){
    setBase(base);
    return getPow(exponent);
  }
  
  /**
   * Compute fast pow.
   *
   * @param ilog2 log(base)*1.44269504088896
   * @param exponent exponent
   *
   * @return radix^exponent
   */
  float powIlog(float ilog2, float exponent){
    _ilog2 = ilog2;
    return getPow(exponent);
  }

  /** 
   * Computes the logarithm.
   * 
   * Computes an approximation of logn(val)
   * using the internal FastLog object
   *
   * @param arg the argument of the logarithm. 
   * @return log(arg), or 0 if arg<=0
   */
  float log(float arg){
    return fastLog.log(arg);
  }
  
   /** 
   * Computes the logarithm.
   * 
   * Computes an approximation of log_{base}(arg)
   * using the internal FastLog object
   *
   * @param base the base of the logarithm. 
   * @param arg the argument of the logaritm
   * @return log(arg), or 0 if arg<=0
   */
  float log(float base, float arg){
    return fastLog.log(base, arg);
  }

  /**
   * Computes the ilog2 to pass to powIlog
   *
   * @param base the base of the logarithm
   * 
   * @return the ilog2
   */
  float const computeIlog(float base){
    return log(base)*1.44269504088896;
  }
};


