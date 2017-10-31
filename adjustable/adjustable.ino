#include <mcp_can.h>
#include <SPI.h>
#include <TaskScheduler.h>

const long SERIAL_BAUD_RATE        = 9600;
const unsigned short SPI_CS_PIN    = 9; // 10 for Elecfreaks.com board
const unsigned char CAN_MSG_LENGTH = 8;
const unsigned long CAN_MSG_ID     = 0x1806E5F4;
MCP_CAN CAN(SPI_CS_PIN);

const unsigned short SAMPLE_READ_CAN         = 20;
const unsigned short SAMPLE_READ_SERIAL      = 500;
const unsigned short SAMPLE_SEND_CAN         = 1000;
const unsigned short SAMPLE_PRINT_DEBUG      = 1000;
const unsigned short SAMPLE_COUNT_CHARGERS   = 5000;
const unsigned short SAMPLE_RAMP_SLOW        = 2500;
const unsigned short SAMPLE_RAMP_FAST        = 1875;

const unsigned short RAMP_SLOW_ITERATIONS    = 12;
const unsigned short RAMP_FAST_ITERATIONS    = 16;

const float          OUTPUT_VOLTAGE_MAX = 116.4;
const unsigned short OUTPUT_CURRENT_MAX = 104; // 1C, battery capacity
const unsigned short MAX_CHARGER_OUTPUT = 35;  // probably less than this

// 1650   =  15A @ 110V
// 3300W  =  30A @ 110V
// 5800W  =  28A @ 208V
// 6160W  =  28A @ 220V
// 6600W  =  31A @ 208V
// 6820W  =  31A @ 220V
// 7200W  =  30A @ 240V
// 7400W  =  35A @ 208V
// 7700W  =  35A @ 220V
// 12000W =  50A @ 240V
// 19200W =  80A @ 240V
// 24000W = 100A @ 240V

unsigned short stationAmps  = 0;
unsigned short stationVolts = 110;
unsigned short maxChargersPerStation = 2;
unsigned short stationCapacity = stationAmps * stationVolts;
unsigned short ampsPerCharger  = 0;

const char *MESSAGE_START      = '<';
const char *MESSAGE_END        = '>';
const char *MESSAGE_DELIMITER  = ":";
const unsigned short MAX_CHARACTERS = 32;

boolean manualMode = false;

unsigned char canMsg[CAN_MSG_LENGTH] = {
  0x04, 0x8c,            // Voltage
  0x00, 0x0a,            // Current, 0x01 0x40 = 32
  0x00, 0x00, 0x00, 0x00
};

struct canMessage {
  float volts;
  unsigned short amps;
};

struct total {
  float volts = 0;
  unsigned short chargers = 0;
} total;

struct counters {
  float chargers = 0;
  float volts    = 0;
} counters;

void readCAN();
void sendCAN();
void readSerial();
void printDebug();
void countChargers();
void rampSlow();
void rampFast();

Task tReadCAN(SAMPLE_READ_CAN, TASK_FOREVER, &readCAN);
Task tSendCAN(SAMPLE_SEND_CAN, TASK_FOREVER, &sendCAN);
Task tReadSerial(SAMPLE_READ_SERIAL, TASK_FOREVER, &readSerial);
Task tPrintDebug(SAMPLE_PRINT_DEBUG, TASK_FOREVER, &printDebug);
Task tCountChargers(SAMPLE_COUNT_CHARGERS, TASK_FOREVER, &countChargers);
Task tRampSlow(SAMPLE_RAMP_SLOW, RAMP_SLOW_ITERATIONS, &rampSlow);
Task tRampFast(SAMPLE_RAMP_FAST, RAMP_FAST_ITERATIONS, &rampFast);

Scheduler runner;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  while (CAN_OK != CAN.begin(CAN_250KBPS)) {
    delay(100);
  }

  runner.init();

  runner.addTask(tReadCAN);
  runner.addTask(tSendCAN);
  runner.addTask(tPrintDebug);
  runner.addTask(tCountChargers);
  runner.addTask(tReadSerial);
  runner.addTask(tRampSlow);
  runner.addTask(tRampFast);

  tReadCAN.enable();
  tSendCAN.enable();
  tPrintDebug.enable();
  tCountChargers.enable();
  tReadSerial.enable();
  tRampSlow.enable();
}

canMessage readChargerData() {
  unsigned char canMsg[CAN_MSG_LENGTH];
  CAN.readMsgBuf(&CAN_MSG_LENGTH, canMsg);

  float volts = word(canMsg[0], canMsg[1]) / 10;
  unsigned short amps = word(canMsg[2], canMsg[3]) / 10;

  struct canMessage data = { .volts = volts, .amps = amps };
  return data;
}

