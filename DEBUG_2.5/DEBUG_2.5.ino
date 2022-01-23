#include <EEPROMex.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SoftwareSerial.h>

#include <SPI.h>

#include <ZERO.h>
#include "Network.h"
#include "unions.h"

#define CHARGER_PIN 9
#define BIKE_PIN 10
#define btRX A0
#define btTX A1

// set up a new serial object
SoftwareSerial btSerial (btRX, btTX);


Zero zero;

#define VERSION 3
#define BAUD_RATE 38400
#define HeartbeatMessageID 0x1806E5F4
#define HeartbeatMessageLength 8

#define VoltageHighByte 0
#define VoltageLowByte 1
#define CurrentHighByte 2
#define CurrentLowByte 3

#define TargetWattsAddress 0
#define TargetVoltsAddress 4
#define TargetChargerCountAddress 8
#define TargetMAX_AMPSAddress 12
#define Target1CPlusAddress 16
#define TargetRAMP_RATEAddress 20

//6600.00
#define STATION_WATTS_DEFAULT 660000
//76.0 watts
#define MAX_12V_WATTS 600

//116.4 volts
#define OUTPUT_VOLTS_MAX 1164

//116.4 volts
unsigned long OUTPUT_VOLTS_END=OUTPUT_VOLTS_MAX;

unsigned long CPlus = 0;

//50.0 volts
#define OUTPUT_VOLTS_MIN 700

//50 watts
unsigned long RAMP_RATE=5000;

//1C Rate / charger count
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
  10,
  7,
  6};

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
const unsigned long OUTPUT_TEMP_C_RATE_CUTBACK[]={2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
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



//target wattage
unsigned long MAX_STATION_WATTS = 0;
//target amps to charge calculated by code
//uses MAX_STATION_WATTS and volts
unsigned short chargingAmps = 0;

int chargerCount = 0;

//previous watts
unsigned long watts = 0;

const int PROGMEM SPI_CS_PIN = 9;

MCP_CAN Bike(BIKE_PIN);
MCP_CAN Charger(CHARGER_PIN);

long delayItteration=55000;
ByteToShort bikeThrottle;

short volts=0;

ByteToLong bikeVolts;
ByteToUnsignedShort bikeAmps;

ByteToLong monolithVolts;
ByteToLong powerTankVolts;

ByteToLong monolithAmps;
ByteToLong powerTankAmps;

ByteToLong monolithAH;
ByteToLong powerTankAH;

ByteToLong monolithMinTemp;
ByteToLong powerTankMinTemp;

ByteToLong monolithMaxTemp;
ByteToLong powerTankMaxTemp;


byte monolithContactorClosed=0;
byte powerTankContactorClosed=0;

unsigned char HeartbeatMessage[HeartbeatMessageLength] = {
  // Voltage, high then low byte in volts
  //default 116.4 not overwritten anywhere,
  //this could be dynamic
  0x04, 0x8c,
  // Current, high then low byte in amps
  //default 32.0 overwritten immediately unless no can messages recieved
  //this can happen on some older units when on 120v
  0x01, 0x40,
  // Reserved/status
  0x00, 0x00, 0x00, 0x00
};

void setChargingAmps(){
  if(monolithVolts.value/100>volts){
    volts=monolithVolts.value/100;
  }

  if(powerTankVolts.value/100>volts){
    volts=powerTankVolts.value/100;
  }

  if(monolithAH.value>0 || powerTankAH.value>0 ){
    C_RATE=(monolithAH.value + powerTankAH.value)*10;
  }

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
      //if monolith cold or not detected
      monolithMinTemp.value < temp ||
      //if PT AND it is cold
      (powerTankAH.value>0 && powerTankMinTemp.value < temp)
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
      //if hot monolith
      monolithMaxTemp.value > temp ||
      //if PT AND it is hot
      (powerTankAH.value > 0 && powerTankMaxTemp.value > temp)
    ){
      if(watts > maxWatts) {
        watts=maxWatts;
      }
    }
  }

  //slow the charger for this cycle if the end voltage reached
  if (volts > OUTPUT_VOLTS_END) {
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

    if(monolithAH.value>0 && monolithAmps.value>monolithAH.value){
      chargingAmps=(monolithAH.value-2)*10;
    }

    if(powerTankAH.value>0 && powerTankAmps.value>powerTankAH.value){
      chargingAmps=(powerTankAH.value-2)*10;
    }
  }

  bikeAmps.value=chargingAmps;

  chargingAmps = chargingAmps / chargerCount;
}

