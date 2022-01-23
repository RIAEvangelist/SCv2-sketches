#ifndef SC_VOLTAGE_TABLE_0_BASIC_H
#define SC_VOLTAGE_TABLE_0_BASIC_H

//140.0 volts
#define OUTPUT_VOLTS_MAX 1400

//number of cutback points

//Voltages and crate cutbacks should be done specific to the battery
#define CUTBACK_COUNT 2
//***Lifted*** voltage x 10 cutback
const unsigned short OUTPUT_VOLTS_CUTBACK[]={200,1320};

//max C-RATE * 10 at ***Lifted*** voltage above
const unsigned long OUTPUT_C_RATE_CUTBACK[]={30,1};

#endif
