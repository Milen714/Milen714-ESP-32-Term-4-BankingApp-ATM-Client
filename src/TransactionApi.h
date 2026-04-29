#pragma once

#include <Arduino.h>
#include <User.h>
#include <Account.h>
#include <vector>

enum class TransactionType
{
    Withdraw,
    Deposit
};

struct TransactionResponse
{
    bool success;
    String message;
    float newBalance;
};

bool postTransaction(
    TransactionType type,
    float amount,
    const String &jwtToken,
    TransactionResponse &out);

bool loginToApi(const String &email, const String &password);

bool fetchUserAccounts(User &currentUser);
float getAccountBalance(const int accountId);