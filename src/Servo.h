#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

extern Servo atmServo;
#define SERVO_PIN 27

void setupServo();
void openAtmDoor();
void closeAtmDoor();
void moveServoToPosition(int angle);
void handleServoSerialInput();
