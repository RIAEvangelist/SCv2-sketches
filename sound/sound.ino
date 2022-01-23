int speaker    = 8;
int throttle   = A5;

unsigned int sound       = 0;
unsigned int currentBase = 0;
unsigned int soundBase   = 40;
unsigned int RPM         = 0;
unsigned int curThrottle = 0;
unsigned int diff        = 0;


boolean soundUp= true;

void setup() {
  pinMode(8, OUTPUT);
  pinMode(A5, INPUT);
  
  tone(speaker,sound);
}

void loop() {
  

  currentBase=soundBase+(analogRead(throttle)/10);
  
  //Serial.println((String)currentBase+" - "+(String)RPM+(String)analogRead(throttle));
  
  if(currentBase<2048 && currentBase>=0){
    if(sound>currentBase+25)
      sound=currentBase-1;

    if(sound<currentBase-25)
      sound=currentBase+1;
    
    if(sound>currentBase){
      sound+=1;
    }else{
      sound-=3;
    }
  }else{
    currentBase=0;
    sound=currentBase;
  }
  
  tone(speaker,sound); 
  delay(2);
}
