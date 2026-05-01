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
    String password; // I will be defining a few hardcoded users for testing, so I will store the password here in plaintext. In a real application, this should never be done.
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

    // Login Profile constructor
    User(const String &email, const String &password)
    {

        this->email = email;
        this->password = password;
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