#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class Account
{
public:
    int64_t id;
    String iban;
    String type;
    float balance;
    float absoluteLimit;
    float dailyLimit;
    int64_t ownerId;

    // Default constructor
    Account() {}

    // Manual constructor
    Account(const String &type, const String &iban, float balance)
        : type(type), iban(iban), balance(balance) {}

    // JSON string constructor
    Account(const String &json)
    {
        fromJson(json);
    }

    // JsonObject constructor (no extra parsing needed)
    Account(const JsonObject &obj)
    {
        fromJsonObject(obj);
    }

private:
    void fromJsonObject(const JsonObject &obj)
    {
        // Map JSON → class fields
        id = obj["id"] | 0;
        iban = obj["iban"] | "";
        type = obj["type"] | "";
        balance = obj["balance"] | 0.0;
        absoluteLimit = obj["absoluteLimit"] | 0.0;
        dailyLimit = obj["dailyLimit"] | 0.0;
        ownerId = obj["ownerId"] | 0;
    }

    void fromJson(const String &json)
    {
        StaticJsonDocument<512> doc;

        DeserializationError error = deserializeJson(doc, json);

        if (error)
        {
            Serial.print("Account JSON parse failed: ");
            Serial.println(error.c_str());
            return;
        }

        fromJsonObject(doc.as<JsonObject>());
    }

public:
    void updateAccountBalance(float newBalance)
    {
        balance = newBalance;
    }
};