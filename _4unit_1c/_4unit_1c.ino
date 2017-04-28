#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN);
unsigned char stmp[8] = {0x04, 0x8c, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x00};

void setup(){
   while (CAN_OK != CAN.begin(CAN_250KBPS)){
       delay(100);
   }
}

void loop(){
   CAN.sendMsgBuf(0x1806E5F4, 1, 8, stmp);
   delay(1000); 
}
