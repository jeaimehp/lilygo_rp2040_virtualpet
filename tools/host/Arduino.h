#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <string>
using std::min; using std::max;
#define PROGMEM
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT_PULLUP 2
class __FlashStringHelper;
class String : public std::string { public: using std::string::string; };
inline void yield() {} inline void delay(unsigned long) {}
inline void pinMode(int,int) {} inline void digitalWrite(int,int) {}
inline int digitalRead(int) { return HIGH; } inline uint32_t millis() { return 0; }
