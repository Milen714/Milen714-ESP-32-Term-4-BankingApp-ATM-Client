#pragma once // this is a header file, it is not meant to be compiled on its own

#include <ESPAsyncWebServer.h>

// extern = "this exists somewhere else"
extern AsyncWebServer server;

void setupWebServer();