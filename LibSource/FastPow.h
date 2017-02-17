/**
 * This class computes pow with a fast algorithm described in 
 * http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html
 * It internally allocates two tables of size 2^9(sizeof(float)) each  and a 
 * FastLog object of the given precision.
 * 
*/
class FastPow {
private:
  /**
   * @_ilog2      one over log, to required radix, of two
  */
  float* tableH_m;
  float* tableL_m;
  static constexpr float _2p23 = 8388608.0f;
  unsigned int _precision;
  float _ilog2;
  FastLog fastLog;
  void setTable(float* const pTable, const unsigned int precision,
     const unsigned int extent, const bool isRound)
  {
    // step along table elements and x-axis positions
    float zeroToOne = !isRound ?
      0.0f : (1.0f / (static_cast<float>(1 << precision) * 2.0f));
    for( int i = 0;  i < (1 << extent);  ++i )
    {
      // make y-axis value for table element
      pTable[i] = ::powf( 2.0f, zeroToOne );

      zeroToOne += 1.0f / static_cast<float>(1 << precision);
    }
  }
public:
  FastPow(){
    tableH_m = NULL;
    tableL_m = NULL;
  }
  ~FastPow(){
    //free(tableH_m);
    //free(tableL_m);
  }
  
  /**
   * Initialize powFast lookup table.
   *
   * @param precision of the associated log table, must be 0 =< precision <= 23
   */
  void setup(const unsigned int precision)
  {
    //free(tableH_m);
    //free(tableL_m);
    //tableH_m=(float*)malloc(sizeof(float)*(1 << 9));
    //tableL_m=(float*)malloc(sizeof(float)*(1 << 9));
    setTable(tableH_m,  9, 9, false);
    setTable(tableL_m, 18, 9, true);
    _precision = precision;
    FloatArray table = FloatArray::create(1 << precision);
    FastLog::fillTable(table, _precision);
    fastLog.setTable(table);
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
    const float t  = tableH_m[(i >> 14) & 0x1FF] * tableL_m[(i >> 5) & 0x1FF];
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