void readCommands(){
  while(btSerial.available()){
    String commandVolts = btSerial.readStringUntil(',');
    String commandMAX_AMPS = btSerial.readStringUntil(',');
    String commandChargers = btSerial.readStringUntil(',');
    String commandWatts = btSerial.readStringUntil(',');
    String command1CPlus = btSerial.readStringUntil(',');
    String commandRAMP_RATE = btSerial.readStringUntil('\n');

    unsigned long targetVolts = commandVolts.toInt();
    unsigned long targetMAX_AMPS = commandMAX_AMPS.toInt();
    unsigned long targetChargers = commandChargers.toInt();
    unsigned long targetWattage = commandWatts.toInt();
    unsigned long target1CPlus = command1CPlus.toInt();
    unsigned long targetRAMP_RATE = commandRAMP_RATE.toInt();


    if(targetVolts>=OUTPUT_VOLTS_MIN && targetVolts<=OUTPUT_VOLTS_MAX){
      OUTPUT_VOLTS_END=targetVolts;
      EEPROM.writeLong(TargetVoltsAddress, OUTPUT_VOLTS_END);
    }

    if(targetMAX_AMPS>=0){
      C_RATE=targetMAX_AMPS*10;
      EEPROM.writeLong(TargetMAX_AMPSAddress, C_RATE);
    }

    if(targetChargers>=0){
      chargerCount=targetChargers;
      EEPROM.writeLong(TargetChargerCountAddress, chargerCount);
    }

    if(targetWattage>=0){
      MAX_STATION_WATTS=targetWattage*100;
      EEPROM.writeLong(TargetWattsAddress, MAX_STATION_WATTS);
    }

    if(target1CPlus>=0){
      CPlus=target1CPlus;
      EEPROM.writeLong(Target1CPlusAddress, CPlus);
    }

    if(targetRAMP_RATE>=0){
      RAMP_RATE=targetRAMP_RATE*100;
      EEPROM.writeLong(TargetRAMP_RATEAddress, RAMP_RATE);
    }
  }
}

void setup() {
  OUTPUT_VOLTS_END=EEPROM.readLong(TargetVoltsAddress);
  C_RATE=EEPROM.readLong(TargetMAX_AMPSAddress);
  chargerCount=EEPROM.readLong(TargetChargerCountAddress);
  MAX_STATION_WATTS=EEPROM.readLong(TargetWattsAddress);
  CPlus=EEPROM.readLong(Target1CPlusAddress);
  RAMP_RATE=EEPROM.readLong(TargetRAMP_RATEAddress);

  
  if (btSerial) {
      // define pin modes for tx, rx:
      pinMode(btRX, INPUT);
      pinMode(btTX, OUTPUT);
      // set the data rate for the SoftwareSerial port
      btSerial.begin(BAUD_RATE);
      btSerial.println("STARTING DEBUG PROCESS");
  }

  // initialize the CAN bus at baud rate 250kbps
  while (CAN_OK != Charger.begin(CAN_250KBPS)) {
    if (btSerial) {
      btSerial.println("CHARGER CAN START FAILED");
    }
    delay(100);
  }

  // initialize the CAN bus at baud rate 250kbps
  while (CAN_OK != Bike.begin(CAN_500KBPS)) {
    if (btSerial) {
      btSerial.println("BIKE CAN START FAILED");
    }
    delay(100);
  }
}

