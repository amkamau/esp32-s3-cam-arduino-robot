#include <Arduino.h>
#include <Wire.h>
#include "GyroSensor.h"
#include "L3G4200D.h"

L3G4200D gyro;

void Gyro_Init(){

  Wire.begin();
  // Initialize gyroscope (2000 DPS range)
  while (!gyro.begin(L3G4200D_SCALE_2000DPS)){
    Serial.print("L3G4200D not detected. Check wiring.**");
    delay(500);
  }

  Serial.println("**Calibrating... Keep sensor still.**");
  delay(2000);
  gyro.calibrate();
  // Noise threshold
  gyro.setThreshold(3);

}

void Gyro_Update(RobotState &state){

    if(state.lastTime != 0){    // check if first run
        unsigned long currentTime = millis();
        float dt = (currentTime - state.lastTime) / 1000.0; // seconds
        state.lastTime = currentTime;

        // Read normalized angular velocity (°/s)
        Vector g = gyro.readNormalize();

        // Integrate angular velocity to get angle
        state.angleX += g.XAxis * dt;
        state.angleY += g.YAxis * dt;
        state.angleZ += g.ZAxis * dt;

        //   // Print angular velocity
        // Serial.print("Gyro (°/s) -> ");
        // Serial.print("X: "); Serial.print(g.XAxis);
        // Serial.print(" Y: "); Serial.print(g.YAxis);
        // Serial.print(" Z: "); Serial.print(g.ZAxis);

        // Print integrated angle
        // Serial.print(" | Angle (°) -> ");
        // Serial.print("X: "); Serial.print(state.angleX);
        // Serial.print(" Y: "); Serial.print(state.angleY);
        // Serial.print(" Z: "); Serial.println(state.angleZ);

    }else{
        state.lastTime = millis();
    }
  
}