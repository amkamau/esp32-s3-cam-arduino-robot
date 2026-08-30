#include <Arduino.h>
#include "Movements.h"

/* ----------------------------------------- PWM servo shield--------------------------------------------*/
const int motorLeft_EN = 5; //PWM
const int motorLeft_IN1 = 12;
const int motorLeft_IN2 = 13;

const int motorRight_EN = 6; //PWM
const int motorRight_IN1 = 8;
const int motorRight_IN2 = 9;


void Movements_Init(){
  // Motor 1
  pinMode(motorLeft_EN, OUTPUT);
  pinMode(motorLeft_IN1, OUTPUT);
  pinMode(motorLeft_IN2, OUTPUT);
  // Motor 2
  pinMode(motorRight_EN, OUTPUT);
  pinMode(motorRight_IN1, OUTPUT);
  pinMode(motorRight_IN2, OUTPUT);

  Serial.print("**Movements Initialized**");

}

/* ****************************************************** */
void left_Reverse(int lspeed){
    analogWrite(motorLeft_EN, lspeed);
    digitalWrite(motorLeft_IN1,  HIGH);
    digitalWrite(motorLeft_IN2, LOW);
}
void right_Reverse(int rspeed){
    analogWrite(motorRight_EN, rspeed);
    digitalWrite(motorRight_IN1,  HIGH);
    digitalWrite(motorRight_IN2, LOW);
}

void left_Forward(int lspeed){
    analogWrite(motorLeft_EN, lspeed);
    digitalWrite(motorLeft_IN1,  LOW);
    digitalWrite(motorLeft_IN2, HIGH);
}
void right_Forward(int rspeed){
    analogWrite(motorRight_EN, rspeed);
    digitalWrite(motorRight_IN1,  LOW);
    digitalWrite(motorRight_IN2, HIGH);
}

void left_Stop(){
    digitalWrite(motorLeft_IN1,  LOW);
    digitalWrite(motorLeft_IN2, LOW);
}
void right_Stop(){
    digitalWrite(motorRight_IN1,  LOW);
    digitalWrite(motorRight_IN2, LOW);
}


/* ****************************************************** */
/* Forwad Reverse Left Right  movements*/
void move_Forward(int speed){
    left_Forward(speed);
    right_Forward(speed);
}
void move_Backward(int speed){
    left_Reverse(speed);
    right_Reverse(speed);
}
void turn_Left(int speed){
    left_Stop();
    right_Forward(speed);    
}
void turn_Right(int speed){
    right_Stop();
    left_Forward(speed);    
}
void motors_stop(){
    right_Stop();
    left_Stop();
}

void Movements_Update(RobotState &state){  
    if (millis() - state.lastMoveInit >= 1000) {
        motors_stop();
        return;
    }
    if (state.direction == 8){              //Forward
    move_Forward(state.motorSpeed);
    } else if (state.direction == 2){       // Reverse
        move_Backward(state.motorSpeed);
    } else if (state.direction == 4){       // Left
        turn_Left(state.motorSpeed);
    } else if (state.direction == 6){       // Right
        turn_Right(state.motorSpeed);
    }
}

