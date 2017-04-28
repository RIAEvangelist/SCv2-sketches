#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);

unsigned char stmp[8] = {
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00
};

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup(){
  Serial.begin(115200);
  
  while (CAN_OK != CAN.begin(CAN_500KBPS)){
    delay(100);
  }
  
  if(Serial){
    Serial.println("CAN BUS Shield init ok!");
  }
  
  attachInterrupt(0, MCP2515_ISR, FALLING);
}

void MCP2515_ISR(){
    flagRecv = 1;
}

void loop(){
  //CAN.sendMsgBuf(0x1806E5F4, 1, 8, stmp);
   
  if(flagRecv){
        flagRecv = 0;
        
        while (CAN_MSGAVAIL == CAN.checkReceive()){
            CAN.readMsgBuf(&len, buf);
            
            unsigned int canId = CAN.getCanId();
        
            Serial.print(canId, HEX);
            Serial.print(" :\t");
            
            for(int i = 0; i<len; i++){
                Serial.print(buf[i]);Serial.print("\t");
            }
            Serial.println();
        }
        
  } 
}
