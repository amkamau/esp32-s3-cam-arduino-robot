#include <Arduino.h>
#include <SoftwareSerial.h>
#include "SerialTxRx.h"
#include <SoftwareSerial.h>

static String rxBuffer = "";

SoftwareSerial espSerial(2, 3);  // RX, TX

static String prevRobotState = "";
static unsigned long lastAngleX = 0;
static unsigned long lastAngleY = 0;
static unsigned long lastAngleZ = 0;

void SerialEsp_Init() {
    espSerial.begin(115200);
    Serial.print(" ** SerialESP Initialized ** ");
}

void Serial_Decode(RobotState &state, const String &command);

void Serial_Read(RobotState &state)
{
    while (espSerial.available()) {
        char c = espSerial.read();        
        if (c == '\n') {
            if (rxBuffer.length() > 0) {
                Serial.println("Received commands : " + rxBuffer);
                Serial_Decode(state, rxBuffer);
                rxBuffer = "";
            }else{
                state.direction = 5; 
            }
        } else if (c != '\r') {            
            rxBuffer += c;            
        }
    }
}

void Serial_Send(const RobotState &state){  
    
    // if (millis() - lastSend < 40) return;   // 25 Hz
    // lastSend = millis();
    
    if (lastAngleX == 0 || abs(lastAngleX-state.angleX) >= 5){lastAngleX = state.angleX;}   
    if (lastAngleY == 0 || abs(lastAngleY-state.angleY) >= 5){lastAngleY = state.angleY;}   
    if (lastAngleZ == 0 || abs(lastAngleZ-state.angleZ) >= 5){lastAngleZ = state.angleZ;}   

    String packet =
            String(state.angle) + "," +
            String(state.distance) + ",(" +
            String(state.frontLeft) + String(state.frontRight) + String(state.rearLeft) + String(state.rearRight) + ")," +
            String(state.scanning) + ",(" +
            String(lastAngleX) + "," +
            String(lastAngleY) + "," +
            String(lastAngleZ) + "),(" +
            String(state.direction) + "," +
            String(state.motorSpeed) + ")";


    if (packet != prevRobotState || prevRobotState == ""){ 
        espSerial.println(packet);
        Serial.println("Robot state: " + packet);        
    }
    prevRobotState = packet; 

}




void Serial_Decode(RobotState &state, const String &command)
{
    // Msample      "111 1-001 2-81150"
    if ((command.length() != 13) || (command.substring(0, 3) != "111"))
        return;

    // Module 1 - Servo Scanner
    String m1_scanner = command.substring(3, 7);
    // First character = module number
    if (m1_scanner.charAt(0) == '1'){
        // Remaining 3 characters = command
        String cmd = m1_scanner.substring(1, 4);
        if (cmd == "001" && !state.scanning){
            state.scanning = true;
            Serial.println("Scan Started");
        }else if (cmd == "000" && state.scanning){
            state.scanning = false;
            Serial.println("Scan Stopped");
        }
    }

    // Module 2 - Motors
    if (command.charAt(7) == '2') {

        state.direction = command.substring(8, 9).toInt();
        state.motorSpeed = command.substring(9, 12).toInt();
        if(command.substring(12, 13).toInt() == 1){
            state.lastMoveInit = millis();
        }
    }
}
