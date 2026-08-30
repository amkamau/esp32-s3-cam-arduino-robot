#include <Arduino.h>
#include "IRSensors.h"

#define FRONT_LEFT A3
#define FRONT_RIGHT A2
#define REAR_LEFT A1
#define REAR_RIGHT A0

void IRSensors_Init()
{
    pinMode(FRONT_LEFT, INPUT);
    pinMode(FRONT_RIGHT, INPUT);
    pinMode(REAR_LEFT, INPUT);
    pinMode(REAR_RIGHT, INPUT);

    Serial.print(" ** IR Initialized ** ");
}

void IRSensors_Update(RobotState &state)
{
    int FR = digitalRead(FRONT_RIGHT);
    int FL = digitalRead(FRONT_LEFT);
    int RR = digitalRead(REAR_RIGHT);
    int RL = digitalRead(REAR_LEFT);

    state.frontLeft  = (FL == LOW);
    state.frontRight = (FR == LOW);
    state.rearLeft   = (RL == LOW);
    state.rearRight  = (RR == LOW);  

}