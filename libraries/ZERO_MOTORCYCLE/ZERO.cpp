#include "ZERO.h"

Zero::Zero(){

}

Zero::~Zero(){

}

byte Zero::messageLength=ZERO_MESSAGE_LENGTH;

byte Zero::hasVoltage(short &canId){
  if(canId == CALEX_CHARGE_CONTROL.id
    || canId == BMS_CELL_VOLTAGE.id
    || canId == BMS1_CELL_VOLTAGE.id ) {

      return 1;

  }
    return 0;
}

byte Zero::hasMonolithVoltage(short &canId){
  if(canId == BMS_CELL_VOLTAGE.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasPowerTankVoltage(short &canId){
  if(canId == BMS1_CELL_VOLTAGE.id ) {

      return 1;

  }
    return 0;
}

byte Zero::hasAmps(short &canId){
  if(canId == BMS_PACK_ACTIVE_DATA.id
    || canId == BMS1_PACK_ACTIVE_DATA.id ) {

      return 1;

  }
    return 0;
}

byte Zero::hasMonolithAmps(short &canId){
  if(canId == BMS_PACK_ACTIVE_DATA.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasPowerTankAmps(short &canId){
  if(canId == BMS1_PACK_ACTIVE_DATA.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasMaxCRate(short &canId){
  if(canId == BMS_PACK_TIME.id
    || canId == BMS1_PACK_TIME.id ) {

      return 1;

  }
    return 0;
}

byte Zero::hasMonolithMaxCRate(short &canId){
  if(canId == BMS_PACK_TIME.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasPowerTankMaxCRate(short &canId){
  if(canId == BMS1_PACK_TIME.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasPackTime(short &canId){
  if(canId == BMS_PACK_TIME.id
    || canId == BMS1_PACK_TIME.id ) {

      return 1;

  }
    return 0;
}

byte Zero::hasMonolithPackTime(short &canId){
  if(canId == BMS_PACK_TIME.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasPowerTankPackTime(short &canId){
  if(canId == BMS1_PACK_TIME.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasPackConfig(short &canId){
  if(canId == BMS_PACK_CONFIG.id
    || canId == BMS1_PACK_CONFIG.id ) {

      return 1;

  }
    return 0;
}

byte Zero::hasMonolithPackConfig(short &canId){
  if(canId == BMS_PACK_CONFIG.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasPowerTankPackConfig(short &canId){
  if(canId == BMS1_PACK_CONFIG.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasPackActiveData(short &canId){
  if(canId == BMS_PACK_ACTIVE_DATA.id
    || canId == BMS1_PACK_ACTIVE_DATA.id ) {

      return 1;

  }
    return 0;
}

byte Zero::hasMonolithPackActiveData(short &canId){
  if(canId == BMS_PACK_ACTIVE_DATA.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasPowerTankPackActiveData(short &canId){
  if(canId == BMS1_PACK_ACTIVE_DATA.id) {

      return 1;

  }
    return 0;
}

byte Zero::hasThrottle(short &canId){
  if(canId == CONTROLLER_RPM_THROTTLE_MOT_TEMP.id) {

      return 1;

  }
    return 0;
}

short Zero::throttle(byte &len,byte buf[ZERO_MESSAGE_LENGTH]){
  ByteToShort throttleVoltage;

  throttleVoltage.bytes[0]=buf[4];
  throttleVoltage.bytes[1]=buf[5];

  //Serial.println(throttleVoltage.value);

  return throttleVoltage.value;
}

short Zero::amps(byte &len,byte buf[ZERO_MESSAGE_LENGTH]){
  ByteToShort amps;

  amps.bytes[0]=buf[3];
  amps.bytes[1]=buf[4];

  //Serial.println(amps.value);

  return amps.value;
}

long Zero::voltage(byte &len,byte buf[ZERO_MESSAGE_LENGTH]){
  ByteToLong voltage;

  voltage.bytes[0]=buf[3];
  voltage.bytes[1]=buf[4];
  voltage.bytes[2]=buf[5];
  voltage.bytes[3]=buf[6];

  //Serial.println(voltage.value);

  return voltage.value;
}

short Zero::sagAdjust(byte &len,byte buf[ZERO_MESSAGE_LENGTH]){
  ByteToShort sagAdjust;

  sagAdjust.bytes[0]=buf[0];
  sagAdjust.bytes[1]=buf[1];

  //Serial.println(sagAdjust.value);

  return sagAdjust.value;
}

short Zero::maxCRate(byte &len,byte buf[ZERO_MESSAGE_LENGTH]){
  ByteToShort CRate;

  CRate.bytes[0]=buf[4];
  CRate.bytes[1]=buf[5];

  //Serial.println(maxChargingAmps.value);

  return CRate.value;
}

short Zero::AH(byte &len,byte buf[ZERO_MESSAGE_LENGTH]){
  ByteToShort AH;

  AH.bytes[0]=buf[5];
  AH.bytes[1]=buf[6];

  //Serial.println(AH.value);

  return AH.value;
}

short Zero::highestTemp(byte &len,byte buf[ZERO_MESSAGE_LENGTH]){
  ByteToShort temp;

  temp.bytes[0]=buf[1];

  //Serial.println(temp.value);

  return temp.value;
}

short Zero::lowestTemp(byte &len,byte buf[ZERO_MESSAGE_LENGTH]){
  ByteToShort temp;

  temp.bytes[0]=buf[2];

  //Serial.println(AH.value);

  return temp.value;
}

void Zero::logInit(){
  if(Serial){
    Serial.begin(SERIAL_BAUD);
  }
}

void Zero::logRaw(byte &len,byte buf[ZERO_MESSAGE_LENGTH],short &canId){
  if(!Serial){
    return;
  }
  byte i;
  byte knownMessage=0;
  byte show=0;

  for(i=0; i<TOTAL_MESSAGES; i++){
    if(canId==Zero_Messages[i].id){
      knownMessage=1;
      show=1;
      Serial.println("-----------------------------");
      Serial.println(Zero_Messages[i].name);
      break;
    }
  }


  Serial.print(" ID: ");
  Serial.println(canId,HEX);
  Serial.println("-----------------------------");
  for(int i = 0; i<len; i++){
      Serial.print(buf[i], HEX);
      Serial.print("\t");
  }

  Serial.println();

}
