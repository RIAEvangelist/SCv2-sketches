
// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13


#include <SPI.h>
#include "mcp_can.h"


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    Serial.begin(38400);

    while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
}


void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned int canId = CAN.getCanId();
        
        Serial.println("-----------------------------");
        Serial.print("Get data from ID: ");
        Serial.println(canId, HEX);

        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buf[i], HEX);
            Serial.print("\t");
        }

        Serial.print(word(buf[0], buf[1]));
        Serial.print("\t");
        Serial.print(word(buf[2], buf[3]));
        Serial.println();
    }
}

