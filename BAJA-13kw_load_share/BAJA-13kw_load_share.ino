#include <EEPROMex.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SoftwareSerial.h>

#include <SPI.h>

#define SPI_CS_PIN 9
#define btRX A0
#define btTX A1

// set up a new serial object
SoftwareSerial btSerial (btRX, btTX);

#define VERSION 2
#define BAUD_RATE 38400
#define HeartbeatMessageID 0x1806E5F4
#define HeartbeatMessageLength 8

#define VoltageHighByte 0
#define VoltageLowByte 1
#define CurrentHighByte 2
#define CurrentLowByte 3

#define TargetWattsAddress 0

//over 14000.00 is not a possible option with this code
//if a value over 14kw is stored it will default back to
//the STATION_WATTS_DEFAULT value
#define WATTAGE_MAX_UNSET 1400000

//6600.00
#define STATION_WATTS_DEFAULT 660000
//76.0 watts
#define MAX_12V_WATTS 7600

//116.4 volts
#define OUTPUT_VOLTS_MAX 1263
//50.0 volts
#define OUTPUT_VOLTS_MIN 700
//32.0 amps
#define OUTPUT_AMPS_MAX 320

//50 watts
#define RAMP_RATE 50000

//number of cutback points
#define CUTBACK_COUNT 8
//***Lifted*** voltage*10 cutback
const unsigned short OUTPUT_VOLTS_CUTBACK[]={1000,
  1140,
  1210,
  1230,
  1240,
  1250,
  1255,
  1250};

//max wattage*100 at ***Lifted*** voltage above
const unsigned long OUTPUT_WATTS_CUTBACK[]={2200000,
  1600000,
  1400000,
  1300000,
  1000000,
  660000,
  500000,
  400000};

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
  //default 126.3 not overwritten anywhere,
  //this could be dynamic
  0x04, 0xef,
  // Current, high then low byte in amps
  //default 32.0 overwritten immediately unless no can messages recieved
  //this can happen on some older units when on 120v
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

void readCommands(){
  while(btSerial.available()){
    String commandWatts = btSerial.readStringUntil("\n");
    unsigned long targetWattage = commandWatts.toInt();
    if(targetWattage>=0){
      MAX_STATION_WATTS=targetWattage;
      EEPROM.writeLong(TargetWattsAddress, MAX_STATION_WATTS);
    }

    if(targetWattage<0){
      MAX_STATION_WATTS=0;
    }
  }
}

void setup() {
  MAX_STATION_WATTS=EEPROM.readLong(TargetWattsAddress);

  if(MAX_STATION_WATTS>WATTAGE_MAX_UNSET){
    MAX_STATION_WATTS=STATION_WATTS_DEFAULT;
  }

  if (btSerial) {
      // define pin modes for tx, rx:
      pinMode(btRX, INPUT);
      pinMode(btTX, OUTPUT);
      // set the data rate for the SoftwareSerial port
      btSerial.begin(BAUD_RATE);
  }

  // initialize the CAN bus at baud rate 250kbps
  while (CAN_OK != CAN.begin(CAN_250KBPS)) {
    delay(100);
  }

  attachInterrupt(0, MCP2515_ISR, FALLING);
}

void loop() {

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

      //check for cutback requirement scenarios.
      for (int i = 0; i < CUTBACK_COUNT; i++) {
         if (volts > OUTPUT_VOLTS_CUTBACK[i] && watts > OUTPUT_WATTS_CUTBACK[i]) {
             MAX_STATION_WATTS=OUTPUT_WATTS_CUTBACK[i];
             watts=MAX_STATION_WATTS;
         }
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

  if (btSerial) {
    //battery volts
    btSerial.print(volts);
    btSerial.print(",");

    //total charging amps
    btSerial.print(amps);
    btSerial.print(",");

    //charger count
    btSerial.print(chargerCount);
    btSerial.print(",");

    //target watts
    btSerial.print(watts/100);

    //start extending with a comma
    btSerial.print(",");

    //print api version
    btSerial.println(VERSION);
  }

  // frame status = extended in second arg
  CAN.sendMsgBuf(HeartbeatMessageID, 1, HeartbeatMessageLength, HeartbeatMessage);
  delay(1000);
}
