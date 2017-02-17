
/**
 * Following the bit-twiddling idea in:
 *
 * 'A Fast, Compact Approximation of the Exponential Function'
 * Technical Report IDSIA-07-98
 * Nicol N. Schraudolph;
 * IDSIA,
 * 1998-06-24.
 *
 * [Rewritten for floats by HXA7241, 2007.]
 *
 * and the adjustable-lookup idea in:
 *
 * 'Revisiting a basic function on current CPUs: A fast logarithm implementation
 * with adjustable accuracy'
 * Technical Report ICSI TR-07-002;
 * Oriol Vinyals, Gerald Friedland, Nikki Mirghafori;
 * ICSI,
 * 2007-06-21.
 *
 * [Improved (doubled accuracy) and rewritten by HXA7241, 2007.]
 */


static const float _2p23 = 8388608.0f;


/**
 * Initialize powFast lookup table.
 *
 * @pTable     length must be 2 ^ precision
 * @precision  number of mantissa bits used, >= 0 and <= 18
 */
static void powFastSetTable
(
   unsigned int* const pTable,
   const unsigned int  precision
)
{
   /* step along table elements and x-axis positions */
   float zeroToOne = 1.0f / ((float)(1 << precision) * 2.0f);
   int   i;
   for( i = 0;  i < (1 << precision);  ++i )
   {
      /* make y-axis value for table element */
      const float f = ((float)pow( 2.0f, zeroToOne ) - 1.0f) * _2p23;
      pTable[i] = (unsigned int)( f < _2p23 ? f : (_2p23 - 1.0f) );

      zeroToOne += 1.0f / (float)(1 << precision);
   }
}


/**
 * Get pow (fast!).
 *
 * @val        power to raise radix to
 * @ilog2      one over log, to required radix, of two
 * @pTable     length must be 2 ^ precision
 * @precision  number of mantissa bits used, >= 0 and <= 18
 */
inline
static float powFastLookup
(
   const float         val,
   const float         ilog2,
   unsigned int* const pTable,
   const unsigned int  precision
)
{
   /* build float bits */
   const int i = (int)( (val * (_2p23 * ilog2)) + (127.0f * _2p23) );

   /* replace mantissa with lookup */
   const int it = (i & 0xFF800000) | pTable[(i & 0x7FFFFF) >> (23 - precision)];

   /* convert bits to float */
   union { int i; float f; } pun;
   return pun.i = it,  pun.f;
}




/*- basic object services ----------------------------------------------------*/

PowFast* powFastCreate
(
   unsigned int precision
)
{
   PowFast* pPowFast = 0;

   precision = (precision <= 18u) ? precision : 18u;

   pPowFast = (PowFast*)malloc( sizeof(PowFast) + ((1 << precision) *
      sizeof(*(pPowFast->pTable_m))) );
   if( pPowFast )
   {
      pPowFast->precision_m = precision;
      pPowFast->pTable_m   = (unsigned int*)((char*)pPowFast + sizeof(PowFast));

      powFastSetTable( pPowFast->pTable_m, pPowFast->precision_m );
   }

   /*PowFast* pPowFast = (PowFast*)malloc( sizeof(PowFast) );
   if( pPowFast )
   {
      pPowFast->precision_m = (precision <= 18u) ? precision : 18u;
      pPowFast->pTable_m    = (unsigned int*)malloc( (1 << precision) *
         sizeof(*(pPowFast->pTable_m)) );

      if( pPowFast->pTable_m )
      {
         powFastSetTable( pPowFast->pTable_m, pPowFast->precision_m );
      }
      else
      {
         free( pPowFast );
         pPowFast = 0;
      }
   }*/

   return pPowFast;
}


int powFastFree
(
   const PowFast* powFast
)
{
   PowFast* ppf = (PowFast*)powFast;

   /*free( ppf->pTable_m );*/
   free( ppf );

   return 1;
}




/*- queries ------------------------------------------------------------------*/

float powFast2
(
   const PowFast* powFast,
   float       f
)
{
   const PowFast* ppf = (const PowFast*)powFast;

   return powFastLookup( f, 1.0f, ppf->pTable_m, ppf->precision_m );
}


float powFastE
(
   const PowFast* powFast,
   float       f
)
{
   const PowFast* ppf = (const PowFast*)powFast;

   return powFastLookup( f, 1.44269504088896f, ppf->pTable_m, ppf->precision_m);
}


float powFast10
(
   const PowFast* powFast,
   float       f
)
{
   const PowFast* ppf = (const PowFast*)powFast;

   return powFastLookup( f, 3.32192809488736f, ppf->pTable_m, ppf->precision_m);
}


float powFast
(
   const PowFast* powFast,
   float       logr,
   float       f
)
{
   const PowFast* ppf = (const PowFast*)powFast;

   return powFastLookup( f, (logr * 1.44269504088896f),
      ppf->pTable_m, ppf->precision_m );
}


unsigned int powFastPrecision
(
   const PowFast* powFast
)
{
   const PowFast* ppf = (const PowFast*)powFast;

   return ppf->precision_m;
}

