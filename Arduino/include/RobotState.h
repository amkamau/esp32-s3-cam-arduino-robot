#pragma once

struct RobotState
{
    // Ultrasonic
    int angle = 0;
    int distance = 0;

    // IR sensors
    bool frontLeft = false;
    bool frontRight = false;
    bool rearLeft = false;
    bool rearRight = false;

    // Commands from PC
    bool scanning = false;
    bool ledState = false;

    // Gyro
    float angleX = 0;
    float angleY = 0;
    float angleZ = 0;
    unsigned long lastTime = 0;

    // movements
    int direction = 5;   // 8 Forwawrd ,2 reverse, 4 Left , 6 Right , 5 Stop
    int motorSpeed = 150;
    unsigned long lastMoveInit = 0;

};