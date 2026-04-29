#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <TFT_eSPI.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_ST7735.h>
#include <SPI.h>
#include "webserver.h"
#include "display.h"
#include "DisplayColor.h"
#include "Button.h"
#include "TransactionApi.h"
#include "User.h"
#include "Account.h"
#include "WiFiManager.h"
#include "WiFiConfig.h"

// ---------- API ----------
const char *API_URL = "https://bookswap.art/api";

// Pin definitions
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 17
#define TFT_BL 25 // active LOW

// button pins
#define BTN_PREV 32
#define BTN_NEXT 33
#define BTN_SELECT 26

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

Button btnPrev(BTN_PREV);
Button btnNext(BTN_NEXT);
Button btnSelect(BTN_SELECT);

AsyncWebServer server(80);

User currentUser;       // Global variable to hold the current user information
Account currentAccount; // Global variable to hold the currently selected account
String lastTitle = "Waiting...";
String lastValue = "No data yet";
String lastUpdated = "Never";

float balance = 250.00;

enum class Screen
{
  ActionSelection,
  Amount,
  Loading,
  Result,
  AccountSelection
};

enum class AtmAction
{
  None,
  Withdraw,
  Deposit
};

Screen currentScreen = Screen::ActionSelection;
AtmAction currentAction = AtmAction::None;

int selectedIndex = 0;

const int amountOptions[] = {10, 20, 50, 100};
const int amountCount = sizeof(amountOptions) / sizeof(amountOptions[0]);

bool lastTransactionSuccess = false;
String lastResultMessage = "";

void showActionSelection()
{
  currentScreen = Screen::ActionSelection;
  currentAction = AtmAction::None;
  selectedIndex = 0;
  drawActionSelectionScreen(selectedIndex);
}

void showAccountSelection()
{
  currentScreen = Screen::AccountSelection;
  selectedIndex = 0;
  drawAccountSelectionScreen(selectedIndex, 0);
}

void showAmountScreen(AtmAction action)
{
  currentScreen = Screen::Amount;
  currentAction = action;
  selectedIndex = 0;

  String title = action == AtmAction::Withdraw
                     ? "Withdraw"
                     : "Deposit";

  drawAmountScreen(title, selectedIndex);
}

void movePrevious()
{
  if (currentScreen == Screen::ActionSelection)
  {
    selectedIndex--;

    if (selectedIndex < 0)
    {
      selectedIndex = 1;
    }

    drawActionSelectionScreen(selectedIndex);
  }

  else if (currentScreen == Screen::AccountSelection)
  {
    selectedIndex--;

    if (selectedIndex < 0)
    {
      selectedIndex = currentUser.accounts.size() - 1;
    }

    drawAccountSelectionScreen(selectedIndex, 0);
  }

  else if (currentScreen == Screen::Amount)
  {
    selectedIndex--;

    if (selectedIndex < 0)
    {
      selectedIndex = amountCount - 1; // Wrap around to last amount option
      // amountCount is the Back option
    }

    String title = currentAction == AtmAction::Withdraw
                       ? "Withdraw"
                       : "Deposit";

    drawAmountScreen(title, selectedIndex);
  }
}

void moveNext()
{
  if (currentScreen == Screen::ActionSelection)
  {
    selectedIndex++;

    if (selectedIndex > 1)
    {
      selectedIndex = 0;
    }

    drawActionSelectionScreen(selectedIndex);
  }

  else if (currentScreen == Screen::AccountSelection)
  {
    selectedIndex++;

    if (selectedIndex >= currentUser.accounts.size())
    {
      selectedIndex = 0;
    }

    drawAccountSelectionScreen(selectedIndex, 0);
  }

  else if (currentScreen == Screen::Amount)
  {
    selectedIndex++;

    if (selectedIndex > amountCount)
    {
      selectedIndex = 0;
    }

    String title = currentAction == AtmAction::Withdraw
                       ? "Withdraw"
                       : "Deposit";

    drawAmountScreen(title, selectedIndex);
  }
}

void performTransaction(float amount)
{
  currentScreen = Screen::Loading;
  drawLoadingScreen("Sending request...");

  TransactionResponse response;

  TransactionType type = currentAction == AtmAction::Withdraw
                             ? TransactionType::Withdraw
                             : TransactionType::Deposit;

  bool ok = postTransaction(type, amount, currentUser.token, response);

  lastTransactionSuccess = ok && response.success;
  lastResultMessage = response.message;

  if (lastTransactionSuccess)
  {
    balance = response.newBalance;
  }

  currentScreen = Screen::Result;

  drawResultScreen(
      lastTransactionSuccess ? "Success" : "Failed",
      lastResultMessage,
      lastTransactionSuccess);
}

void handleSelect()
{
  if (currentScreen == Screen::ActionSelection)
  {
    if (selectedIndex == 0)
    {
      showAmountScreen(AtmAction::Withdraw);
    }
    else if (selectedIndex == 1)
    {
      showAmountScreen(AtmAction::Deposit);
    }
    // to get out of the home screen
    else if (selectedIndex == 2)
    {
      showAccountSelection();
    }
  }

  else if (currentScreen == Screen::AccountSelection)
  {
    currentAccount = currentUser.accounts[selectedIndex];
    showActionSelection();
  }

  else if (currentScreen == Screen::Amount)
  {
    if (selectedIndex == amountCount)
    {
      showActionSelection();
      return;
    }

    int amount = amountOptions[selectedIndex];
    performTransaction(amount);
  }

  else if (currentScreen == Screen::Result)
  {
    showAccountSelection();
  }
}

// ---------- Main ----------
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting ESP32 ePaper project...");

  enableBacklight();

  btnPrev.begin();
  btnNext.begin();
  btnSelect.begin();

  tftSetup();

  connectWiFi(WIFI_SSID, WIFI_PASSWORD);

  setupWebServer();

  loginToApi("john@bank.nl", "password");

  // showActionSelection();
  fetchUserAccounts(currentUser);
  Serial.println("User accounts fetched: " + String(currentUser.accounts.size()));
  showAccountSelection();
}

void enableBacklight()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW); // ON because active LOW
}

void loop()
{
  if (btnPrev.wasPressed())
  {
    movePrevious();
  }

  if (btnNext.wasPressed())
  {
    moveNext();
  }

  if (btnSelect.wasPressed())
  {
    handleSelect();
  }

  delay(5);
}