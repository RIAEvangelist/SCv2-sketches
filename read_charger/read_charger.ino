

// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13


#include <SPI.h>
#include "mcp_can.h"
#include <SoftwareSerial.h>


#define btRX A0
#define btTX A1
#define BAUD_RATE 38400

// set up a new serial object
SoftwareSerial btSerial (btRX, btTX);


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    if (btSerial) {
      // define pin modes for tx, rx:
      pinMode(btRX, INPUT);
      pinMode(btTX, OUTPUT);
      // set the data rate for the SoftwarebtSerial port
      btSerial.begin(BAUD_RATE);
      btSerial.println("STARTING DEBUG PROCESS");
  }

    while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 500k
    {
        btSerial.println("CAN BUS Shield init fail");
        btSerial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    btSerial.println("CAN BUS Shield init ok!");
}


void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned int canId = CAN.getCanId();
        
        btSerial.println("-----------------------------");
        btSerial.print("Get data from ID: ");
        btSerial.println(canId, HEX);

        for(int i = 0; i<len; i++)    // print the data
        {
            btSerial.print(buf[i], HEX);
            btSerial.print("\t");
        }

        btSerial.print(word(buf[0], buf[1]));
        btSerial.print("\t");
        btSerial.print(word(buf[2], buf[3]));
        btSerial.println();
    }
}

