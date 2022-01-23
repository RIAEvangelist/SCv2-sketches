#ifndef ZERO_V3_BLE_SETUP_H
#define ZERO_V3_BLE_SETUP_H

#include <ArduinoBLE.h>

//ba77 = BATT
BLEService batteryService("ba770000-0000-0000-0000-000000000000");
BLELongCharacteristic battery1Volts("ba770001-1010-0000-0000-000000000000",BLERead | BLENotify);
BLELongCharacteristic battery2Volts("ba770002-1010-0000-0000-000000000000",BLERead | BLENotify);
BLEShortCharacteristic battery1Amps("ba770003-1010-0000-0000-000000000000",BLERead | BLENotify);
BLEShortCharacteristic battery2Amps("ba770004-1010-0000-0000-000000000000",BLERead | BLENotify);
BLEShortCharacteristic battery1Temp1("ba770005-1010-0000-0000-000000000000",BLERead | BLENotify);
BLEShortCharacteristic battery1Temp2("ba770006-1010-0000-0000-000000000000",BLERead | BLENotify);
BLEShortCharacteristic battery2Temp1("ba770007-1010-0000-0000-000000000000",BLERead | BLENotify);
BLEShortCharacteristic battery2Temp2("ba770008-1010-0000-0000-000000000000",BLERead | BLENotify);
BLEShortCharacteristic battery1AH("ba770009-1010-0000-0000-000000000000",BLERead | BLENotify);
BLEShortCharacteristic battery2AH("ba770010-1010-0000-0000-000000000000",BLERead | BLENotify);
BLEShortCharacteristic batteryMaxVolts("ba779999-1110-0000-0000-000000000000",BLERead | BLEWrite | BLENotify);

//d161 = DIGI
BLEService chargerService("d1610000-0000-0000-0000-000000000000");
BLEUnsignedLongCharacteristic chargerTargetVolts("d1610001-1110-0000-0000-000000000000",BLERead | BLEWrite | BLENotify);
BLEUnsignedShortCharacteristic chargerTargetWatts("d1610002-1110-0000-0000-000000000000",BLERead | BLENotify);
BLEUnsignedLongCharacteristic chargerMaxWatts("d1610003-1110-0000-0000-000000000000",BLERead | BLEWrite | BLENotify);
BLEIntCharacteristic chargerCountBLE("d1610004-1110-0000-0000-000000000000",BLERead | BLEWrite | BLENotify);

//ca11ed = CALLED
BLEService bleService("ca11ed00-0000-0000-0000-000000000000");
BLECharCharacteristic bleName("ca11ed01-1110-0000-0000-000000000000",BLERead |  BLENotify);
BLEByteCharacteristic bleUnits("ca11ed02-1110-0000-0000-000000000000",BLERead | BLEWrite | BLENotify);

//dc2ac = DC to AC ... charger
BLEService controllerService("dc2ac000-0000-0000-0000-000000000000");

//da5 = DASH
BLEService dashService("da500000-0000-0000-0000-000000000000");


void userUpdate(BLEDevice central, BLECharacteristic characteristic){
  Serial.println("Got update");

  Serial.println(OUTPUT_VOLTS_END);
  Serial.println(MAX_STATION_WATTS);

  OUTPUT_VOLTS_END=chargerTargetVolts.value();
  MAX_STATION_WATTS=chargerMaxWatts.value();
  chargerCount=chargerCountBLE.value();

  Serial.println(chargerTargetVolts.value());
  Serial.println(OUTPUT_VOLTS_END);

  Serial.println(chargerMaxWatts.value());
  Serial.println(MAX_STATION_WATTS);

  // handle units in a var
  // bleUnits.value();
}

void bleSetup(){
  // begin initialization
  if (!BLE.begin()) {
    while (1);
  }

  // set advertised local name and services :
  BLE.setLocalName(BLEName);
  BLE.setAdvertisedService(batteryService);
  BLE.setAdvertisedService(chargerService);
  BLE.setAdvertisedService(bleService);
  BLE.setAdvertisedService(controllerService);
  BLE.setAdvertisedService(dashService);

  // add the characteristics to services
  batteryService.addCharacteristic(battery1Volts);
  batteryService.addCharacteristic(battery2Volts);
  batteryService.addCharacteristic(battery1Amps);
  batteryService.addCharacteristic(battery2Amps);
  batteryService.addCharacteristic(battery1Temp1);
  batteryService.addCharacteristic(battery1Temp2);
  batteryService.addCharacteristic(battery2Temp1);
  batteryService.addCharacteristic(battery2Temp2);
  batteryService.addCharacteristic(battery1AH);
  batteryService.addCharacteristic(battery2AH);
  batteryService.addCharacteristic(batteryMaxVolts);

  chargerService.addCharacteristic(chargerTargetVolts);
  chargerService.addCharacteristic(chargerTargetWatts);
  chargerService.addCharacteristic(chargerMaxWatts);
  chargerService.addCharacteristic(chargerCountBLE);

  chargerTargetVolts.setEventHandler(BLEWritten, userUpdate);
  chargerMaxWatts.setEventHandler(BLEWritten, userUpdate);

  bleService.addCharacteristic(bleUnits);

  bleUnits.setEventHandler(BLEWritten, userUpdate);

  // add services
  BLE.addService(batteryService);
  BLE.addService(chargerService);
  BLE.addService(bleService);

  // set the initial values for the characeristics:
  battery1Volts.setValue(monolithVolts.value);
  battery2Volts.setValue(powerTankVolts.value);
  battery1Amps.setValue(monolithAmps.value);
  battery2Amps.setValue(powerTankAmps.value);
  battery1Temp1.setValue(monolithMinTemp.value);
  battery1Temp2.setValue(monolithMaxTemp.value);
  battery2Temp1.setValue(powerTankMinTemp.value);
  battery2Temp2.setValue(powerTankMaxTemp.value);
  battery1AH.setValue(monolithAH.value);
  battery2AH.setValue(powerTankAH.value);
  batteryMaxVolts.setValue(OUTPUT_VOLTS_MAX);

  chargerTargetVolts.setValue(OUTPUT_VOLTS_END);
  chargerTargetWatts.setValue(watts);
  chargerMaxWatts.setValue(MAX_STATION_WATTS);
  chargerCountBLE.setValue(chargerCount);

  bleUnits.setValue(0);

  // start advertising
  BLE.advertise();
}

void bleLoop(){
  battery1Volts.setValue(monolithVolts.value);
  battery2Volts.setValue(powerTankVolts.value);
  battery1Amps.setValue(monolithAmps.value);
  battery2Amps.setValue(powerTankAmps.value);
  battery1Temp1.setValue(monolithMinTemp.value);
  battery1Temp2.setValue(monolithMaxTemp.value);
  battery2Temp1.setValue(powerTankMinTemp.value);
  battery2Temp2.setValue(powerTankMaxTemp.value);
  battery1AH.setValue(monolithAH.value);
  battery2AH.setValue(powerTankAH.value);
  batteryMaxVolts.setValue(OUTPUT_VOLTS_MAX);

  chargerTargetVolts.setValue(OUTPUT_VOLTS_END);
  chargerTargetWatts.setValue(watts);
  chargerMaxWatts.setValue(MAX_STATION_WATTS);
  chargerCountBLE.setValue(chargerCount);
  //should also show max watts
}

void blePeripheralConnectHandler(BLEDevice central) {

}

void blePeripheralDisconnectHandler(BLEDevice central) {

}

#endif
