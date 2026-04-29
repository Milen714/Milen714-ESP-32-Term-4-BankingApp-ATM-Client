#include "WiFiManager.h"
#include "display.h"

void connectWiFi(const char *ssid, const char *password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");
    drawHeader("Connecting to WiFi...");

    unsigned long startAttempt = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 20000)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("Connected. IP: ");
        Serial.println(WiFi.localIP());
        drawHeader("WiFi Connected Ip: " + WiFi.localIP().toString());
    }
    else
    {
        Serial.println("WiFi connection failed.");
        drawHeader("WiFi Connection Failed!");
    }
}
