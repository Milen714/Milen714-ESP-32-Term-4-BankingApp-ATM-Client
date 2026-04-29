#include "webserver.h"

#include <WiFi.h>
#include <ArduinoJson.h>
#include "User.h"

// From main.cpp
extern User currentUser;

void setupWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <title>ESP32 ATM</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
      </head>
      <body style="font-family: Arial; margin: 2rem;">
        <h1>ESP32 ATM</h1>
        <p><strong>Current User:</strong> )rawliteral" + currentUser.fullName() + R"rawliteral(</p>
      </body>
      </html>
    )rawliteral";

    request->send(200, "text/html", html); });

    server.begin();
}