void loop() {

  readCommands();

  while (CAN_MSGAVAIL == Charger.checkReceive()) {
    // The charger sends out every second a CAN status message with voltage, current and status information.
    unsigned char ReceivedChargerMessageLength = 0;
    unsigned char ReceivedChargerMessage[8];
    char str[20];
    Charger.readMsgBuf(&ReceivedChargerMessageLength, ReceivedChargerMessage);
    volts = word(ReceivedChargerMessage[VoltageHighByte], ReceivedChargerMessage[VoltageLowByte]);
  }

  while(CAN_MSGAVAIL == Bike.checkReceive()){
      byte len = 0;
      byte buf[zero.messageLength];

      Bike.readMsgBuf(&len, buf);

      short canId = Bike.getCanId();

      if(zero.hasMonolithVoltage(canId)){
        monolithVolts.value= zero.voltage(len,buf,canId);
        bikeVolts.value=monolithVolts.value;
      }

      if(zero.hasMonolithAmps(canId)){
        monolithAmps.value= abs(zero.amps(len,buf,canId));
        bikeAmps.value=monolithAmps.value+powerTankAmps.value;
        if(monolithAH.value>0 && monolithAmps.value>monolithAH.value && CPlus<1 && delayItteration>10){
          delayItteration=10;
        }
      }

      if(zero.hasPowerTankVoltage(canId)){
        powerTankVolts.value= zero.voltage(len,buf,canId);
      }

      if(zero.hasPowerTankAmps(canId)){
        powerTankAmps.value= abs(zero.amps(len,buf,canId));
        bikeAmps.value=monolithAmps.value+powerTankAmps.value;
        if(powerTankAH.value>0 && powerTankAmps.value>powerTankAH.value && CPlus<1 && delayItteration>10){
          delayItteration=10;
        }
      }

      if(zero.hasMonolithPackConfig(canId)){
        monolithAH.value= zero.AH(len,buf,canId);
      }

      if(zero.hasPowerTankPackConfig(canId)){
        powerTankAH.value= zero.AH(len,buf,canId);
      }

      if(zero.hasMonolithPackActiveData(canId)){
        monolithMaxTemp.value= zero.highestTemp(len,buf,canId);
        monolithMinTemp.value= zero.lowestTemp(len,buf,canId);
      }

      if(zero.hasPowerTankPackActiveData(canId)){
        powerTankMaxTemp.value= zero.highestTemp(len,buf,canId);
        powerTankMinTemp.value= zero.lowestTemp(len,buf,canId);
      }

      if(zero.hasThrottle(canId)){
        bikeThrottle.value= zero.throttle(len,buf,canId);
      }

      if(bikeThrottle.value<300){
        if(canId==DASH_STATUS3.id){
          ByteToLong displayVolts;
          displayVolts.value=volts*10*KM_TO_MI;
          buf[2]=displayVolts.bytes[0];
          buf[3]=displayVolts.bytes[1];
          Bike.sendMsgBuf(DASH_STATUS3.id, 0, len, buf);
        }

        if(canId==DASH_STATUS2.id){
          ByteToLong bikeWatts;
          bikeWatts.value=watts*MI_TO_KM*100;
          buf[4]=bikeWatts.bytes[0];
          buf[5]=bikeWatts.bytes[1];
          Bike.sendMsgBuf(DASH_STATUS2.id, 0, len, buf);
        }
      }
  }

  if(delayItteration>0){
    delayItteration--;
  }

  if(delayItteration<1){
    delayItteration=22000;

    setChargingAmps();

    HeartbeatMessage[CurrentHighByte] = highByte(chargingAmps);
    HeartbeatMessage[CurrentLowByte] = lowByte(chargingAmps);


    if (btSerial) {
      //battery volts
      btSerial.print(volts);
      btSerial.print(",");

      //battery volts
      btSerial.print(monolithVolts.value/100);
      btSerial.print(",");

      //battery volts
      btSerial.print(powerTankVolts.value/100);
      btSerial.print(",");

      //total charging amps
      btSerial.print(chargingAmps*chargerCount);
      btSerial.print(",");

      //total monolith amps
      btSerial.print(monolithAmps.value);
      btSerial.print(",");

      //total power tank amps
      btSerial.print(powerTankAmps.value);
      btSerial.print(",");

      //monolith C Rate
      btSerial.print(monolithAH.value);
      btSerial.print(",");

      //power tank C Rate
      btSerial.print(powerTankAH.value);
      btSerial.print(",");

      //c rate
      btSerial.print(C_RATE/10);
      btSerial.print(",");

      //monolith temps
      btSerial.print(monolithMinTemp.value);
      btSerial.print(",");
      btSerial.print(monolithMaxTemp.value);
      btSerial.print(",");

      //powertank temps
      btSerial.print(powerTankMinTemp.value);
      btSerial.print(",");
      btSerial.print(powerTankMaxTemp.value);
      btSerial.print(",");

      //charger count
      btSerial.print(chargerCount);
      btSerial.print(",");

      //target watts
      btSerial.print(watts/100);
      btSerial.print(",");

      //MAX_STATION_WATTS
      btSerial.print(MAX_STATION_WATTS/100);
      btSerial.print(",");

      //1C Plus
      btSerial.print(CPlus);
      btSerial.print(",");

      //RAMP_RATE
      btSerial.print(RAMP_RATE/100);

      //start extending with a comma
      btSerial.print(",");

      //print api version
      btSerial.println(VERSION);
    }

    byte chargingBuf[8];

    chargingBuf[0]=0x1;
//    Bike.sendMsgBuf(CCU_CHARGE_STATUS.id, 0, zero.messageLength, chargingBuf);
//    Bike.sendMsgBuf(CALEXMX_CHARGE_STATUS.id, 0, zero.messageLength, chargingBuf);
//    Bike.sendMsgBuf(CALEX_CHARGE_STATUS.id, 0, zero.messageLength, chargingBuf);

    // frame status = extended in second arg
    Charger.sendMsgBuf(HeartbeatMessageID, 1, HeartbeatMessageLength, HeartbeatMessage);
  }
}
