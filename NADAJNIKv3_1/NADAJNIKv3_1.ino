#include <PS2X_lib.h>
#include <SPI.h> //radio
#include "RF24.h" //radio

struct{
  uint16_t start:9; 
  uint8_t predkosc;
  uint8_t skret;
  bool hamulec;
  bool kierunkowskazL;
  bool kierunkowskazP;
  bool swiatla;
  bool nitro;
  bool neon;
  bool odleglosciomierz;
}msg;

PS2X ps2x; // create PS2 Controller Class
int error = 0; 
byte type = 0;
byte vibrate = 0;
const int ledPin=5;

RF24 radio(9,10);
const uint64_t pipe = 0xE8E8F0F0E1LL;
// ========= SETUP =========
void setup() {  
  Serial.begin(9600); //Virtual COM
 
  radio.begin(); //radio
  set_radio();
  radio.openWritingPipe(pipe); 

  error = ps2x.config_gamepad( 2,7,8,4, true, true );   //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  if(error == 0) Serial.println("Znaleziono kontroler :)"); //clock - SCK, command - MOSI, attention - SS, data - MISO
  else Serial.println("Nie znaleziono kontrolera :(");
  type = ps2x.readType(); 

  pinMode(ledPin, OUTPUT);
  Serial.println("Wykonano setup()"); //debug
  
  msg.start=456;
  delay(50);
}
// ======= LOOP ======
void loop() {
digitalWrite(ledPin, HIGH);
  ps2x.read_gamepad(false, vibrate);  
  
  msg.predkosc = ps2x.Analog(PSS_LY); //zczytanie stanu pada
  msg.skret = ps2x.Analog(PSS_RX);
  msg.hamulec = ps2x.Button(PSB_RED);
  msg.nitro = ps2x.Button(PSB_L1);
  msg.neon = ps2x.Button(PSB_L2);
  
  
  updateKierL( ps2x.Button(PSB_PAD_LEFT) );
  updateKierP( ps2x.Button(PSB_PAD_RIGHT) );
  updateOdl( ps2x.Button(PSB_SELECT) );
  updateSwiatla( ps2x.Button(PSB_GREEN) );
 
  
  radio.write(&msg, sizeof(msg)); //poprawione
  radio.write(0,8);

digitalWrite(ledPin, LOW);
}
//================================================================

void set_radio(){
  radio.setPayloadSize(32);
  if( radio.setDataRate( RF24_2MBPS ) ) Serial.println("Ustawiono predkosc transmisji na 2Mbps.");
  else Serial.println("Niepowodzenie przy ustawianiu predkosci transmisji na 2Mbps");
}

void updateKierL(bool kierL){
  static bool kierL_poprz;
  static bool sw;
  static uint8_t co;
  if(kierL!=kierL_poprz){ //on-off kierunkowskaz L
   co++;
      if(co%2==0){
        if(sw==0){
          sw=1;
          msg.kierunkowskazL=1;
        }
        else{
          sw=0;
          msg.kierunkowskazL=0;
        }
      }
  }
  kierL_poprz=kierL;
}

void updateKierP(bool kierP){
  static bool kierP_poprz;
  static bool sw;
  static uint8_t co;
  if(kierP!=kierP_poprz){ //on-off kierunkowskaz P
   co++;
      if(co%2==0){
        if(sw==0){
          sw=1;
          msg.kierunkowskazP=1;
        }
        else{
          sw=0;
          msg.kierunkowskazP=0;
        }
      }
  }
  kierP_poprz=kierP;
}

void updateSwiatla(bool swiatla){
  static bool swiatla_poprz;
  static bool sw;
  static uint8_t co;
  if(swiatla!=swiatla_poprz){ //on-off swiatla
   co++;
      if(co%2==0){
        if(sw==0){
          sw=1;
          msg.swiatla=1;
        }
        else{
          sw=0;
          msg.swiatla=0;
        }
      }
  }
  swiatla_poprz=swiatla;
}

void updateOdl(bool odl){
  static bool odl_poprz;
  static bool sw;
  static uint8_t co;
  if(odl!=odl_poprz){ //on-off odleglosciomierz
   co++;
      if(co%2==0){
        if(sw==0){
          sw=1;
          msg.odleglosciomierz=1;
        }
        else{
          sw=0;
          msg.odleglosciomierz=0;
        }
      }
  }
  odl_poprz=odl;
}
