#include <Arduino.h>
#include "RobotState.h"
#include "SerialTxRx.h"
#include "Ultrasonic.h"
#include "IRSensors.h"
#include "GyroSensor.h"
#include "Movements.h"

RobotState robot;

void setup()
{
    Serial.begin(115200);
    Serial.println("......Init Started....");
    SerialEsp_Init();
    Ultrasonic_Init();
    IRSensors_Init();
    Gyro_Init();
    Movements_Init();
    Serial.println("......Init completed....");
}

void loop()
{

    Serial_Read(robot);

    Movements_Update(robot);

    Ultrasonic_Update(robot);

    IRSensors_Update(robot);

    Gyro_Update(robot);

    Serial_Send(robot);
}