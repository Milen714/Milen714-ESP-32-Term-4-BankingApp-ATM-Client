#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "User.h"

#define PN532_SDA 21
#define PN532_SCL 22
extern Adafruit_PN532 nfc;

void setupCardReader();
void readCard();
void readCardData();
void writeCard(const String &data);
void writeDataMultipleBlocks(const String &data);
String readDataMultipleBlocks();
void handleCardSerialInput();
String userDataToJson(const User &user);
User jsonToUserData(const String &json);
