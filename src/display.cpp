#include "display.h"
#include "DisplayColor.h"
#include "Account.h"
#include "User.h"

extern Adafruit_ST7735 tft;
extern User currentUser;
extern Account currentAccount;
extern std::vector<User> usersTologin;

PaginationInfo calculatePagination(int selectedIndex, int itemsPerPage, int totalItems)
{
    PaginationInfo info;
    info.pageOffset = (selectedIndex / itemsPerPage) * itemsPerPage;
    info.maxPages = (totalItems + itemsPerPage - 1) / itemsPerPage;
    info.currentPage = (selectedIndex / itemsPerPage) + 1;
    return info;
}

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
void drawFooter(String content)
{
    tft.setCursor(45, tft.height() - 10);
    tft.setTextColor(colorValue(DisplayColor::White));
    tft.print(content);
}
void drawPageInfo(int maxPages, const int startY, int drawnItems, const int buttonHeight, const int spacing, int currentPage)
{
    if (maxPages > 1)
    {
        int lastButtonY = startY + (drawnItems - 1) * (buttonHeight + spacing);
        int pageTextY = lastButtonY + buttonHeight + 5;

        tft.setCursor(8, pageTextY);
        tft.setTextColor(colorValue(DisplayColor::Cyan));
        tft.setTextSize(1);
        tft.print("Page ");
        tft.print(currentPage);
        tft.print("/");
        tft.print(maxPages);
    }
}
void drawPageInfo(int currentPage, int maxPages)
{
    String pageInfo = "Page " + String(currentPage) + "/" + String(maxPages);
    int textWidth = pageInfo.length() * 6; // Approximate width of text
    int x = tft.width() - textWidth - 8;
    int y = 9;

    tft.setCursor(x, y);
    tft.setTextColor(colorValue(DisplayColor::White));
    tft.setTextSize(1);
    tft.print(pageInfo);
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

    drawFooter("Select to return to menu");
}
void drawAccountSelectionScreen(int selectedIndex)
{
    tft.fillScreen(colorValue(DisplayColor::Black));
    drawHeader("Select Account");

    const int accountsPerPage = 2;
    const int totalAccounts = currentUser.accounts.size();
    const int buttonHeight = 30;
    const int spacing = 5;
    const int startY = 38;

    PaginationInfo pagination = calculatePagination(selectedIndex, accountsPerPage, totalAccounts);

    int drawnItems = 0;

    if (totalAccounts > 0)
    {
        for (int i = 0; i < accountsPerPage && (pagination.pageOffset + i) < totalAccounts; i++)
        {
            int accountIndex = pagination.pageOffset + i;
            int yPos = startY + i * (buttonHeight + spacing);

            drawMenuButton(
                10, yPos, 140, buttonHeight,
                currentUser.accounts[accountIndex].type + " (" +
                    String(currentUser.accounts[accountIndex].balance) + " EUR)",
                selectedIndex == accountIndex);

            drawnItems++;
        }

        drawPageInfo(pagination.maxPages, startY, drawnItems, buttonHeight, spacing, pagination.currentPage);

        drawFooter("Prev/Next + Select");
    }
    else
    {
        tft.setCursor(10, 60);
        tft.setTextColor(colorValue(DisplayColor::Red));
        tft.setTextSize(1);
        tft.print("No accounts available");
        drawFooter("Select to return");
    }
}

void drawUserAccontsSelectionScreen(int selectedIndex)
{
    tft.fillScreen(colorValue(DisplayColor::Black));
    drawHeader("Select User");

    // For simplicity, we will just show the current user's email as the only option
    // drawMenuButton(10, 60, 140, 30, currentUser.email, selectedIndex == 0);
    const int accountsPerPage = 2;
    const int totalUserAccounts = usersTologin.size();
    const int buttonHeight = 30;
    const int spacing = 5;
    const int startY = 38;

    PaginationInfo pagination = calculatePagination(selectedIndex, accountsPerPage, totalUserAccounts);

    int drawnItems = 0;
    if (totalUserAccounts > 0)
    {
        for (int i = 0; i < accountsPerPage && (pagination.pageOffset + i) < totalUserAccounts; i++)
        {
            int accountIndex = pagination.pageOffset + i;
            int yPos = startY + i * (buttonHeight + spacing);

            drawMenuButton(
                10, yPos, 140, buttonHeight, usersTologin[accountIndex].email,
                selectedIndex == accountIndex);

            drawnItems++;
        }

        drawPageInfo(pagination.maxPages, startY, drawnItems, buttonHeight, spacing, pagination.currentPage);
    }
    else
    {
        tft.setCursor(10, 60);
        tft.setTextColor(colorValue(DisplayColor::Red));
        tft.setTextSize(1);
        tft.print("No users available");
    }

    // ---------- Footer ----------
    drawFooter("Prev/Next + Select");
}