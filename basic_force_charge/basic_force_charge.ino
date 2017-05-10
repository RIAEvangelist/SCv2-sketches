#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include <SPI.h>

const int SPI_CS_PIN = 9;
const unsigned short STATION_CURRENT_GOOD = 31; //calcs use a short so decimal is cut off. so 6.4kw is 30 and 6.6kW is 31
const unsigned short STATION_CURRENT_BAD = 26; //calcs use a short so decimal is cut off. so 5.6kw is 26
const unsigned short OUTPUT_VOLTAGE_MAX = 116; // 28 Farasis cells in series at 100% SoC.
const unsigned short OUTPUT_CURRENT_MAX = 94; // 1C-rate minus 12A for the onboard charger.

const unsigned short STATION_VOLTAGE = 208; // Lowest acceptable station voltage as an assumption.
const unsigned short MAX_CHARGERS_PER_STATION = 2; // = 6.6kW / 3.3kW per charger

#define BAUD_RATE 115200

unsigned short MAX_JAMPS = 0;
int shitPin = 7;

MCP_CAN CAN(SPI_CS_PIN);

// From http://www.elconchargers.com/frequentlyasked_questions.html#161971 for Elcon CAN bus messaging:

// The charger expects to receive every second a message from the BMS with CAN ID 1806E5F4 and 8-byte data with the voltage and current requested.
// If the charger doesn't receive a valid CAN message in 5 seconds, it stops charging until it receives a valid CAN message.
// NOTE: DigiNow could use more Elcon IDs to control chargers separately, but prefers to control them in tandem.
#define HeartbeatMessageID 0x1806E5F4
#define HeartbeatMessageLength 8 // bytes in HeartbeatMessage
unsigned char HeartbeatMessage[HeartbeatMessageLength] = {
  // Voltage, high then low byte in volts
  0x04, 0x8c,
  // Current, high then low byte in amps
  0x01, 0x40,
  // Reserved/status
  0x00, 0x00, 0x00, 0x00
};

#define VoltageHighByte 0
#define VoltageLowByte 1
#define CurrentHighByte 2
#define CurrentLowByte 3

// The charger sends out every second a CAN status message with voltage, current and status information.
unsigned char flagRecv = 0;
unsigned char ReceivedChargerMessageLength = 0;
unsigned char ReceivedChargerMessage[8];
char str[20];

void setup() {
  if (Serial) {
    Serial.begin(BAUD_RATE);
  }

  pinMode(shitPin, INPUT_PULLUP);
  //digitalWrite(shitPin, LOW);

  while (CAN_OK != CAN.begin(CAN_250KBPS)) { // initialize the CAN bus at baud rate 250kbps
    delay(100);
  }

  if (Serial) {
    Serial.println("CAN BUS Shield init ok!");
  }

  attachInterrupt(0, MCP2515_ISR, FALLING);
}

void MCP2515_ISR() {
    flagRecv = 1;
//    if (Serial) {
//      Serial.println("got can");
//    }
}

