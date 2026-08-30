#include <Arduino.h>
#include <Servo.h>
#include "Ultrasonic.h"

#define SERVO_PIN 10

Servo myServo;

const int trigPin = 4;
const int echoPin = 7;

float distance = 0;

int angle = 0;
int step = 2;

static float get_distance();

void Ultrasonic_Init(){
    myServo.attach(SERVO_PIN);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    Serial.print("**Ultrasonic Initialized**");
}

void Ultrasonic_Update(RobotState &state){
    if(state.scanning){
        myServo.write(angle);
        delay(20);
        distance = get_distance();

        Serial.print(distance);
        
        state.angle = angle;
        state.distance = int(distance);

        angle += step;
        if (angle >= 180 || angle <= 0)
            step = -step;
    }else if((state.scanning == false) && (state.angle != 90)){
        myServo.write(90);
        state.angle = 90;
        return;
    }
}

float Ultrasonic_GetDistance(){
    return distance;
}

int Ultrasonic_GetAngle(){
    return angle;
}

static float get_distance(){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);

    if (duration == 0)
        return -1;

    return duration * 0.0343 / 2;
}