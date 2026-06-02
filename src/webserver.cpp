#include "webserver.h"

#include <WiFi.h>
#include <ArduinoJson.h>
#include "User.h"
#include "CardReader.h"

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
        <section>
          <h2>Make a new card (Enter Email, Password, and PIN) Submit and Present the card to the reader</h2>
          <form action="/create-card" method="POST">
            <input type="email" name="email" placeholder="Email" required><br><br>
            <input type="password" name="password" placeholder="Password" required><br><br>
            <input type="text" name="pin" placeholder="PIN (4 digits)" pattern="\d{4}" required><br><br>
            <button type="submit">Create Card</button>
          </form>
        </section>
      </body>
      </html>
    )rawliteral";

    request->send(200, "text/html", html); });

  server.begin();
}
void handleCreateCard()
{
  server.on("/create-card", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("email", true) && request->hasParam("password", true) && request->hasParam("pin", true))
    {
      String email = request->getParam("email", true)->value();
      String password = request->getParam("password", true)->value();
      String pin = request->getParam("pin", true)->value();

      User userToWrite(email, password, pin);
      String jsonData = userDataToJson(userToWrite);
      Serial.println("Writing JSON to card: ");
      Serial.println(jsonData);
      writeDataMultipleBlocks(jsonData);

      request->send(200, "text/plain", "Card created successfully! Please present the card to the reader.");
    }
    else
    {
      request->send(400, "text/plain", "Missing email, password, or pin.");
    } });
}