void loop() {
  unsigned short chargingAmps = 0;

  if (digitalRead(shitPin)) {
//    if (Serial && MAX_JAMPS > STATION_CURRENT_BAD) {
//      Serial.println("Treating as Shit Station Max watts : ");
//    }
    MAX_JAMPS = STATION_CURRENT_BAD;
    if (MAX_JAMPS < STATION_CURRENT_BAD) {
      chargingAmps = ((MAX_JAMPS * STATION_VOLTAGE) / 90) / MAX_CHARGERS_PER_STATION;
    }
//    if (Serial) {
//      Serial.println(MAX_JAMPS * STATION_VOLTAGE);
//    }
  } else {
//    if (Serial && MAX_JAMPS < STATION_CURRENT_GOOD) {
//      Serial.println("reating as good Station Max watts : ");
//    }
    MAX_JAMPS = STATION_CURRENT_GOOD;
    if (MAX_JAMPS < STATION_CURRENT_GOOD) {
      chargingAmps = ((MAX_JAMPS * STATION_VOLTAGE) / 90) / MAX_CHARGERS_PER_STATION;
    }
//    if (Serial) {
//      Serial.println(MAX_JAMPS * STATION_VOLTAGE);
//    }
  }



  if (flagRecv) {
      flagRecv = 0;

      int chargerCount = 0; // How many messages have we received from chargers in this loop represents how many chargers are connected/live.
      unsigned short volts = 0;
      unsigned short amps = 0;
      unsigned long watts = 0;

//      if (Serial) {
//        Serial.println("-----------------------------");
//      }

      while (CAN_MSGAVAIL == CAN.checkReceive()) {
          CAN.readMsgBuf(&ReceivedChargerMessageLength, ReceivedChargerMessage);
          chargerCount += 1;

          unsigned short chargerVolts = word(ReceivedChargerMessage[VoltageHighByte], ReceivedChargerMessage[VoltageLowByte]) / 10;
          if (chargerVolts > 0) {
            volts = volts + chargerVolts;
            if (chargerCount > 1) {
              // We might divide by chargerCount here instead, to make an average voltage reading:
              volts = volts / MAX_CHARGERS_PER_STATION;
            }
          }
          amps += word(ReceivedChargerMessage[CurrentHighByte], ReceivedChargerMessage[CurrentLowByte]) / 10;

//          if (Serial) {
//            Serial.print(chargerCount);
//            Serial.print(" : volts - ");
//            Serial.print(word(ReceivedChargerMessage[VoltageHighByte], ReceivedChargerMessage[VoltageLowByte]));
//            Serial.print(", amps - ");
//            Serial.print(word(ReceivedChargerMessage[CurrentHighByte], ReceivedChargerMessage[CurrentLowByte]));
//            Serial.println();
//          }
      }

      watts = (amps * volts);

      unsigned short jAmps = watts / STATION_VOLTAGE;

      if (chargerCount > MAX_CHARGERS_PER_STATION) {
        // We divide by 3 but it might be chargerCount instead:
        jAmps = ((watts / 3) * MAX_CHARGERS_PER_STATION) / STATION_VOLTAGE;
      }

      if (amps > OUTPUT_CURRENT_MAX) {
        chargingAmps = OUTPUT_CURRENT_MAX / chargerCount;
      }

      if (jAmps > MAX_JAMPS) {
        unsigned short chargingAmpsJ = ((MAX_JAMPS * STATION_VOLTAGE) / volts) / MAX_CHARGERS_PER_STATION;
        if (chargingAmpsJ < chargingAmps) {
          chargingAmps = chargingAmpsJ;
        }
      }

      if (chargingAmps > 0) {
        HeartbeatMessage[CurrentHighByte] = highByte(chargingAmps * 10);
        HeartbeatMessage[CurrentLowByte] = lowByte(chargingAmps * 10);
      }

      if (volts > OUTPUT_VOLTAGE_MAX) {
        if (Serial) {
          Serial.println("Voltage Max exceeded, stopping charge");
        }
        // Set current to zero:
        HeartbeatMessage[CurrentHighByte] = highByte(0);
        HeartbeatMessage[CurrentLowByte] = lowByte(0);
      }


      if (Serial) {
//
//        Serial.print(chargerCount);
//        Serial.print(" chargers : ");
//
        Serial.print(volts);
        Serial.print("v, ");
//
        Serial.print(amps);
        Serial.print("A/");
        Serial.print(word(HeartbeatMessage[CurrentHighByte], HeartbeatMessage[CurrentLowByte]) / 10);
        Serial.print(" ");
//        Serial.print(watts);
//        Serial.print("W, ");
//
        Serial.print(jAmps);
        Serial.print(" jAmps of ");
        Serial.print(MAX_JAMPS);

        Serial.println();
      }
  }

  // frame status = extended in second arg
  CAN.sendMsgBuf(HeartbeatMessageID, 1, HeartbeatMessageLength, HeartbeatMessage);
  delay(1000);
}
