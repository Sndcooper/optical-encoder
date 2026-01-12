#include <Arduino.h>

#define enc 2

#define ENA 4
#define IN1 16
#define IN2 17

#define pot 12

volatile unsigned long count;
int speed;
unsigned long t1 = millis();
unsigned long t2 = t1; 
unsigned long t3 = t1; 

int freq_for_RPM = 20; //freq for output

void counter(){
  if ((t1-t2)>1){
  count++;
  t2 = t1;
  }
}
void forward(int rightSpeed) {
  if (rightSpeed > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, rightSpeed);
  }else{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, -rightSpeed);
  }
}

void setup(){
  t1 = millis();
  pinMode(enc, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(enc), counter, RISING);
  Serial.begin(9600);

}
int pulses=0;
unsigned long pulse=0;
void  loop(){
  speed = analogRead(pot);
  speed = map(speed, 0, 1024, 50, 255);
  forward(speed); 
  t1= millis();
  if(t1-t3 >= 1000/freq_for_RPM){
    t3 = t1;
    noInterrupts();
    pulses = count;
    count = 0;
    pulse += pulses;
    interrupts();
    double rpm = ((60*freq_for_RPM)/64.0)*(double)pulses;
    Serial.print(pulses);
    Serial.print(" pulses: ");
    Serial.print(pulse);
    Serial.print(" rpm: ");
    Serial.println(rpm, 5);
  }
}