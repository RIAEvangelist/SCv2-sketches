#ifndef SC_VOLTAGE_TABLE_ZERO_H
#define SC_VOLTAGE_TABLE_ZERO_H

//116.4 volts
#define OUTPUT_VOLTS_MAX 1164

//96.0 amps 1C Rate / charger count
long C_RATE=960;

//number of cutback points
#define CUTBACK_COUNT 10
//***Lifted*** voltage x 10 cutback
const unsigned short OUTPUT_VOLTS_CUTBACK[]={900,
  980,
  1000,
  1010,
  1020,
  1060,
  1100,
  1140,
  1150,
  1160};

//max C-RATE * 10 at ***Lifted*** voltage above
const unsigned long OUTPUT_C_RATE_CUTBACK[]={30,
  28,
  25,
  23,
  20,
  18,
  13,
  7,
  4,
  2};

#endif
