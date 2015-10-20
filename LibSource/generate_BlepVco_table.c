/* 
 * minblep.c
 * MinBLEP Generation Code
 * By Daniel Werner
 *
 * Bug fixes, adaptation for FFTW use, and discontinuity delta
 * generation code by Sean Bolton
 *
 * Cubic interpolation code by Steve Harris
 *
 * Copyright (C) 2005 Sean Bolton.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 */

/*
  By Giulio Moro
  The minblepVCOs have been ported for the OWL programmable
  effect pedal [http://hoxtonowl.com].
  As the size of the patch has to be smaller than 64k, we only want 
  to store the minblep table (which is smaller).
  
  step_dd_table and slope_dd_table will then be generated at runtime
  in the constructor
  
  The present file will dump to minblep_table.c the minblep table
USAGE: if you already have a file minblep_table.c you are probably
  happy, otherwise:
  - you need to have fftw installed.
  - compile, run and clean binary with 
    $ gcc --std=c99 minblep.c -lfftw -lm -o minblep && ./minblep && rm minblep
    this will generate a file minblep_minblep_table.c
    
  This assumes you have a valid minblep_table.h file with the following content::

#ifndef _MINBLEP_TABLES_H
#define _MINBLEP_TABLES_H
// minBLEP constants
// minBLEP table oversampling factor (must be a power of two):
#define MINBLEP_PHASES          64
//  MINBLEP_PHASES minus one: 
#define MINBLEP_PHASE_MASK      (MINBLEP_PHASES-1)
//  length in samples of (truncated) step discontinuity delta:
#define STEP_DD_PULSE_LENGTH    72
//  length in samples of (truncated) slope discontinuity delta:
#define SLOPE_DD_PULSE_LENGTH   71
//  the longer of the two above:
#define LONGEST_DD_PULSE_LENGTH STEP_DD_PULSE_LENGTH
//  delay between start of DD pulse and the discontinuity, in samples:
#define DD_SAMPLE_DELAY          4

extern double minblep_table[];
#endif

*/
#define _BSD_SOURCE    1
#define _SVID_SOURCE   1
#define _ISOC99_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <fftw.h>

#define CHECK_WRITE_FILE(ret) if(ret < 0){fprintf(stderr, "ERROR: Unable to write to file\n");return 1;}

/* SINCM Function */
double SINCM(double period, double x)
{
    double pix, m;
    int mi = (int)period;
    m = (mi & 1) ? (double)mi : (double)(mi - 1);

    if (fabs(x) < 1e-12)
        return m / period;
    else {
        pix = M_PI * x;
    return m / period * sin(pix * m / period) / (m * sin(pix / period));
    }
}

/* Generate Blackman Window */
void BlackmanWindow(int n, double *w)
{
  int   m = n - 1;
  int   i;
  double f1, f2, fm;

  fm = (double)m;
  for(i = 0; i <= m; i++)
  {
    f1 = (2.0 * M_PI * (double)i) / fm;
    f2 = 2.0 * f1;
    w[i] = 0.42 - (0.5 * cos(f1)) + (0.08 * cos(f2));
  }
}

