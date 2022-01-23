#include <SoftwareSerial.h>

SoftwareSerial bikeSerial(3, 4);

long bauds[]={9600,19200,38400,57600,115200};
int currentBaud=0;
int maxBaud=5;

void commands(){
  Serial.println(bauds[currentBaud]);
  delay(500);
  checkOutput();
  delay(500);
  bikeSerial.println("+++");     //enter config
  delay(500);
  checkOutput();
  bikeSerial.println("AT+S=1");  //set baud to 9600
  delay(500);
  checkOutput();
  bikeSerial.println("AT+Q");    //enter normnal mode
  delay(500);
  checkOutput();
  delay(500);
  
}

void checkOutput(){
  bikeSerial.flush();
  while (bikeSerial.available()) {
    bikeSerial.flush();
    Serial.write(bikeSerial.read());
  }
  bikeSerial.flush();
}

void setup()
{
    Serial.begin(9600);
    
    bikeSerial.begin(
      bauds[currentBaud]
    );

    bikeSerial.flush();
    
    commands();
 
}


unsigned long id = 0;
unsigned char dta[8];

void loop()
{
    commands();
        
    bikeSerial.end();

    currentBaud++;

    if(currentBaud==maxBaud){
      currentBaud=0;
    }
    
    bikeSerial.begin(
      bauds[currentBaud]
    );
    bikeSerial.flush();
}


// END FILE