void rampSlow() {
  if (manualMode) {
    tRampSlow.disable();
    tRampFast.disable();
  } else {
    stationAmps += 1;
    if (tRampSlow.isLastIteration()) {
      stationVolts = 208;
      tRampFast.enable();
    }
  }
}

void rampFast() {
  if (manualMode) {
    tRampSlow.disable();
    tRampFast.disable();
  } else {
    stationAmps += 1;
  }
}

void readCAN() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    // Serial.println("<= CAN");

    noInterrupts();
    canMessage chargerData = readChargerData();
    counters.chargers += 1;
    counters.volts += chargerData.volts;
    interrupts();
  }
}

void readSerial() {
  static boolean inProgress = false;
  static byte index = 0;
  char receivedByte;
  boolean serialFlagReceived = false;
  char serialMsg[MAX_CHARACTERS];
  char serialMsgBuffer[MAX_CHARACTERS];
  char serialMsgVersion[MAX_CHARACTERS] = {0};

  while (Serial.available() > 0 && serialFlagReceived == false) {
    receivedByte = Serial.read();

    if (inProgress == true) {
      if (receivedByte != MESSAGE_END) {
        serialMsg[index] = receivedByte;
        index++;
        if (index >= MAX_CHARACTERS) {
          index = MAX_CHARACTERS - 1;
        }
      }
      else {
        serialMsg[index] = '\0';
        inProgress = false;
        index = 0;
        serialFlagReceived = true;
      }
    }

    else if (receivedByte == MESSAGE_START) {
      inProgress = true;
    }
  }

  if (serialFlagReceived) {
    serialFlagReceived = false;
    strcpy(serialMsgBuffer, serialMsg);

    char * index;

    index = strtok(serialMsgBuffer, MESSAGE_DELIMITER);
    strcpy(serialMsgVersion, index);

    // <v2:maxChargersPerStation:stationAmps:stationVolts>
    if (strcmp(serialMsgVersion, "v2") == 0) {
      index = strtok(NULL, MESSAGE_DELIMITER);
      maxChargersPerStation = atoi(index);

      index = strtok(NULL, MESSAGE_DELIMITER);
      stationAmps = atoi(index);

      index = strtok(NULL, MESSAGE_DELIMITER);
      stationVolts = atoi(index);

      manualMode = true;
    }

    // <CAN:1:105>
    if (strcmp(serialMsgVersion, "CAN") == 0) {
      index = strtok(NULL, MESSAGE_DELIMITER);
      total.chargers = atoi(index);

      index = strtok(NULL, MESSAGE_DELIMITER);
      total.volts = atoi(index);
    }
  }
}

void setChargerCurrent(unsigned short amps) {
  canMsg[2] = highByte(amps * 10);
  canMsg[3] = lowByte(amps * 10);

  CAN.sendMsgBuf(CAN_MSG_ID, 1, CAN_MSG_LENGTH, canMsg);
}

void sendCAN() {
  if (total.chargers > 0) {
    if (total.volts > OUTPUT_VOLTAGE_MAX) {
      setChargerCurrent(0);
    } else {
      int chargersPerStation = min(total.chargers, maxChargersPerStation);
      stationCapacity = stationAmps * stationVolts;
      unsigned short ampsToCharge = stationCapacity / total.volts;
      ampsPerCharger = ampsToCharge / chargersPerStation;

      ampsPerCharger = min(ampsPerCharger, MAX_CHARGER_OUTPUT);
      ampsPerCharger = min(ampsPerCharger, OUTPUT_CURRENT_MAX / total.chargers);

      setChargerCurrent(ampsPerCharger);

      Serial.print("<v2:");
      Serial.print(total.chargers);
      Serial.print(":");
      Serial.print(ampsPerCharger);
      Serial.print(":");
      Serial.print(total.volts);
      Serial.println(">");
    }
  }
}

void countChargers() {
  noInterrupts();
  total.chargers = round(counters.chargers / (SAMPLE_COUNT_CHARGERS / 1000));
  if (counters.chargers > 0) {
    total.volts = round(counters.volts / counters.chargers);
  }

  counters.chargers = 0;
  counters.volts = 0;
  interrupts();
}

void printDebug() {
  Serial.print("Chargers: ");
  Serial.print(total.chargers);
  Serial.print(", ");
  Serial.print(total.volts);
  Serial.print("V, Station capacity: ");
  Serial.print(stationAmps);
  Serial.print("A @ ");
  Serial.print(stationVolts);
  Serial.print("V (");
  Serial.print(stationAmps * stationVolts);
  Serial.print("W), ");
  Serial.print(ampsPerCharger);
  Serial.println("A per charger");
}

void loop() {
  runner.execute();
}
