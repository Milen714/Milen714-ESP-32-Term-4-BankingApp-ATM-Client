#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Account.h>
#include <vector>

class User
{
public:
    int64_t id;
    String firstName;
    String lastName;
    String email;
    String role;
    String phoneNumber;
    String token; // JWT
    std::vector<Account> accounts;

    // Default constructor
    User() {}

    // JSON constructor
    User(const String &json)
    {
        fromJson(json);
    }

private:
    void fromJson(const String &json)
    {
        StaticJsonDocument<1024> doc;

        DeserializationError error = deserializeJson(doc, json);

        if (error)
        {
            Serial.print("User JSON parse failed: ");
            Serial.println(error.c_str());
            return;
        }

        id = doc["id"] | 0;
        firstName = doc["firstName"] | "";
        lastName = doc["lastName"] | "";
        email = doc["email"] | "";
        role = doc["role"] | "";
        phoneNumber = doc["phoneNumber"] | "";
        token = doc["token"] | "";
    }

public:
    String fullName() const
    {
        return firstName + " " + lastName;
    }
};