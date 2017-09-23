#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include <SPI.h>

#define BAUD_RATE 38400
#define HeartbeatMessageID 0x1806E5F4
#define HeartbeatMessageLength 8

#define VoltageHighByte 0
#define VoltageLowByte 1
#define CurrentHighByte 2
#define CurrentLowByte 3

const int SPI_CS_PIN = 9;

//6600.0kw
const unsigned long STATION_WATTS_GOOD = 660000;

//76.0 watts
const unsigned long MAX_12V_WATTS = 7600;

//116.4 volts
const unsigned short OUTPUT_VOLTS_MAX = 1164;
//50.0 volts
const unsigned short OUTPUT_VOLTS_MIN = 500;
//32.0 amps
const unsigned short OUTPUT_AMPS_MAX =  320;

//50 watts
const unsigned short RAMP_RATE = 5000;

//target wattage
unsigned long MAX_STATION_WATTS = 0;
//target amps to charge calculated by code
//uses MAX_STATION_WATTS and volts
unsigned short chargingAmps = 0;

int chargerCount = 0;
//current voltage
unsigned short volts = 0;
//charging amps
unsigned short amps = 0;
//previous watts
unsigned long lastWatts = 0;
//charging watts
unsigned long watts = 0;
//estimated watts being pulled from station
unsigned long stationWatts =0;

MCP_CAN CAN(SPI_CS_PIN);

unsigned char HeartbeatMessage[HeartbeatMessageLength] = {
  // Voltage, high then low byte in volts
  //default 116.4 not overwritten anywhere,
  //this could be dynamic
  0x04, 0x8c,
  // Current, high then low byte in amps
  //default 32.0 overwritten immediately
  0x01, 0x40,
  // Reserved/status
  0x00, 0x00, 0x00, 0x00
};

// The charger sends out every second a CAN status message with voltage, current and status information.
unsigned char flagRecv = 0;
unsigned char ReceivedChargerMessageLength = 0;
unsigned char ReceivedChargerMessage[8];
char str[20];

void MCP2515_ISR() {
    flagRecv = 1;
}

void setChargingAmps(){
  chargingAmps = watts / volts / chargerCount;
}

void setUpStationLimits(){
  //default to good station if max not yet set
  if(MAX_STATION_WATTS==0){
    MAX_STATION_WATTS = STATION_WATTS_GOOD;
  }
}

void readCommands(){
  while(Serial.available()){
    String commandWatts = Serial.readStringUntil("\n");

    Serial.println(commandWatts);

    // And again parse that as an int.
    unsigned long targetWattage = commandWatts.toInt();

    Serial.println(targetWattage);

    Serial.println(targetWattage>=0);

    if(targetWattage>=0){
      MAX_STATION_WATTS=targetWattage;
    }

    Serial.println(MAX_STATION_WATTS);
  }
}

void setup() {
  if (Serial) {
    Serial.begin(BAUD_RATE);
  }

  // initialize the CAN bus at baud rate 250kbps
  while (CAN_OK != CAN.begin(CAN_250KBPS)) {
    delay(100);
  }

  attachInterrupt(0, MCP2515_ISR, FALLING);
}

void loop() {

  setUpStationLimits();

  readCommands();

  if (flagRecv) {
      flagRecv = 0;

       // How many messages have we received from chargers in this loop
      // represents how many chargers are connected/live.
      chargerCount = 0;
      volts = 0;
      amps = 0;
      lastWatts=watts;
      watts = 0;

      while (CAN_MSGAVAIL == CAN.checkReceive()) {
          CAN.readMsgBuf(&ReceivedChargerMessageLength, ReceivedChargerMessage);
          chargerCount = chargerCount + 1;

          volts = volts + word(ReceivedChargerMessage[VoltageHighByte], ReceivedChargerMessage[VoltageLowByte]);

          //this could cause wattage to read high
          //if some chargers were on 120vac and others on 240vac
          amps = amps + word(ReceivedChargerMessage[CurrentHighByte], ReceivedChargerMessage[CurrentLowByte]);
      }

      if (chargerCount > 1) {
        //get average voltage
        volts = volts / chargerCount;
      }

      watts = (amps * volts);

      if(watts<lastWatts){
        watts=lastWatts;
      }

      stationWatts = watts + MAX_12V_WATTS;

      //if we are not running at full tilt
      //or if we are not yet started this will be a slow ramp
      if(stationWatts<MAX_STATION_WATTS){
        watts=watts+RAMP_RATE;
      }

      //if we want more than the station can deliver
      if (stationWatts > MAX_STATION_WATTS) {
        watts = MAX_STATION_WATTS - MAX_12V_WATTS;
      }

      //stop the charger for this cycle if the max voltage reached
      if (volts > OUTPUT_VOLTS_MAX) {
        watts=0;
      }

      //stop the charger for this cycle if the max voltage reached
      if (volts < OUTPUT_VOLTS_MIN) {
        // Set current to zero:
        watts=0;
      }

      //set this cycles charging amps
      if (watts > 0) {
        setChargingAmps();
      }

      HeartbeatMessage[CurrentHighByte] = highByte(chargingAmps);
      HeartbeatMessage[CurrentLowByte] = lowByte(chargingAmps);
  }

  if (Serial) {
    //battery volts
    Serial.print(volts);
    Serial.print(",");

    //total charging amps
    Serial.print(amps);
    Serial.print(",");

    //total charging amps per charger
    Serial.print(amps/chargerCount);
    Serial.print(",");

    //charger count
    Serial.print(chargerCount);
    Serial.print(",");

    //target watts
    Serial.print(watts);
    Serial.print(",");

    //target max watts
    Serial.print(MAX_STATION_WATTS);
    Serial.print(",");

    //target total amps
    Serial.print(chargingAmps*chargerCount);
    Serial.print(",");

    //target amps per charger
    Serial.print(chargingAmps);

    //start extending with a comma
    //Serial.print(",");

    Serial.println("");
  }

  // frame status = extended in second arg
  CAN.sendMsgBuf(HeartbeatMessageID, 1, HeartbeatMessageLength, HeartbeatMessage);
  delay(1000);
}
