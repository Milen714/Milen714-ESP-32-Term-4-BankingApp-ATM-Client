#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <AsyncTCP.h>
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

User currentUser;               // Global variable to hold the current user information
Account currentAccount;         // Global variable to hold the currently selected account
std::vector<User> usersTologin; // For testing purposes

float balance = 250.00;
enum class Screen
{
  ActionSelection,
  Amount,
  Loading,
  Result,
  AccountSelection,
  UserSelection
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

void showUserSelection()
{
  currentScreen = Screen::UserSelection;
  selectedIndex = 0;
  drawUserAccontsSelectionScreen(selectedIndex);
}
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
  drawAccountSelectionScreen(selectedIndex);
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
  else if (currentScreen == Screen::UserSelection)
  {
    selectedIndex--;

    if (selectedIndex < 0)
    {
      selectedIndex = usersTologin.size() - 1;
    }

    drawUserAccontsSelectionScreen(selectedIndex);
  }

  else if (currentScreen == Screen::AccountSelection)
  {
    selectedIndex--;

    if (selectedIndex < 0)
    {
      selectedIndex = currentUser.accounts.size() - 1;
    }

    drawAccountSelectionScreen(selectedIndex);
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
  else if (currentScreen == Screen::UserSelection)
  {
    selectedIndex++;

    if (selectedIndex >= usersTologin.size())
    {
      selectedIndex = 0;
    }

    drawUserAccontsSelectionScreen(selectedIndex);
  }

  else if (currentScreen == Screen::AccountSelection)
  {
    selectedIndex++;

    if (selectedIndex >= currentUser.accounts.size())
    {
      selectedIndex = 0;
    }

    drawAccountSelectionScreen(selectedIndex);
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
  else if (currentScreen == Screen::UserSelection)
  {
    try
    {
      drawLoadingScreen("Logging in...");
      currentUser = usersTologin[selectedIndex];
      bool loginSuccess = loginToApi(usersTologin[selectedIndex].email, usersTologin[selectedIndex].password);
      if (loginSuccess)
      {
        fetchUserAccounts(currentUser);
        showAccountSelection();
      }
      else
      {
        drawResultScreen("Login Failed", "Invalid credentials", false);
      }
    }
    catch (const std::exception &e)
    {
      drawResultScreen("Login Failed", String(e.what()), false);
    }
  }

  else if (currentScreen == Screen::AccountSelection)
  {
    if (selectedIndex < currentUser.accounts.size())
    {
      currentAccount = currentUser.accounts[selectedIndex];
      showActionSelection();
    }
    else if (currentUser.accounts.size() == 0)
    {
      showUserSelection();
    }
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
    showUserSelection();
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

  // Initialize test users
  usersTologin.push_back(User("admin@bank.nl", "password"));
  usersTologin.push_back(User("john@bank.nl", "password"));
  usersTologin.push_back(User("jane@bank.nl", "password"));
  usersTologin.push_back(User("karen@bank.nl", "password"));
  usersTologin.push_back(User("joe@bank.nl", "password"));

  connectWiFi(WIFI_SSID, WIFI_PASSWORD);

  setupWebServer();

  showUserSelection();
  // loginToApi("john@bank.nl", "password");

  // showActionSelection();
  // fetchUserAccounts(currentUser);
  // Serial.println("User accounts fetched: " + String(currentUser.accounts.size()));
  // showAccountSelection();
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