/* Discrete Fourier Transform */
void DFT(int n, double *realTime, double *imagTime, double *realFreq, double *imagFreq)
{
    fftw_complex in[n], out[n];
    fftw_plan p;
    int i;

    for (i = 0; i < n; i++) {
        in[i].re = realTime[i];
        in[i].im = imagTime[i];
    }

    p = fftw_create_plan(n, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_one(p, in, out);
    fftw_destroy_plan(p);

    for (i = 0; i < n; i++) {
        realFreq[i] = out[i].re / (double)n;
        imagFreq[i] = out[i].im / (double)n;
    }
}

/* Inverse Discrete Fourier Transform */
void InverseDFT(int n, double *realTime, double *imagTime, double *realFreq, double *imagFreq)
{
    fftw_complex in[n], out[n];
    fftw_plan p;
    int i;

    for (i = 0; i < n; i++) {
        in[i].re = realFreq[i];
        in[i].im = imagFreq[i];
    }

    p = fftw_create_plan(n, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_one(p, in, out);
    fftw_destroy_plan(p);

    for (i = 0; i < n; i++) {
        realTime[i] = out[i].re;
        imagTime[i] = out[i].im;
    }
}

/* Complex Absolute Value */
inline double cabs(double x, double y)
{
  return sqrt((x * x) + (y * y));
}

/* Complex Exponential */
inline void cexp(double x, double y, double *zx, double *zy)
{
  double expx;
  
  expx = exp(x);
  *zx = expx * cos(y);
  *zy = expx * sin(y);
}

/* Compute Real Cepstrum Of Signal */
void RealCepstrum(int n, double *signal, double *realCepstrum)
{
  int    i;
  double *realTime, *imagTime, *realFreq, *imagFreq;

    realTime = (double *)calloc(n, sizeof(double));
    imagTime = (double *)calloc(n, sizeof(double));
    realFreq = (double *)calloc(n, sizeof(double));
    imagFreq = (double *)calloc(n, sizeof(double));

  /* Compose Complex FFT Input */

  for(i = 0; i < n; i++)
  {
    realTime[i] = signal[i];
    imagTime[i] = 0.0;
  }
  
  /* Perform DFT */

  DFT(n, realTime, imagTime, realFreq, imagFreq);

  /* Calculate Log Of Absolute Value */

  for(i = 0; i < n; i++)
  {
    realFreq[i] = log(cabs(realFreq[i], imagFreq[i]));
    imagFreq[i] = 0.0;
  }

  /* Perform Inverse FFT */

  InverseDFT(n, realTime, imagTime, realFreq, imagFreq);

  /* Output Real Part Of FFT */
  for(i = 0; i < n; i++)
    realCepstrum[i] = realTime[i];

    free(realTime);
    free(imagTime);
    free(realFreq);
    free(imagFreq);
}

/* Compute Minimum Phase Reconstruction Of Signal */
void MinimumPhase(int n, double *realCepstrum, double *minimumPhase)
{
  int i, nd2;
  double *realTime, *imagTime, *realFreq, *imagFreq;

    realTime = (double *)calloc(n, sizeof(double));
    imagTime = (double *)calloc(n, sizeof(double));
    realFreq = (double *)calloc(n, sizeof(double));
    imagFreq = (double *)calloc(n, sizeof(double));

  nd2 = n / 2;

  if((n % 2) == 1)
  {
    realTime[0] = realCepstrum[0];
    for(i = 1; i <= nd2; i++)
      realTime[i] = 2.0 * realCepstrum[i];
    for(; i < n; i++)
      realTime[i] = 0.0;
  }
  else
  {
    realTime[0] = realCepstrum[0];
    for(i = 1; i < nd2; i++)
      realTime[i] = 2.0 * realCepstrum[i];
    realTime[nd2] = realCepstrum[nd2];
    for(i = nd2 + 1; i < n; i++)
      realTime[i] = 0.0;
  }

  for(i = 0; i < n; i++)
    imagTime[i] = 0.0;

  DFT(n, realTime, imagTime, realFreq, imagFreq);
  
  for(i = 0; i < n; i++)
    cexp(realFreq[i], imagFreq[i], &realFreq[i], &imagFreq[i]);

  InverseDFT(n, realTime, imagTime, realFreq, imagFreq);

  for(i = 0; i < n; i++)
    minimumPhase[i] = realTime[i];

    free(realTime);
    free(imagTime);
    free(realFreq);
    free(imagFreq);
}

/* Generate MinBLEP And Return It In An Array Of Floating Point Values */
double *GenerateMinBLEP(int period, int zeroCrossings, double dilation, int oversampling, int *ww)
{
    int    i, winwidth;
    double a, *buffer1, *buffer2;
    double *minBLEP;
    int po = period * oversampling;
    int pod2 = po / 2;
    buffer1 = (double *)calloc(po, sizeof(double));
    buffer2 = (double *)calloc(po, sizeof(double));
    minBLEP = (double *)calloc(po, sizeof(double));
    winwidth = lrint(ceil((double)(zeroCrossings * 2 * oversampling) / dilation));
    if (!(winwidth & 1)) winwidth++;
    *ww = winwidth;

    /* Generate Sincm */
    for(i = 0; i < po; i++) {
        buffer1[i] = SINCM((double)period, (double)(i - pod2) / (double)oversampling * dilation);
    }

  /* Window Sinc */

#define WINDOW
#ifdef WINDOW
  BlackmanWindow(winwidth, buffer2 + pod2 - winwidth/2);
  for(i = 0; i < po; i++)
    buffer1[i] *= buffer2[i];
#endif

  /* Minimum Phase Reconstruction */

#define MINPHASE
#ifdef MINPHASE
  RealCepstrum(po, buffer1, buffer2);
  MinimumPhase(po, buffer2, buffer1);
#endif

  /* Integrate Into MinBLEP */
  a = 0.0;
  for(i = 0; i < po; i++)
  {
    a += buffer1[i];
#define INTEGRATE
#ifdef INTEGRATE
    minBLEP[i] = a;
#else
    minBLEP[i] = buffer1[i];
#endif
  }

  /* Normalize */
#ifdef INTEGRATE
  a = 1.0 / a;
#else
  a = (double)oversampling / a;
#endif
  for(i = 0; i < po; i++)
    minBLEP[i] *= a;

    free(buffer1);
    free(buffer2);

  return minBLEP;
}

/*****************************************************************************
 * Description: Interpolates between p0 and n0 taking the previous (p1)
 *              and next (n1) points into account, using a 3rd order
 *              polynomial (aka cubic spline)
 *
 *   Arguments: interval    Normalised time interval between inteprolated
 *                           sample and p0
 *              p1, p0      Samples prior to interpolated one
 *              n0, n1      Samples following interpolated one
 *
 *     Returns: interpolated sample
 *
 *        Code: Adapted from Steve Harris' plugin code
 *              swh-plugins-0.2.7/ladspa-util.h::cube_interp
 *              http://plugin.org.uk/releases/0.2.7/
 *
 *****************************************************************************/
static inline double
interpolate_cubic (double interval,
                   double p1,
                   double p0,
                   double n0,
                   double n1)
{
        return p0 + 0.5f * interval * (n0 - p1 +
                           interval * (4.0f * n0 + 2.0f * p1 - 5.0f * p0 - n1 +
                           interval * (3.0f * (p0 - n0) - p1 + n1)));
}

int
main(void)
{
    /* A 30Hz BLIT at 96Khz sample rate should be narrow enough for most
     * uses.... */
    int period = 96000 / 30;

    int oversampling = 64;
    int i, winwidth;

    /* Generate the minBLEP: */
    double *blep = GenerateMinBLEP(period,
                                   32,       /* zero crossings */
                                   0.9,      /* dilation */
                                   oversampling,
                                   &winwidth /* window width return */);
    printf("/* window width is %d */\n\n", winwidth);

    /* When combining the run-time-calculated 'ideal' or 'naive' waveform with
     * the pre-calculated minBLEP delta, it is useful to have the offset into
     * the delta of the discontinuity be an integral number of samples.  I'm
     * not sure what the correct way of determining where the discontinuity
     * 'center' of the minBLEP occurs, but it doesn't really matter with this
     * technique, as long as it's reasonably close, and the offsets used in
     * both delta pre-calculation and run-time output construction are exactly
     * the same.  What I did to find the 'center' of the minBLEP was integrate
     * the minBLEP, then extrapolate its ramp slope back to the zero crossing
     * (218.2761946 oversamples), then subtract where I wanted the 'ideal'
     * discontinuity to be (4 samples * 64 = 256 oversamples).
     *
     * And I'm curious: might there be a way to do a near-minimum-phase
     * reconstruction of the BLEP, so that its 'center' falls at a chosen
     * offset?
     */
    double shift = 218.27619467838002 - 256.0;  /* good only for 32000/32/0.9/64 minBLEP */

    int index;
    double frac, t1, t2, a, d;
    double *shiftblep = (double *)calloc(period * oversampling, sizeof(double));

    /* Shift the minBLEP. */
    for (i = 0; i < period * oversampling; i++) {
        if ((double)(i - 1) + shift < 0.0) {
            shiftblep[i] = 0.0;
        } else {
            frac = (double)i + shift;
            index = lrint(frac - 0.5);
            frac -= (double)index;
            shiftblep[i] = interpolate_cubic(frac, blep[index - 1], blep[index],
                                             blep[index + 1], blep[index + 2]);
        }
    }

    FILE *file = fopen("minblep_table.c", "w");
    if(file == NULL){
        fprintf(stderr, "ERROR: Unable to open output file\n");
        return 1;
    }
    int ret;
    ret = fprintf(file, "int period = %d;\n", period);
    CHECK_WRITE_FILE(ret);
    ret = fprintf(file, "int oversampling = %d;\n", oversampling);
    CHECK_WRITE_FILE(ret);
    ret = fprintf(file, "double minblep_table[] ={\n");
    CHECK_WRITE_FILE(ret);
    
    for( i = 0; i < 4609; i++){
        ret = fprintf(file, "%.20f,\n", shiftblep[i]);
        CHECK_WRITE_FILE(ret);
    }
    
    ret = fprintf(file, "\n};\n");
    CHECK_WRITE_FILE(ret);
    
    ret = fclose(file);
    if(ret < 0){
        fprintf(stderr, "ERROR: Unable to close output file\n");
        return 1;
    }
    printf("SUCCESS: Succesfully written to file!\n");
    free(blep);
    free(shiftblep);
    return 0;
    // Daniel Werner's program continued with the below lines, while 
    // we exit earlier and will do those operations inside the BlepVco
    // class constructor.
    
    
    
    /* Dump the step discontinuity delta table, which is the minBLEP minus the
     * 'ideal' unit step.  Note that since the step delta is discontinuous,
     * the common interpolation form:
     *
     *     value = sample[index] + fraction * (sample[index + 1] - sample[index])
     *
     * would fail at the discontinuity.  So instead, we store the table as
     * pairs are numbers, the first being the delta's value at a particular
     * index 'n', and the second being the difference between that value and
     * the limit of the delta's value as the index approaches n + 1.
     */
    printf("typedef struct { float value, delta; } float_value_delta;\n");
    printf("float_value_delta step_dd_table[] =\n{\n");

    for (i = 0; i < period * oversampling; i++) {

        if (i % 4 == 0) {
            if (i == 0) printf(" ");
            else printf("\n ");
        }

        /* Alas, more empirically derived constants....  The full minBLEP is
         * period * oversampling (2048000) oversamples long, but we only need
         * the first part of it approximately as long as the window width
         * (4553) (approximate because the minimum-phase reconstruction smears
         * the window a bit.)  I examined the full step delta and found a zero
         * crossing at the 4606th oversample, beyond which the delta stays
         * below -150dB, and truncated the pulse there, which, padded with zeros,
         * yields a 72-sample pulse. */
        if (i <= 4605) {

            t1 = shiftblep[i];
            t2 = shiftblep[i + 1];
            if (i == 4605) t2 = 1.0;
            if (i >= 4 * oversampling) {  /* subtract the ideal step */
                t1 -= 1.0;
                t2 -= 1.0;
            }

            printf("{%13.6e,%13.6e},", t1, t2 - t1);

        } else if (i < 72 * 64) {

            /* pad remainder of last sample with zeros */
            printf("{%13.6e,%13.6e},", 0.0, 0.0);

        } else if (i == 72 * 64) {

            printf("{%13.6e,%13.6e}", 0.0, 0.0);  /* guard point */
            break;

        }
    }
    printf("\n};\n\n");

    /* Dump the slope discontinuity delta table, which is the integral of the
     * minBLEP minus the 'ideal' unit slope change.  While its slope, or first
     * differential, is discontinuous, the delta itself is continuous, so we
     * can just store the table as single values.
     */
    printf("float slope_dd_table[] =\n{\n");

    a = 0.0;
    for (i = 0; i < period * oversampling; i++) {

        if (i % 8 == 0) {
            if (i == 0) printf(" ");
            else printf("\n ");
        }

        /* More emperically derived constants  (local minimum at 4513th
         * oversample, yielding a 71-sample pulse).... */
        if (i <= 4512) {

            a += shiftblep[i] / (double)oversampling;
            if (i < 4 * oversampling) {
                d = a;
            } else {
                d = a - (double)(i - 4 * oversampling) / (double)oversampling;   /* subtract ideal unit slope */
            }
            printf("%13.6e,", d);

        } else if (i < 71 * 64) {

            /* pad remainder of last sample with zeros */
            printf("%13.6e,", 0.0f);

        } else if (i == 71 * 64) {

            printf("%13.6e", 0.0f);  /* guard point */
            break;

        }
    }
    printf("\n};\n\n");

    free(blep);
    free(shiftblep);
    return 0;
}

