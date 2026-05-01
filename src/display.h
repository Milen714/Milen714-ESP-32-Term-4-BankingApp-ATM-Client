#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Account.h>

extern Adafruit_ST7735 tft;

// Pagination helper struct
struct PaginationInfo
{
    int pageOffset;
    int maxPages;
    int currentPage;
};

// Calculate pagination info
PaginationInfo calculatePagination(int selectedIndex, int itemsPerPage, int totalItems);

void tftSetup();

void drawHeader(const String &content);

void drawMenuButton(
    int x,
    int y,
    int w,
    int h,
    const String &label,
    bool selected);

void drawAmountButton(
    int x,
    int y,
    int w,
    int h,
    int amount,
    bool selected);

void drawActionSelectionScreen(int selectedIndex);
void drawAmountScreen(const String &mode, int selectedIndex);
void drawLoadingScreen(const String &message);
void drawResultScreen(const String &title, const String &message, bool success);
void drawAccountSelectionScreen(int selectedIndex);
void drawFooter(String content = "Prev/Next + Select");
void drawPageInfo(int maxPages, const int startY, int drawnItems, const int buttonHeight, const int spacing, int currentPage);
void drawUserAccontsSelectionScreen(int selectedIndex);