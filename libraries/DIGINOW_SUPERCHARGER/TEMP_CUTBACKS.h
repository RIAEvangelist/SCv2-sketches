#ifndef SC_TEMP_CUTBACKS_H
#define SC_TEMP_CUTBACKS_H

//pretty universal
#define TEMP_CUTBACK_COUNT 32
//temp deg C cutback
const unsigned short OUTPUT_TEMP_CUTBACK[]={0,
  1,
  5,
  6,
  8,
  9,
  10,
  12,
  14,
  16,
  18,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  33,
  34,
  35,
  37,
  40};

//temp based C-Rate cutback
const unsigned long OUTPUT_TEMP_C_RATE_CUTBACK[]={1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30};

 #define OVER_TEMP_CUTBACK_COUNT 4
//temp deg C cutback
const unsigned short OUTPUT_OVER_TEMP_CUTBACK[]={50,
  60,
  70,
  75};

//temp based C-Rate cutback
const unsigned long OUTPUT_OVER_TEMP_C_RATE_CUTBACK[]={25,
  10,
  3,
  0};

#endif
