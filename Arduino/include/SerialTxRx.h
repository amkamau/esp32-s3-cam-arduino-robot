#pragma once

#include "RobotState.h"

void SerialEsp_Init();

void Serial_Read(RobotState &state);

void Serial_Send(const RobotState &state);