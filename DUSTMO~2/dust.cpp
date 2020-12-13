#include "dust.h"
#define sampling 280
#define waiting 40
float stop_time = 9680;

dust::dust(int A, int B){
  this->A=A;
  this->B=B;
  Minit();
}

//초기화
void dust::Minit(){
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  motor=MOTOR_OFF;
  autoM=Auto_Off;
}

//센서에서 데이터 수집
void dust::getDvalue(int pin){
  delayMicroseconds(sampling);
  dust_value=analogRead(pin);
  delayMicroseconds(waiting);
  delayMicroseconds(stop_time);
}

//공기질 농도 저장. 
float dust::getden(){
  dustDensityug=0;
  dustDensityug=(0.17*(dust_value*(5.0/1024))-0.1)*100; //원래는 1000배
  checkState();
  return dustDensityug;
}

//공지질 상태 저장
void dust::checkState(){
  if(dustDensityug>100){
    state=dustHIGH;
  }
  else{
    state=dustLOW;
  }
}

//Auto ON 일 때 실행
void dust::actionAutoOn(){
  autoM=Auto_On;
  if(state==dustHIGH){
     motor=MOTOR_ON;
    actionon();
  }
  else{
    motor=MOTOR_OFF;
    actionoff();
  }
}

//AutoOFF일 때
void dust::actionAutoOff(){
  autoM=Auto_Off;
}

//Motor ON 일 때
void dust::actionon(){
  motor=MOTOR_ON;
  digitalWrite(A, HIGH);
  digitalWrite(B, LOW);
  delay(10000);
}

//Motor OFF 일 때
void dust::actionoff(){
  motor=MOTOR_OFF;
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  delay(10000);
}

//공기질 상태값 반환
byte dust::getState(){
  return state;
}

//Motor 상태값 반환
byte dust::getMotor(){
  return motor;
}

//Auto 상태값 반환
byte dust::getAuto(){
  return autoM;
}
