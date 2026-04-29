#include "TransactionApi.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <User.h>
#include <Account.h>

const String API_URL = "https://bookswap.art/api";
extern User currentUser;
extern Account currentAccount;

bool postTransaction(
    TransactionType type,
    float amount,
    const String &jwtToken,
    TransactionResponse &out)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        out.success = false;
        out.message = "No WiFi";
        return false;
    }

    HTTPClient http;
    String url = API_URL + "/transactions";
    http.begin(url);

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + jwtToken);

    StaticJsonDocument<256> requestDoc;
    if (type == TransactionType::Withdraw)
    {
        requestDoc["fromIban"] = currentAccount.iban;
    }
    else
    {
        requestDoc["fromIban"].set(nullptr);
    }
    if (type == TransactionType::Deposit)
    {
        requestDoc["toIban"] = currentAccount.iban;
    }
    else
    {
        requestDoc["toIban"].set(nullptr);
    }
    requestDoc["amount"] = amount;
    requestDoc["type"] = type == TransactionType::Withdraw
                             ? "WITHDRAWAL"
                             : "DEPOSIT";

    String body;
    serializeJson(requestDoc, body);

    int httpCode = http.POST(body);

    if (httpCode <= 0)
    {
        Serial.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }

    if (httpCode != HTTP_CODE_OK && httpCode != 201)
    {
        Serial.printf("Unexpected HTTP code: %d\n", httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    Serial.println("API response:");
    Serial.println(payload);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    // Check if transaction was created (id field exists)
    if (doc.containsKey("id"))
    {
        out.success = true;
        out.message = "Transaction successful";
        currentAccount.updateAccountBalance(getAccountBalance(currentAccount.id));
        out.newBalance = currentAccount.balance;
        return true;
    }
    else
    {
        out.success = false;
        out.message = doc["message"] | "Transaction failed";
        out.newBalance = currentAccount.balance;
        return false;
    }
}

bool loginToApi(const String &email, const String &password)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected.");
        return false;
    }

    HTTPClient http;
    http.begin(String(API_URL) + "/auth/login"); // your login endpoint
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> requestDoc;
    requestDoc["email"] = email;
    requestDoc["password"] = password;

    String requestBody;
    serializeJson(requestDoc, requestBody);

    int httpCode = http.POST(requestBody);

    String response = http.getString();

    Serial.printf("HTTP code: %d\n", httpCode);
    Serial.println(response);

    http.end();

    if (httpCode != 200 && httpCode != 201)
    {
        Serial.println("Login failed.");
        return false;
    }

    StaticJsonDocument<1024> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);

    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }
    currentUser = User(response); // Initialize currentUser with the response JSON

    if (currentUser.token.length() == 0)
    {
        Serial.println("No JWT token found in response.");
        return false;
    }

    Serial.println("Login successful.");
    Serial.println("User: " + currentUser.fullName());
    Serial.println("JWT saved globally.");

    return !currentUser.token.isEmpty();
}

bool fetchUserAccounts(User &user)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected.");
        return false;
    }

    HTTPClient http;
    http.begin(String(API_URL) + "/accounts/me");

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + user.token);

    int httpCode = http.GET();
    String response = http.getString();

    Serial.printf("HTTP code: %d\n", httpCode);
    Serial.println(response);

    http.end();

    if (httpCode != 200)
    {
        Serial.println("Failed to fetch accounts.");
        return false;
    }

    // Parse JSON array of accounts
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    user.accounts.clear();

    // Assume response is an array of account objects
    if (doc.is<JsonArray>())
    {
        JsonArray arr = doc.as<JsonArray>();
        for (JsonObject obj : arr)
        {
            Serial.println("Parsing account:");
            user.accounts.push_back(Account(obj));
            Serial.println("Account IBAN: " + user.accounts.back().iban);
        }
    }

    return user.accounts.size() > 0;
}
float getAccountBalance(const int accountId)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected.");
        return 0.0;
    }

    HTTPClient http;
    http.begin(String(API_URL) + "/accounts/" + String(accountId));

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + currentUser.token);

    int httpCode = http.GET();

    String response = http.getString();

    Serial.printf("HTTP code: %d\n", httpCode);
    Serial.println(response);

    http.end();

    if (httpCode != 200)
    {
        Serial.println("Failed to fetch account balance.");
        return 0.0;
    }

    // Parse the JSON response to extract the balance
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return 0.0;
    }

    return doc["balance"] | 0.0;
}
