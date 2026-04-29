#pragma once
#include <stdint.h>

// 16-bit RGB565 color values suitable for TFT displays
enum class DisplayColor : uint16_t
{
    Black = 0x0000,
    White = 0xFFFF,
    Red = 0xF800,
    Green = 0x07E0,
    Blue = 0x001F,
    Yellow = 0xFFE0,
    Cyan = 0x07FF,
    Magenta = 0xF81F,
    Orange = 0xFD20
};

// Helper to get raw 16-bit value
static inline uint16_t colorValue(DisplayColor c)
{
    return static_cast<uint16_t>(c);
}
