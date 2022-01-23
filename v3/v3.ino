#define VERSION 4
#define BAUD_RATE 38400
#define CHARGER_PIN 3
#define BIKE_PIN 2
//BLE ADVERTISED NAME
#define BLEName "SC v3"

#include <Network.h>
#include <unions.h>
#include <ZERO.h>

#include <mcp_can.h>
#include <mcp_can_dfs.h>

MCP_CAN Bike(BIKE_PIN);
Zero zero;

#include <VOLTAGE_TABLE_ZERO.h>
#include <SC.h>

#include "ZERO_BIKE.h"
#include "ZERO_V3_BLE.h"
#include "debug.h"

void setup() {
  OUTPUT_VOLTS_END=1320;

  debugSCSetup();
  superChargerSetup();
  bleSetup();
  bikeSetup();
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  bikeLoop();
  if(delayItteration>0){
    delayItteration--;
  }

  if(delayItteration<1){
    delayItteration=(DelayItteration/2);

    superChargerLoop();


    byte chargingBuf[8];

    chargingBuf[0]=0x1;
    Bike.sendMsgBuf(CCU_CHARGE_STATUS.id, 0, zero.messageLength, chargingBuf);
    Bike.sendMsgBuf(CALEXMX_CHARGE_STATUS.id, 0, zero.messageLength, chargingBuf);
    Bike.sendMsgBuf(CALEX_CHARGE_STATUS.id, 0, zero.messageLength, chargingBuf);

    BLE.poll();

    superCharge();

    debugSCLog();
  }
}
