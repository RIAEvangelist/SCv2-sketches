#ifndef ZERO_V3_BLE_H
#define ZERO_V3_BLE_H
#define comma ","
#define debugSCFlag 0

void debugSCSetup(){
  if(!Serial && debugSCFlag){
    return;
  }

  Serial.begin(SERIAL_BAUD);
}

void debugSCLog(){
  if (!Serial && debugSCFlag) {
    return;
  }

  //battery volts
  Serial.print(volts);
  Serial.print(comma);

  //battery volts
  Serial.print(monolithVolts.value/100);
  Serial.print(comma);

  //battery volts
  Serial.print(powerTankVolts.value/100);
  Serial.print(comma);

  //total charging amps
  Serial.print(chargingAmps*chargerCount);
  Serial.print(comma);

  //total monolith amps
  Serial.print(monolithAmps.value);
  Serial.print(comma);

  //total power tank amps
  Serial.print(powerTankAmps.value);
  Serial.print(comma);

  //monolith C Rate
  Serial.print(monolithAH.value);
  Serial.print(comma);

  //power tank C Rate
  Serial.print(powerTankAH.value);
  Serial.print(comma);

  //c rate
  Serial.print(C_RATE/10);
  Serial.print(comma);

  //monolith temps
  Serial.print(monolithMinTemp.value);
  Serial.print(comma);
  Serial.print(monolithMaxTemp.value);
  Serial.print(comma);

  //powertank temps
  Serial.print(powerTankMinTemp.value);
  Serial.print(comma);
  Serial.print(powerTankMaxTemp.value);
  Serial.print(comma);

  //charger count
  Serial.print(chargerCount);
  Serial.print(comma);

  //target watts
  Serial.print(watts/100);
  Serial.print(comma);

  //MAX_STATION_WATTS
  Serial.print(MAX_STATION_WATTS/100);
  Serial.print(comma);

  //1C Plus
  Serial.print(CPlus);
  Serial.print(comma);

  //RAMP_RATE
  Serial.print(RAMP_RATE/100);

  //start extending with a comma
  Serial.print(comma);

  //print api version
  Serial.println(VERSION);
}

#endif
