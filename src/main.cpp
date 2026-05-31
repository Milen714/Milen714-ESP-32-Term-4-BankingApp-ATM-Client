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
#include "CardReader.h"
#include "Servo.h"

#include <Wire.h>
#include <Adafruit_PN532.h>
#include <ESP32Servo.h>

// Pin definitions
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 17
#define TFT_BL 25 // active LOW

#define PN532_SDA 21
#define PN532_SCL 22

#define SERVO_PIN 27

// button pins
#define BTN_PREV 32
#define BTN_NEXT 33
#define BTN_SELECT 26

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// Adafruit_PN532 nfc(PN532_SDA, PN532_SCL);

Servo atmServo;

Button btnPrev(BTN_PREV);
Button btnNext(BTN_NEXT);
Button btnSelect(BTN_SELECT);

AsyncWebServer server(80);

User currentUser;               // Global variable to hold the current user information
Account currentAccount;         // Global variable to hold the currently selected account
std::vector<User> usersTologin; // For testing purposes

float balance = 250.00;
int selectedAmmount = 0;
bool cardOperationInProgress = false;
enum class Screen
{
  CardLogin,
  ActionSelection,
  Amount,
  Loading,
  Result,
  AccountSelection,
  UserSelection,
  PinEntry
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

String cardPin = "";
String enteredPin = "";
int currentDigitIndex = 0;

void showCardloginScreen()
{
  currentScreen = Screen::CardLogin;
  selectedIndex = 0;
  drawCardLoginScreen(selectedIndex);
}
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

void showPinEntryScreen()
{
  currentScreen = Screen::PinEntry;
  selectedIndex = 0;
  currentDigitIndex = 0;
  enteredPin = "0000";
  drawPinEntryScreen(selectedIndex);
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
  else if (currentScreen == Screen::CardLogin)
  {
    selectedIndex--;

    if (selectedIndex < 0)
    {
      selectedIndex = 0; // Only one option on card login screen
    }

    showUserSelection();
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
  else if (currentScreen == Screen::PinEntry)
  {
    if (selectedIndex < 4)
    {
      // Decrement digit value (0 -> 9 wraps)
      int digit = enteredPin[selectedIndex] - '0';
      digit = (digit - 1 + 10) % 10;
      enteredPin[selectedIndex] = '0' + digit;
    }
    else if (selectedIndex == 4)
    {
      // Wrap from Next button to last digit
      selectedIndex = 3;
    }

    drawPinEntryScreen(selectedIndex);
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
  else if (currentScreen == Screen::PinEntry)
  {
    if (selectedIndex < 4)
    {
      // Increment digit value (9 -> 0 wraps)
      int digit = enteredPin[selectedIndex] - '0';
      digit = (digit + 1) % 10;
      enteredPin[selectedIndex] = '0' + digit;
    }
    else if (selectedIndex == 4)
    {
      // Wrap from Next button to first digit
      selectedIndex = 0;
    }

    drawPinEntryScreen(selectedIndex);
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
  if (lastTransactionSuccess)
  {
    openAtmDoor();
    delay(5000); // Keep vault open for 5 seconds
    closeAtmDoor();
  }
}
bool cardPinIsValid(User user = currentUser)
{
  return enteredPin == user.pin;
}
void handleLogin(User &user)
{
  drawLoadingScreen("Logging in...");

  bool loginSuccess = loginToApi(user.email, user.password, user.pin);

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
  else if (currentScreen == Screen::CardLogin)
  {
    if (selectedIndex == 0)
    {
      cardOperationInProgress = true;
      drawLoadingScreen("Reading card...");
      String jsonData = readDataMultipleBlocks();
      Serial.println("Read JSON from card: ");
      Serial.println(jsonData);
      User userFromCard = jsonToUserData(jsonData);
      Serial.println("Parsed user data from JSON:");
      Serial.println("Email: " + userFromCard.email);
      if (userFromCard.email.length() == 0 || userFromCard.password.length() == 0)
      {
        drawResultScreen("Login Failed", "No valid user data found on card", false);
        cardOperationInProgress = false;
        delay(2000); // Show error message for 2 seconds
        showCardloginScreen();
        return;
      }
      drawResultScreen("Welcome", "Email: " + userFromCard.email, true);
      delay(2000); // Show welcome message for 2 seconds
      handleLogin(userFromCard);
      cardOperationInProgress = false;
      delay(100); // Small delay to prevent rapid re-triggering
    }
  }
  else if (currentScreen == Screen::UserSelection)
  {
    try
    {
      drawLoadingScreen("Logging in...");
      currentUser = usersTologin[selectedIndex];
      bool loginSuccess = loginToApi(usersTologin[selectedIndex].email, usersTologin[selectedIndex].password, usersTologin[selectedIndex].pin);
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

    // int amount = amountOptions[selectedIndex];
    // performTransaction(amount);
    selectedAmmount = amountOptions[selectedIndex];
    showPinEntryScreen();
  }

  else if (currentScreen == Screen::Result)
  {
    showUserSelection();
  }
  else if (currentScreen == Screen::PinEntry)
  {
    Serial.println("Selected Index: " + String(selectedIndex));
    if (selectedIndex < 4)
    {
      // Move to next digit position
      selectedIndex++;
      if (selectedIndex > 3)
      {
        selectedIndex = 4; // Move to Next button
      }
      if (selectedIndex < 4)
      {
        currentDigitIndex = selectedIndex;
      }
    }
    else if (selectedIndex == 4)
    {
      // Next button selected - ready for screen transition
      // User will handle this after PIN validation
      Serial.println("PIN entered: " + enteredPin);
      if (cardPinIsValid())
      {
        Serial.println("PIN is valid. Proceeding with transaction. Entered PIN: " + enteredPin + ", Expected PIN: " + currentUser.pin + "Selected amount: " + String(selectedAmmount));
        performTransaction(selectedAmmount);
      }
      else
      {
        Serial.println("PIN is invalid.");
        drawResultScreen("Invalid PIN", "Please try again", false);
        delay(4000); // Show error message for 4 seconds
        drawPinEntryScreen(selectedIndex);
      }
    }

    drawPinEntryScreen(selectedIndex);
  }
}

void enableBacklight()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW); // ON because active LOW
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

  // servo setup
  setupServo();

  setupCardReader();

  // Initialize test users
  usersTologin.push_back(User("admin@bank.nl", "password", "1234"));
  usersTologin.push_back(User("john@bank.nl", "password", "1234"));
  usersTologin.push_back(User("jane@bank.nl", "password", "9012"));
  usersTologin.push_back(User("karen@bank.nl", "password", "3456"));
  usersTologin.push_back(User("joe@bank.nl", "password", "7890"));

  connectWiFi(WIFI_SSID, WIFI_PASSWORD);

  setupWebServer();

  showCardloginScreen();
  // showPinEntryScreen();
  // showUserSelection();
  // loginToApi("john@bank.nl", "password");

  // showActionSelection();
  // fetchUserAccounts(currentUser);
  // Serial.println("User accounts fetched: " + String(currentUser.accounts.size()));
  // showAccountSelection();
}

void loop()
{
  if (Serial.available() > 0 && !cardOperationInProgress)
  {
    char firstChar = Serial.peek();
    if (firstChar == 'w' || firstChar == 'W')
    {
      // Consume the input
      while (Serial.available() > 0)
      {
        Serial.read();
      }

      cardOperationInProgress = true;
      User userToWrite = usersTologin[1];
      String jsonData = userDataToJson(userToWrite);
      Serial.println("Writing JSON to card: ");
      Serial.println(jsonData);
      writeDataMultipleBlocks(jsonData);
      cardOperationInProgress = false;
      delay(100); // Small delay to prevent rapid re-triggering
    }
    else if (firstChar == 'r' || firstChar == 'R')
    {
      // Consume the input
      while (Serial.available() > 0)
      {
        Serial.read();
      }

      cardOperationInProgress = true;
      String jsonData = readDataMultipleBlocks();
      Serial.println("Read JSON from card: ");
      Serial.println(jsonData);
      User userFromCard = jsonToUserData(jsonData);
      Serial.println("Parsed user data from JSON:");
      Serial.println("Email: " + userFromCard.email);
      Serial.println("Password: " + userFromCard.password);
      Serial.println("PIN: " + userFromCard.pin);
      cardOperationInProgress = false;
      delay(100); // Small delay to prevent rapid re-triggering
    }
    else
    {
      handleServoSerialInput();
    }
  }

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