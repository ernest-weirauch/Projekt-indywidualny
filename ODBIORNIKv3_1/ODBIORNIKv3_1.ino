#include <Timers.h> //wielozadaniowość
#include <Servo.h> //serwo
#include <SPI.h> //radio
#include "RF24.h" //radio
#include <Ultrasonic.h> //hc-sr04

#define RDLED 8 //led

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

struct{
  bool motorPin1, motorPin2;
  bool brk;
  long odleglosc;
  bool kierL, kierP; //
  uint8_t jasnosc;
}stan;

//DEKLARACJE ZBĘDNE!
Servo serwo;  // create servo object to control a servo 
const int motorPin1 =2; // H-bridge leg 1 (Motor Logic Pin 1)
const int motorPin2 =4; // H-bridge leg 2 (Motor Logic Pin 2)
#define speedPin 5 // H-bridge PWM (Speed) / Enable pin
#define lightsPin 6 //PWM
#define kierunkowskazLPin A1 
#define kierunkowskazPPin A2
RF24 radio( 9,10 ); //Inicjalizacja radia (piny)
const uint64_t pipe = 0xE8E8F0F0E1LL; //Kanał komunikacyjny

Ultrasonic ultrasonic(0,1); // (Trig PIN,Echo PIN)

TimerPool timer(3); //Dekl (n) slotowego timera
void setup(){
  serwo.attach(7); //Ustawienie serwa na pinie 
  
  pinMode( motorPin1, OUTPUT ); //Mostek-H
  pinMode( motorPin2, OUTPUT ); 
  pinMode( speedPin, OUTPUT );
  
  pinMode( A0, INPUT ); //Jasnosc swiatel
  pinMode( lightsPin, OUTPUT );
  pinMode( A1, OUTPUT ); //kierunkowskazy
  pinMode( A2, OUTPUT );
  
  radio.begin(); //uruch radio
  set_radio();  //Radio
  radio.openReadingPipe( 1, pipe );
  radio.startListening();
  
  pinMode(RDLED, OUTPUT);
  timer.connect(0, 1, rc); //id, time(ms), func
  timer.connect(1, 15, pomiar_odl); 
  timer.connect(2, 40, pomiar_jasnosci); //25fps
  
  delay(50);
}

void loop(){
  timer.update(); //aktualizacja stanu timera 
}
//================================================================
void rc(){
   if (radio.available()){ //jesli odebrane dane
     digitalWrite(RDLED, HIGH); //dioda statusu (zapala jeśli otrzymuje dane)
      radio.read( &msg, sizeof(msg) ); //odczytanie danych  
      if(msg.start!=456) return;
      go(msg.predkosc);
      
      if(msg.hamulec){ //hamulec
        brk(1);
      } else if(stan.brk) brk(0); //fix hamulca
      
      static bool flaga=0;
      if(msg.odleglosciomierz){ //
        if(stan.odleglosc!=0 && stan.odleglosc<=20) {       
          brk(1);
          digitalWrite(kierunkowskazLPin, HIGH);
          digitalWrite(kierunkowskazPPin, HIGH);
          flaga=1;
        } 
        else{
          brk(0);
          digitalWrite(kierunkowskazLPin, LOW);
          digitalWrite(kierunkowskazPPin, LOW);
          flaga=0;
        }
      } 
      else if(!flaga && stan.brk) brk(0);

      turn(msg.skret);
                                
      if(msg.kierunkowskazL) kierunkowskazL(1); //kierunkowskazy
        else kierunkowskazL(0);
      if(msg.kierunkowskazP) kierunkowskazP(1);
        else kierunkowskazP(0);
       
      digitalWrite(RDLED, LOW);
    }     
}

void go(uint8_t pred){
  if( pred>128 ){    //tyl
    pred=map(pred, 129, 255, 0, 255);
    motorPin1Chng(1);
    motorPin2Chng(0);
  } 
  else if( pred<128 ){ //przod
    pred=map(pred, 127, 0, 0, 255);
    motorPin1Chng(0);
    motorPin2Chng(1);
  }
  else pred=0;
  analogWrite( speedPin,  pred );
}

void turn(uint8_t ang){
    serwo.write( map(ang, 255, 0, 55, 180-65) );
}

void brk(bool tmp){
  if(tmp){
    digitalWrite(motorPin1, HIGH); //dobrze bo nie zmienia stanu zeby moc w elsie przywrocic po hamowaniu
    digitalWrite(motorPin1, HIGH);
    stan.brk=1;
  }
  else{ //przywrocenie ustawien pinow sprzed hamowania
    if(stan.motorPin1) digitalWrite(motorPin1, HIGH);
    else digitalWrite(motorPin1, LOW);
    if(stan.motorPin2) digitalWrite(motorPin2, HIGH);
    else digitalWrite(motorPin2, LOW); 
    stan.brk=0;
  }
}

void set_radio(){
  radio.setPayloadSize(32);
  radio.setDataRate( RF24_2MBPS );
}

void pomiar_odl(){
  if(!msg.odleglosciomierz) stan.odleglosc = 0;
  else stan.odleglosc = ultrasonic.Ranging(CM);
}

void pomiar_jasnosci(){
  if(!msg.swiatla) return;
  else stan.jasnosc = map( analogRead(A0), 0, 1023, 255, 0 ); //jasniejsze otoczenie = wiekszy opor = ciemniejsze swiatla
}

void kierunkowskazL(bool todo){ //todo=1 wlacz, todo=0 wylacz
  static uint8_t czas = 499;
  static bool sstatus=0;
   if(todo==0){
      digitalWrite(kierunkowskazLPin, LOW);
      sstatus=0;
      czas=499;
      return;
    }
  else{
    czas++;
    if(czas%200==0){
      if(sstatus){
        digitalWrite(kierunkowskazLPin, LOW);
        sstatus=0;
      }
      else{
        digitalWrite(kierunkowskazLPin, HIGH);
        sstatus=1;
      }
    }  
  }  
}

void kierunkowskazP(bool todo){
  static uint8_t czas = 499;
  static bool sstatus=0;
   if(todo==0){
      digitalWrite(kierunkowskazPPin, LOW);
      sstatus=0;
      czas=499;
      return;
    }
  else{
    czas++;
    if(czas%200==0){
      if(sstatus){
        digitalWrite(kierunkowskazPPin, LOW);
        sstatus=0;
      }
      else{
        digitalWrite(kierunkowskazPPin, HIGH);
        sstatus=1;
      }
    }  
  }  
}

void motorPin1Chng(bool tmp){
  if(tmp){
    stan.motorPin1=1;
    digitalWrite(motorPin1, HIGH);
  }
  else{
    stan.motorPin1 = 0;
    digitalWrite(motorPin1, LOW);
  }
}

void motorPin2Chng(bool tmp){
  if(tmp){
    stan.motorPin2 = 1;
    digitalWrite(motorPin2, HIGH);
  }
  else{
    stan.motorPin2 = 0;
    digitalWrite(motorPin2, LOW);
  }
}


