// IF the user is using a populated CHARGE TABLE, they should also modify
// lowTemp and highTemp for battery

//THE FOLLOWING MUST BE DEFINED BY THE USER PROGRAM
//
// using ints will save memory for the version
// #define VERSION 0
//
// //116.4 volts
// #define OUTPUT_VOLTS_MAX 1164
//
// //96.0 amps 1C Rate
//   long C_RATE=960;
//
// #define CHARGER_PIN 3 //CS PIN from your board
#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include "TEMP_CUTBACKS.h"

#define debugSCFlag 0

#define HeartbeatMessageID 0x1806E5F4
#define HeartbeatMessageLength 8

#define VoltageHighByte 0
#define VoltageLowByte 1
#define CurrentHighByte 2
#define CurrentLowByte 3

#define DelayItteration 85000
long delayItteration=DelayItteration;

//prepopulate end voltage with max voltage. if different in flash memory value in flash memory will be used.
unsigned long OUTPUT_VOLTS_END=OUTPUT_VOLTS_MAX;

unsigned long CPlus = 0;
short lowTemp = 0;
short highTemp = 0;

//50.0 volts
#define OUTPUT_VOLTS_MIN 500

//50.00 watts
unsigned long RAMP_RATE=5000;

//6600.00
#define STATION_WATTS_DEFAULT 660000

//60.0 watts
#define MAX_12V_WATTS 600

//target wattage
unsigned long MAX_STATION_WATTS = STATION_WATTS_DEFAULT; //0;
//target amps to charge calculated by code
//uses MAX_STATION_WATTS and volts
unsigned short chargingAmps = 0;

int chargerCount = 0;

//previous watts
unsigned long watts = 0;

short volts=0;

MCP_CAN Charger(CHARGER_PIN);

unsigned char HeartbeatMessage[HeartbeatMessageLength] = {
  // Voltage, high then low byte in volts
  //default 116.4
  0x04, 0x8c,
  // Current, high then low byte in amps
  //default 32.0
  0x01, 0x40,
  // Reserved/status
  0x00, 0x00, 0x00, 0x00
};

void setChargingAmps(){
  //if we are not running at full tilt
  //or if we are not yet started this will be a slow ramp
  if(watts<MAX_STATION_WATTS){
    watts=watts+RAMP_RATE;
  }

  //check for cutback requirement scenarios.
  for (int i = 0; i < CUTBACK_COUNT; i++) {
    long maxAmps=OUTPUT_C_RATE_CUTBACK[i]*C_RATE/10;

    if(CPlus<1 && maxAmps>C_RATE){
      maxAmps=C_RATE;
    }

    long maxWatts=maxAmps*volts;

    if(maxWatts>MAX_STATION_WATTS){
      maxWatts=MAX_STATION_WATTS;
    }

    if (volts > OUTPUT_VOLTS_CUTBACK[i] && watts > maxWatts) {
      watts=maxWatts;
    }

    if(watts > maxWatts) {
      watts=maxWatts;
    }
  }

  //check for under thermal cutback requirement scenarios.
  for (int i = 0; i <TEMP_CUTBACK_COUNT ; i++) {
    long maxAmps=OUTPUT_TEMP_C_RATE_CUTBACK[i]*C_RATE/10;

    if(CPlus<1 && maxAmps>C_RATE){
      maxAmps=C_RATE;
    }

    long maxWatts=maxAmps*volts;
    if(maxWatts>MAX_STATION_WATTS){
      maxWatts=MAX_STATION_WATTS;
    }

    short temp=OUTPUT_TEMP_CUTBACK[i];


    //if cold
    if (
      //if battery cold or not detected
      lowTemp <= temp
    ){
      if(watts > maxWatts) {
        watts=maxWatts;
      }
    }
  }

  //check for over thermal cutback requirement scenarios.
  for (int i = 0; i <OVER_TEMP_CUTBACK_COUNT ; i++) {
    long maxAmps=OUTPUT_OVER_TEMP_C_RATE_CUTBACK[i]*C_RATE/10;

    if(CPlus<1 && maxAmps>C_RATE){
      maxAmps=C_RATE;
    }

    long maxWatts=maxAmps*volts;
    if(maxWatts>MAX_STATION_WATTS){
      maxWatts=MAX_STATION_WATTS;
    }

    short temp=OUTPUT_OVER_TEMP_CUTBACK[i];

    //if hot
    if (
      //if hot battery
      highTemp >= temp
    ){
      if(watts > maxWatts) {
        watts=maxWatts;
      }
    }
  }

  //slow the charger for this cycle if the end voltage reached
  if (volts >= OUTPUT_VOLTS_END) {
    if (watts > RAMP_RATE*3) {
      watts=watts-RAMP_RATE*3;
    }else{
      watts=0;
    }
  }

  //stop the charger for this cycle if the max voltage reached
  if (volts > OUTPUT_VOLTS_MAX || volts < OUTPUT_VOLTS_MIN) {
   watts=0;
  }

  //if we want more than the station can deliver
  if (watts > MAX_STATION_WATTS) {
    watts = MAX_STATION_WATTS - MAX_12V_WATTS*chargerCount;
  }

  chargingAmps = watts / volts;

  if(CPlus<1){
    if(chargingAmps>C_RATE){
      chargingAmps=C_RATE;
    }
  }

  chargingAmps = chargingAmps / chargerCount;
}

void superChargerSetup(){
  // initialize the CAN bus at baud rate 250kbps
  while (CAN_OK != Charger.begin(CAN_250KBPS)) {
    delay(100);
  }
}

void superChargerLoop(){
  while (CAN_MSGAVAIL == Charger.checkReceive()) {
    // The charger sends out every second a CAN status message with voltage, current and status information.
    unsigned char ReceivedChargerMessageLength = 0;
    unsigned char ReceivedChargerMessage[8];
    char str[20];
    Charger.readMsgBuf(&ReceivedChargerMessageLength, ReceivedChargerMessage);
    volts = word(ReceivedChargerMessage[VoltageHighByte], ReceivedChargerMessage[VoltageLowByte]);
  }
  setChargingAmps();
}

void superCharge(){
  ByteToShort voltage;
  voltage.value=OUTPUT_VOLTS_END;
  HeartbeatMessage[VoltageHighByte] = voltage.bytes[VoltageHighByte];
  HeartbeatMessage[VoltageLowByte] = voltage.bytes[VoltageLowByte];

  HeartbeatMessage[CurrentHighByte] = highByte(chargingAmps);
  HeartbeatMessage[CurrentLowByte] = lowByte(chargingAmps);

  Charger.sendMsgBuf(HeartbeatMessageID, 1, HeartbeatMessageLength, HeartbeatMessage);
}
