#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include <SPI.h>

const int SPI_CS_PIN = 9;
const unsigned short STATION_CURRENT_GOOD = 31; //calcs use a short so decimal is cut off. so 6.4kw is 30 and 6.6kW is 31
const unsigned short STATION_CURRENT_BAD = 26; //calcs use a short so decimal is cut off. so 5.6kw is 26
const unsigned short OUTPUT_VOLTAGE_MAX = 116; // 28 Farasis cells in series at 100% SoC.
const unsigned short OUTPUT_CURRENT_MAX = 94; // 1C-rate minus 12A for the onboard charger.

const unsigned short STATION_VOLTAGE = 208; // Lowest acceptable station voltage as an assumption.
const unsigned short MAX_CHARGERS_PER_STATION = 2;

unsigned short MAX_JAMPS = 0;
int shitPin = 7;

MCP_CAN CAN(SPI_CS_PIN);

unsigned char stmp[8] = {
  //volts
  0x04, 0x8c,
  //amps
  0x01, 0x40,
  //reserved
  0x00, 0x00, 0x00, 0x00
};

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup() {
  if (Serial) {
    Serial.begin(115200);
  }

  pinMode(shitPin, INPUT_PULLUP);
  //digitalWrite(shitPin, LOW);

  while (CAN_OK != CAN.begin(CAN_250KBPS)) {
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

      int chargerCount = 0;
      unsigned short volts = 0;
      unsigned short amps = 0;
      unsigned long watts = 0;

//      if (Serial) {
//        Serial.println("-----------------------------");
//      }

      while (CAN_MSGAVAIL == CAN.checkReceive()) {
          CAN.readMsgBuf(&len, buf);
          chargerCount+=1;

          unsigned short chargerVolts = word(buf[0],buf[1]) / 10;
          if (chargerVolts > 0) {
            volts = volts + chargerVolts;
            if (chargerCount > 1) {
              volts = volts / MAX_CHARGERS_PER_STATION;
            }
          }
          amps+=word(buf[2],buf[3]) / 10;

//          if (Serial) {
//            Serial.print(chargerCount);
//            Serial.print(" : volts - ");
//            Serial.print(word(buf[0],buf[1]));
//            Serial.print(", amps - ");
//            Serial.print(word(buf[2],buf[3]));
//            Serial.println();
//          }
      }

      watts = (amps * volts);

      unsigned short jAmps = watts / STATION_VOLTAGE;

      if (chargerCount > MAX_CHARGERS_PER_STATION) {
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
        stmp[2] = highByte(chargingAmps * 10);
        stmp[3] = lowByte(chargingAmps * 10);
      }

      if (volts > OUTPUT_VOLTAGE_MAX) {
        if (Serial) {
          Serial.println("Voltage Max exceeded, stopping charge");
        }
        stmp[2] = highByte(0);
        stmp[3] = lowByte(0);
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
        Serial.print(word(stmp[2],stmp[3]) / 10);
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

  CAN.sendMsgBuf(0x1806E5F4, 1, 8, stmp);
  delay(1000);
}
