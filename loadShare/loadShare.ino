#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include <SPI.h>

#define BAUD_RATE 115200
#define HeartbeatMessageID 0x1806E5F4
#define HeartbeatMessageLength 8

#define VoltageHighByte 0
#define VoltageLowByte 1
#define CurrentHighByte 2
#define CurrentLowByte 3

const int SPI_CS_PIN = 9;
int lowPowerStation = 7;

//6600.0kw
const unsigned long STATION_WATTS_GOOD = 660000;
//5000.0 kw
const unsigned long STATION_WATTS_BAD = 500000;

//76.0 watts
const unsigned short MAX_12V_WATTS = 7600;

//116.4 volts
const unsigned short OUTPUT_VOLTS_MAX = 1164;
//50.0 volts
const unsigned short OUTPUT_VOLTS_MIN = 500;
//32.0 amps
const unsigned short OUTPUT_AMPS_MAX =  320;

//50 watts
const unsigned short RAMP_RATE = 500;

//target wattage
unsigned short MAX_STATION_WATTS = 0;
//target amps to charge calculated by code
//uses MAX_STATION_WATTS and volts
unsigned short chargingAmps = 0;

bool verbose=true;

int chargerCount = 0;
//current voltage
unsigned short volts = 0;
//charging amps
unsigned short amps = 0;
//charging watts
unsigned long watts = 0;
//estimated watts being pulled from station
unsigned short stationWatts =0;

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
    if (Serial && verbose) {
      Serial.println("got can");
    }
}

void setChargingAmps(){
  chargingAmps = watts / volts / chargerCount;
}

void setUpStationLimits(){
  //default to good station if max not yet set
  if(MAX_STATION_WATTS==0){
    MAX_STATION_WATTS = STATION_WATTS_GOOD;
  }

  //if low Power station switch on and wattage not modified
  if (digitalRead(lowPowerStation) && MAX_STATION_WATTS == STATION_WATTS_GOOD) {
    if (MAX_STATION_WATTS != STATION_WATTS_BAD) {
      MAX_STATION_WATTS = STATION_WATTS_BAD;
    }
  }

  //if NOT low Power station switch on and wattage not modified
  if (!digitalRead(lowPowerStation) && MAX_STATION_WATTS == STATION_WATTS_BAD) {
    if (MAX_STATION_WATTS != STATION_WATTS_GOOD) {
      MAX_STATION_WATTS = STATION_WATTS_GOOD;
    }
  }
}

void setup() {
  if (Serial && verbose) {
    Serial.begin(BAUD_RATE);
  }

  pinMode(lowPowerStation, INPUT_PULLUP);
  //digitalWrite(lowPowerStation, LOW);

  // initialize the CAN bus at baud rate 250kbps
  while (CAN_OK != CAN.begin(CAN_250KBPS)) {
    delay(100);
  }

  attachInterrupt(0, MCP2515_ISR, FALLING);
}

void loop() {

  setUpStationLimits();

  if (flagRecv) {
      flagRecv = 0;

       // How many messages have we received from chargers in this loop
      // represents how many chargers are connected/live.
      chargerCount = 0;
      volts = 0;
      amps = 0;
      watts = 0;

      while (CAN_MSGAVAIL == CAN.checkReceive()) {
          CAN.readMsgBuf(&ReceivedChargerMessageLength, ReceivedChargerMessage);
          chargerCount += 1;

          unsigned short chargerVolts = word(ReceivedChargerMessage[VoltageHighByte], ReceivedChargerMessage[VoltageLowByte]) / 10;
          if (chargerVolts > 0) {
            volts = volts + chargerVolts;
          }

          //this could get weird if some chargers were on 120vac and
          //others on 240vac
          amps += word(ReceivedChargerMessage[CurrentHighByte], ReceivedChargerMessage[CurrentLowByte]);
      }

      if (chargerCount > 1) {
        //get average voltage
        volts = volts / chargerCount;
      }

      watts = (amps * volts);

      stationWatts = watts + MAX_12V_WATTS;

      //if we are not running at full tilt
      //or if we are not yet started this will be a slow ramp
      if(stationWatts<MAX_STATION_WATTS){
        watts=watts+RAMP_RATE;
      }

      if (Serial && verbose) {
        Serial.print("reported watts : ");
        Serial.println(watts);

        Serial.print("stationWatts : ");
        Serial.println(stationWatts);

        Serial.print("target station watts : ");
        Serial.println(MAX_STATION_WATTS);
      }

      //if we want more than the station can deliver
      if (stationWatts > MAX_STATION_WATTS) {
        watts = (MAX_STATION_WATTS / volts) - MAX_12V_WATTS;
      }

      //set this cycles charging amps
      if (watts > 0) {
        setChargingAmps();
      }

      //stop the charger for this cycle if the max voltage reached
      if (volts > OUTPUT_VOLTS_MAX) {
        chargingAmps=0;
      }

      //stop the charger for this cycle if the max voltage reached
      if (volts < OUTPUT_VOLTS_MIN) {
        // Set current to zero:
        chargingAmps=0;
      }

      HeartbeatMessage[CurrentHighByte] = highByte(chargingAmps);
      HeartbeatMessage[CurrentLowByte] = lowByte(chargingAmps);
  }

  if (Serial && verbose) {
    Serial.print("volts : ");
    Serial.println(volts);

    Serial.print("chargerCount : ");
    Serial.println(chargerCount);

    Serial.print("amps reported : ");
    Serial.println(amps);

    Serial.print("target charging watts : ");
    Serial.println(watts);

    Serial.print("target chargingAmps : ");
    Serial.println(chargingAmps);
  }

  // frame status = extended in second arg
  CAN.sendMsgBuf(HeartbeatMessageID, 1, HeartbeatMessageLength, HeartbeatMessage);
  delay(1000);
}
