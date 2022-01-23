#ifndef ZERO_BIKE_H
#define ZERO_BIKE_H

ByteToShort bikeThrottle;

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

void bikeSetup(){
  // initialize the CAN bus at baud rate 250kbps
  while (CAN_OK != Bike.begin(CAN_500KBPS)) {
    delay(100);
  }
}

void bikeLoop(){
  while(CAN_MSGAVAIL == Bike.checkReceive()){
    byte len = 0;
    byte buf[zero.messageLength];

    Bike.readMsgBuf(&len, buf);

    short canId = Bike.getCanId();

    if(zero.hasMonolithVoltage(canId)){
      monolithVolts.value= zero.voltage(len,buf);
      bikeVolts.value=monolithVolts.value;
      volts=bikeVolts.value/100;
    }

    if(zero.hasMonolithAmps(canId)){
      monolithAmps.value= abs(zero.amps(len,buf));
      bikeAmps.value=monolithAmps.value+powerTankAmps.value;
      if(monolithAH.value>0 && monolithAmps.value>monolithAH.value && CPlus<1 && delayItteration>10){
        chargingAmps=(monolithAH.value-2)*10;
        delayItteration=10;
      }
    }

    if(zero.hasPowerTankVoltage(canId)){
      powerTankVolts.value=zero.voltage(len,buf);
      if(powerTankVolts.value>bikeVolts.value){
          bikeVolts.value=powerTankVolts.value;
          volts=bikeVolts.value/100;
      }
    }

    if(zero.hasPowerTankAmps(canId)){
      powerTankAmps.value= abs(zero.amps(len,buf));
      bikeAmps.value=monolithAmps.value+powerTankAmps.value;
      if(powerTankAH.value>0 && powerTankAmps.value>powerTankAH.value && CPlus<1 && delayItteration>10){
        chargingAmps=(powerTankAH.value-2)*10;
        delayItteration=10;
      }
    }

    if(zero.hasMonolithPackConfig(canId)){
      monolithAH.value= zero.AH(len,buf);
      C_RATE=(monolithAH.value + powerTankAH.value)*10;
    }

    if(zero.hasPowerTankPackConfig(canId)){
      powerTankAH.value= zero.AH(len,buf);
      C_RATE=(monolithAH.value + powerTankAH.value)*10;
    }

    if(zero.hasMonolithPackActiveData(canId)){
      monolithMaxTemp.value= zero.highestTemp(len,buf);
      monolithMinTemp.value= zero.lowestTemp(len,buf);
      lowTemp=monolithMinTemp.value;
      highTemp=monolithMaxTemp.value;
    }

    if(zero.hasPowerTankPackActiveData(canId)){
      powerTankMaxTemp.value= zero.highestTemp(len,buf);
      powerTankMinTemp.value= zero.lowestTemp(len,buf);
      lowTemp=powerTankMinTemp.value;
      highTemp=powerTankMaxTemp.value;
    }

    if(zero.hasThrottle(canId)){
      bikeThrottle.value= zero.throttle(len,buf);
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
}

#endif
