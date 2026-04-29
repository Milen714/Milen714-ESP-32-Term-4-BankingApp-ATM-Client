#include "display.h"
#include "DisplayColor.h"
#include "Account.h"
#include "User.h"

extern Adafruit_ST7735 tft;
extern User currentUser;
extern Account currentAccount;

void tftSetup()
{
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(colorValue(DisplayColor::Black));
}

void drawHeader(const String &content)
{
    tft.fillRect(0, 0, tft.width(), 28, colorValue(DisplayColor::Black));

    tft.setCursor(8, 9);
    tft.setTextColor(colorValue(DisplayColor::White));
    tft.setTextSize(1);
    tft.print(content);

    tft.drawFastHLine(0, 28, tft.width(), colorValue(DisplayColor::White));
}

void drawMenuButton(int x, int y, int w, int h, const String &label, bool selected)
{
    uint16_t border = selected
                          ? colorValue(DisplayColor::Yellow)
                          : colorValue(DisplayColor::White);

    uint16_t fill = selected
                        ? colorValue(DisplayColor::Blue)
                        : colorValue(DisplayColor::Black);

    uint16_t text = selected
                        ? colorValue(DisplayColor::White)
                        : colorValue(DisplayColor::Cyan);

    tft.fillRoundRect(x, y, w, h, 5, fill);
    tft.drawRoundRect(x, y, w, h, 5, border);

    tft.setCursor(x + 8, y + 10);
    tft.setTextSize(1);
    tft.setTextColor(text);
    tft.print(label);
}

void drawAmountButton(int x, int y, int w, int h, int amount, bool selected)
{
    String label = "EUR " + String(amount);
    drawMenuButton(x, y, w, h, label, selected);
}

void drawActionSelectionScreen(int selectedIndex)
{
    tft.fillScreen(colorValue(DisplayColor::Black));
    drawHeader("Select Action");

    tft.setCursor(8, 42);
    tft.setTextColor(colorValue(DisplayColor::Green));
    tft.setTextSize(1);
    tft.print("Balance: EUR ");
    tft.print(currentAccount.balance, 2);

    drawMenuButton(10, 55, 140, 30, "Withdraw", selectedIndex == 0);
    drawMenuButton(10, 85, 140, 30, "Deposit", selectedIndex == 1);

    tft.setCursor(8, 150);
    tft.setTextColor(colorValue(DisplayColor::White));
    tft.print("Prev/Next + Select");
}

void drawAmountScreen(const String &mode, int selectedIndex)
{
    tft.fillScreen(colorValue(DisplayColor::Black));
    drawHeader(mode + " - " + currentAccount.type);

    tft.setCursor(8, 38);
    tft.setTextColor(colorValue(DisplayColor::Green));
    tft.setTextSize(1);
    tft.print("Balance: EUR ");
    tft.print(currentAccount.balance, 2);

    drawAmountButton(8, 60, 68, 28, 10, selectedIndex == 0);
    drawAmountButton(84, 60, 68, 28, 20, selectedIndex == 1);

    drawAmountButton(8, 95, 68, 28, 50, selectedIndex == 2);
    drawAmountButton(84, 95, 68, 28, 100, selectedIndex == 3);

    drawMenuButton(8, 130, 144, 24, "Back", selectedIndex == 4);
}

void drawLoadingScreen(const String &message)
{
    tft.fillScreen(colorValue(DisplayColor::Black));
    drawHeader("Processing");

    tft.setCursor(15, 75);
    tft.setTextColor(colorValue(DisplayColor::Yellow));
    tft.setTextSize(1);
    tft.print(message);
}

void drawResultScreen(const String &title, const String &message, bool success)
{
    tft.fillScreen(colorValue(DisplayColor::Black));
    drawHeader(title);

    tft.setCursor(10, 55);
    tft.setTextColor(success ? colorValue(DisplayColor::Green) : colorValue(DisplayColor::Red));
    tft.setTextSize(1);
    tft.println(message);

    tft.setCursor(10, 95);
    tft.setTextColor(colorValue(DisplayColor::White));
    tft.print("Balance: EUR ");
    tft.print(currentAccount.balance, 2);

    tft.setCursor(10, 140);
    tft.setTextColor(colorValue(DisplayColor::Cyan));
    tft.print("Select = Home");
}
void drawAccountSelectionScreen(int selectedIndex, int pageOffset)
{
    tft.fillScreen(colorValue(DisplayColor::Black));
    drawHeader("Select Account");

    const int accountsPerPage = 2;
    const int totalAccounts = currentUser.accounts.size();
    const int maxPages = (totalAccounts + accountsPerPage - 1) / accountsPerPage;

    // Display up to 2 accounts on this page
    for (int i = 0; i < accountsPerPage && (pageOffset + i) < totalAccounts; i++)
    {
        int accountIndex = pageOffset + i;
        int yPos = 50 + i * 40;
        drawMenuButton(10, yPos, 140, 30, currentUser.accounts[accountIndex].type, selectedIndex == i);
    }

    // Show page info if there are multiple pages
    if (maxPages > 1)
    {
        int currentPage = pageOffset / accountsPerPage + 1;
        tft.setCursor(8, 130);
        tft.setTextColor(colorValue(DisplayColor::Cyan));
        tft.setTextSize(1);
        tft.print("Page ");
        tft.print(currentPage);
        tft.print("/");
        tft.print(maxPages);
    }

    tft.setCursor(8, 150);
    tft.setTextColor(colorValue(DisplayColor::White));
    tft.print("Prev/Next + Select");
}