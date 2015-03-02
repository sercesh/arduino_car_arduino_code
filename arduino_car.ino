
#include "EEPROM.h"

#define D1 2          // sol motor yönü
#define M1 3          // sol motor hızı
#define D2 4          // sag motor yönü
#define M2 5          // sag motor hızı

#define cmdL 'L'      // UART-command-sol motor
#define cmdR 'R'      // UART-command-sag motor
#define cmdH 'H'
#define cmdF 'F'     
#define cmdr 'r'      
#define cmdw 'w'    

char incomingByte;    

char L_Data[4];       // sol motor gelen bilgileri 
byte L_index = 0;     
char R_Data[4];       // sol motor gelen bilgileri
byte R_index = 0;     
char H_Data[1];       // array data for additional channel
byte H_index = 0;     
char F_Data[8];       
byte F_index = 0;     // index of array F
char command;         // command

unsigned long currentTime, lastTimeCommand, autoOFF;

void setup() {
  Serial.begin(9600);       
  pinMode(D1, OUTPUT);      
  pinMode(D2, OUTPUT);      
  timer_init();             
}

void timer_init() {
  uint8_t sw_autoOFF = EEPROM.read(0);   
  if(sw_autoOFF == '1'){                 
    char var_Data[3];
    var_Data[0] = EEPROM.read(1);
    var_Data[1] = EEPROM.read(2);
    var_Data[2] = EEPROM.read(3);
    autoOFF = atoi(var_Data)*100;        
  }
  else if(sw_autoOFF == '0'){         
    autoOFF = 999999;
  } 
  else if(sw_autoOFF == 255){ 
    autoOFF = 2500;                      
  } 
  currentTime = millis();                
}
 
void loop() {
  if (Serial.available() > 0) {         
    incomingByte = Serial.read();       
    if(incomingByte == cmdL) {          
      command = cmdL;                    
      memset(L_Data,0,sizeof(L_Data));  
      L_index = 0;                       
    }
    else if(incomingByte == cmdR) {      // sol motor için bilgi
      command = cmdR;
      memset(R_Data,0,sizeof(R_Data));
      R_index = 0;
    }
    else if(incomingByte == cmdH) {      
      command = cmdH;
      memset(H_Data,0,sizeof(H_Data));
      H_index = 0;
    }    
    else if(incomingByte == cmdF) { 
      command = cmdF;
      memset(F_Data,0,sizeof(F_Data));
      F_index = 0;
    }
    else if(incomingByte == '\r') command = 'e';   
    else if(incomingByte == '\t') command = 't';  
    
    if(command == cmdL && incomingByte != cmdL){
      L_Data[L_index] = incomingByte;              // gelen bilgileri arraye kaydet
      L_index++;                                  
    }
    else if(command == cmdR && incomingByte != cmdR){
      R_Data[R_index] = incomingByte;
      R_index++;
    }
    else if(command == cmdH && incomingByte != cmdH){
      H_Data[H_index] = incomingByte;
      H_index++;
    }    
    else if(command == cmdF && incomingByte != cmdF){
      F_Data[F_index] = incomingByte;
      F_index++;
    }    
    else if(command == 'e'){                       
      Control4WD(atoi(L_Data),atoi(R_Data),atoi(H_Data));
      delay(10);
    }
    else if(command == 't'){                       
      Flash_Op(F_Data[0],F_Data[1],F_Data[2],F_Data[3],F_Data[4]);
    }
    lastTimeCommand = millis();                    // uygulama başladığından beri zamanı tut
  }
  if(millis() >= (lastTimeCommand + autoOFF)){     // gelenle son zaman kontrolü
    Control4WD(0,0,0);                             // dur aga
  }
}

void Control4WD(int mLeft, int mRight){

  bool directionL, directionR;
  byte valueL, valueR;             
  
  if(mLeft > 0){
    valueL = mLeft;
    directionL = 0;
  }
  else if(mLeft < 0){
    valueL = 255 - abs(mLeft);
    directionL = 1;
  }
  else {
    directionL = 0;
    valueL = 0;
  }
 
  if(mRight > 0){
    valueR = mRight;
    directionR = 0;
  }
  else if(mRight < 0){
    valueR = 255 - abs(mRight);
    directionR = 1;
  }
  else {
    directionR = 0;
    valueR = 0;
  }
   
  analogWrite(M1, valueL);            // sol motor hızı
  analogWrite(M2, valueR);            // sag motor hızı
  digitalWrite(D1, directionL);       // sol motor yönü
  digitalWrite(D2, directionR);       // sag motor yönü
  
}

void Flash_Op(char FCMD, uint8_t z1, uint8_t z2, uint8_t z3, uint8_t z4){// ne yaptığımız konusunda fikrim yok

  if(FCMD == cmdr){		      // if EEPROM data read command
    Serial.print("FData:");	      // send EEPROM data
    Serial.write(EEPROM.read(0));     // read value from the memory with 0 address and print it to UART
    Serial.write(EEPROM.read(1));
    Serial.write(EEPROM.read(2));
    Serial.write(EEPROM.read(3));
    Serial.print("\r\n");	      // mark the end of the transmission of data EEPROM
  }
  else if(FCMD == cmdw){	      // if EEPROM write read command
    EEPROM.write(0,z1);               // z1 record to a memory with 0 address
    EEPROM.write(1,z2);
    EEPROM.write(2,z3);
    EEPROM.write(3,z4);
    timer_init();		      // reinitialize the timer
    Serial.print("FWOK\r\n");	      // send a message that the data is successfully written to EEPROM
  }
}
