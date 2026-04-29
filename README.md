# ESP32 Mock ATM Client

A portable ATM client built on an ESP32 microcontroller with a small TFT LCD display and 3-button navigation interface. This project connects to a banking API to provide core ATM functionality including account selection, balance viewing, and transaction processing (deposits/withdrawals).

## Hardware Requirements

- **Microcontroller:** ESP32 (ESP-DevKit)
- **Display:** Adafruit ST7735 1.8" TFT LCD (128x160 pixels)
- **Buttons:** 3x Push buttons for navigation
  - Button 1 (GPIO 32): Previous/Up navigation
  - Button 2 (GPIO 33): Next/Down navigation
  - Button 3 (GPIO 26): Select/Confirm action
- **Backlight:** LED (GPIO 25, active LOW)

## Features

### Current Functionality

- ✅ WiFi connectivity with automatic connection on boot
- ✅ User authentication via banking API (JWT-based)
- ✅ Multi-account support with pagination (2 accounts per screen)
- ✅ Real-time balance display
- ✅ Deposit & Withdrawal transactions
- ✅ Transaction confirmation screens with success/failure feedback
- ✅ Web server endpoint showing currently logged-in user
- ✅ Color-coded UI with custom display themes

### Upcoming Features

- 🔄 Transaction history viewing
- 🔄 Pin-protected access
- 🔄 Admin/Manager mode
- 🔄 Receipt printing support
- 🔄 Card reader integration
- 🔄 NFC payment support

## Getting Started

### Prerequisites

- PlatformIO installed (VSCode extension or CLI)
- ESP32 board with USB cable
- Required libraries (auto-installed by PlatformIO):
  - Arduino Framework
  - Adafruit ST7735 & GFX
  - AsyncTCP
  - ESPAsyncWebServer
  - ArduinoJson

### Installation

1. **Clone/Download the project:**

   ```bash
   git clone <repository-url>
   cd ATM-Esp
   ```

2. **Configure WiFi credentials** in `src/main.cpp`:

   ```cpp
   const char *WIFI_SSID = "YOUR_SSID";
   const char *WIFI_PASSWORD = "YOUR_PASSWORD";
   ```

3. **Update API endpoint** in `src/TransactionApi.cpp` if needed:

   ```cpp
   const String API_URL = "https://your-api-endpoint.com/api";
   ```

4. **Build and upload:**

   ```bash
   platformio run --target upload --upload-port COM6
   ```

   _(Replace COM6 with your ESP32 port)_

5. **Monitor serial output:**
   ```bash
   platformio device monitor --port COM6
   ```

## Usage

### Navigation Flow

1. **Boot:** Device connects to WiFi and automatically logs in with demo credentials
2. **Account Selection:** Use Prev/Next buttons to navigate through accounts, press Select to choose
3. **Action Selection:** Choose between Deposit or Withdraw
4. **Amount Selection:** Select from predefined amounts (€10, €20, €50, €100)
5. **Processing:** Transaction is sent to the API
6. **Result:** Success or failure screen with updated balance

### Button Controls

| Button     | Home/Action     | Amount          | Account               | Result      |
| ---------- | --------------- | --------------- | --------------------- | ----------- |
| **Prev**   | Previous option | Previous amount | Previous account page | N/A         |
| **Next**   | Next option     | Next amount     | Next account page     | N/A         |
| **Select** | Confirm action  | Confirm amount  | Confirm account       | Return home |

## Project Structure

```
ATM-Esp/
├── src/
│   ├── main.cpp              # Main program & state machine
│   ├── display.cpp/h         # TFT display rendering functions
│   ├── WiFiManager.cpp/h     # WiFi connectivity
│   ├── TransactionApi.cpp/h  # API communication (login, transactions, accounts)
│   ├── webserver.cpp/h       # Simple HTTP server
│   ├── Button.cpp/h          # Button debouncing logic
│   ├── DisplayColor.h        # Color palette enum (RGB565)
│   ├── User.h                # User class (email, accounts, JWT)
│   └── Account.h             # Account class (IBAN, balance, type)
├── platformio.ini            # PlatformIO configuration
└── README.md                 # This file
```

## Architecture

### State Machine

The application uses a finite state machine to manage the user flow:

- `ActionSelection`: Choose Deposit or Withdraw
- `Amount`: Select transaction amount
- `AccountSelection`: Select which account to use
- `Loading`: Processing transaction
- `Result`: Display transaction outcome

### API Integration

Communication with the banking API uses:

- **Auth:** `POST /auth/login` - User authentication (returns JWT)
- **Accounts:** `GET /accounts/me` - Fetch user's accounts
- **Transactions:** `POST /transactions` - Submit deposit/withdrawal

### Display

- 128x160 pixel TFT display
- Custom button rendering with selection highlighting
- Real-time account balance display
- Pagination support for multiple accounts

## Configuration

Edit `src/main.cpp` to customize:

- WiFi credentials
- Demo login credentials
- Default account selection
- Transaction amounts

## Testing

For testing without a real backend, responses are mocked in demo mode. Update `TransactionApi.cpp` to point to your backend API endpoints.

## Troubleshooting

**WiFi not connecting:**

- Verify SSID/password in `main.cpp`
- Check serial monitor for connection details
- Ensure ESP32 is within range of the router

**Display not showing:**

- Check TFT_CS, TFT_DC, TFT_RST pin connections
- Verify SPI pins (GPIO 18/19 for MOSI/MISO)
- Check backlight pin (GPIO 25) is enabled

**API errors:**

- Verify API endpoint URL in `TransactionApi.cpp`
- Check JWT token is valid
- Monitor serial output for detailed error messages

---

**Status:** Alpha - In Active Development  
**Last Updated:** April 29, 2026
