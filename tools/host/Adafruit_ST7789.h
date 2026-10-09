#pragma once
#include "SPI.h"
#define ST77XX_RED 0xF800
class Adafruit_ST7789 { public:
Adafruit_ST7789(SPIClass*,int,int,int) {} void init(int,int) {} void setRotation(int) {}
void fillScreen(int) {} void drawRGBBitmap(int,int,uint16_t*,int,int) {}
};
