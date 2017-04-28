#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 9;
const unsigned short MAX_C=129; //leave 12A for onboard
const unsigned short GOOD_JAMPS = 30; //calcs use a short so decimal is cut off. so 6.4kw is 30 and 6.6kW is 31
const unsigned short SHIT_JAMPS = 26; //calcs use a short so decimal is cut off. so 5.6kw is 26
const unsigned short MAX_V = 119;

unsigned short MAX_JAMPS = 0;
int shitPin = 7; 

MCP_CAN CAN(SPI_CS_PIN);

unsigned char stmp[8] = {
  //volts
  0x04, 0xa0, 
  //amps
  0x01, 0x7f, 
  //reserved
  0x00, 0x00, 0x00, 0x00
};

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup(){
  if(Serial){
    Serial.begin(115200);
  }
  
  pinMode(shitPin, INPUT_PULLUP);
  //digitalWrite(shitPin, LOW);
  
  while (CAN_OK != CAN.begin(CAN_250KBPS)){
    delay(100);
  }
  
  if(Serial){
    Serial.println("CAN BUS Shield init ok!");
  }
  
  attachInterrupt(0, MCP2515_ISR, FALLING);
}

void MCP2515_ISR(){
    flagRecv = 1;
//    if(Serial){      
//      Serial.println("got can");
//    }
}

void loop(){
  unsigned short chargingAmps=0;
  
  if(digitalRead(shitPin)){
//    if(Serial && MAX_JAMPS > SHIT_JAMPS){      
//      Serial.println("Treating as Shit Station Max watts : ");
//    }
    MAX_JAMPS = SHIT_JAMPS;
    if(MAX_JAMPS < SHIT_JAMPS){      
      chargingAmps=((MAX_JAMPS*208)/90)/2;
    }
//    if(Serial){      
//      Serial.println(MAX_JAMPS*208);
//    }
  }else{
//    if(Serial && MAX_JAMPS < GOOD_JAMPS){      
//      Serial.println("reating as good Station Max watts : ");
//    }
    MAX_JAMPS = GOOD_JAMPS;
    if(MAX_JAMPS < GOOD_JAMPS){      
      chargingAmps=((MAX_JAMPS*208)/90)/2;
    }
//    if(Serial){      
//      Serial.println(MAX_JAMPS*208);
//    }
  }
  
  
   
  if(flagRecv){
      flagRecv = 0;
      
      int chargerCount=0;
      unsigned short volts=0;
      unsigned short amps=0;
      unsigned long watts=0;
      
//      if(Serial){      
//        Serial.println("-----------------------------");
//      }
      
      while (CAN_MSGAVAIL == CAN.checkReceive()){
          CAN.readMsgBuf(&len, buf);
          chargerCount+=1;
          
          unsigned short chargerVolts=word(buf[0],buf[1])/10;
          if(chargerVolts>0){
            volts=volts+chargerVolts;
            if(chargerCount>1){
              volts=volts/2;
            }
          }
          amps+=word(buf[2],buf[3])/10;
          
//          if(Serial){
//            Serial.print(chargerCount);
//            Serial.print(" : volts - ");
//            Serial.print(word(buf[0],buf[1]));
//            Serial.print(", amps - ");
//            Serial.print(word(buf[2],buf[3]));
//            Serial.println();
//          }
      }
      
      watts=(amps*volts);
      
      unsigned short jAmps=watts/208;
      
      if(chargerCount>2){
        jAmps=((watts/3)*2)/208;
      }
      
      if(amps>MAX_C){    
        chargingAmps=MAX_C/chargerCount;
      }
      
      if(jAmps>MAX_JAMPS){          
        unsigned short chargingAmpsJ=((MAX_JAMPS*208)/volts)/2;
        if(chargingAmpsJ<chargingAmps){
          chargingAmps=chargingAmpsJ;
        }
      }
      
      if(chargingAmps>0){
        stmp[2]=highByte(chargingAmps*10);
        stmp[3]=lowByte(chargingAmps*10);
      }
      
      if(volts>MAX_V){
        if(Serial){
          Serial.println("Voltage Max exceeded, stopping charge");
        }
        stmp[2]=highByte(0);
        stmp[3]=lowByte(0);
      }
      
      
      if(Serial){
//        
//        Serial.print(chargerCount);
//        Serial.print(" chargers : ");
//        
        Serial.print(volts);
        Serial.print("v, ");
//        
        Serial.print(amps);
        Serial.print("A/");
        Serial.print(word(stmp[2],stmp[3])/10);
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
