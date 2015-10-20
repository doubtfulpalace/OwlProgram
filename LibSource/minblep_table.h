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