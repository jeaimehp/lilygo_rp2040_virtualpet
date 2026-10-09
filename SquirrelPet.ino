#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// LILYGO T-PicoC3, RP2040 side. GPIO numbers, not physical pin numbers.
constexpr uint8_t LCD_CS=5, LCD_DC=1, LCD_RST=0, LCD_BL=4;
constexpr uint8_t POWER=22, FEED=6, PLAY=7;
Adafruit_ST7789 lcd(&SPI, LCD_CS, LCD_DC, LCD_RST);
GFXcanvas16 screen(240,135); // 64,800-byte framebuffer avoids flicker.
constexpr uint16_t SKY=0xD71F, GRASS=0x6C69, INK=0x2945;
constexpr uint16_t FUR=0xA344, LIGHT=0xD4A8, BELLY=0xFE37;
int food=80, joy=80, energy=90;
bool sleeping=false;
uint32_t actionAt=0, lastTick=0, lastFrame=0;
enum Action { IDLE, EATING, PLAYING };
Action action=IDLE;

struct Button {
  uint8_t pin;
  bool raw=false, down=false;
  uint32_t changed=0;
  bool update(uint32_t now) {
    bool read=digitalRead(pin)==LOW;
    if(read!=raw) { raw=read; changed=now; }
    if(raw!=down && uint32_t(now-changed)>=30) {
      down=raw;
      return down;
    }
    return false;
  }
};
Button feedButton{FEED}, playButton{PLAY};
bool chord=false;

void label(int x,int y,const char *text,uint16_t color=INK) {
  screen.setCursor(x,y); screen.setTextColor(color); screen.print(text);
}
void meter(int x,const char *name,int value,uint16_t color) {
  label(x,19,name);
  screen.fillRoundRect(x,29,65,7,3,0xFFFF);
  screen.fillRect(x+2,31,(61*value)/100,3,color);
}
void acorn(int x,int y) {
  screen.fillCircle(x,y+3,5,BELLY);
  screen.fillRoundRect(x-6,y-2,12,5,2,FUR);
  screen.drawLine(x,y-2,x+2,y-6,INK);
}
void squirrel(uint32_t now) {
  bool playing=action==PLAYING;
  int bounce=(!sleeping && ((now/240)%2)) ? 1 : 0;
  int hop=playing ? int((now/100)%6) : 0;
  if(hop>3) hop=6-hop;
  int x=117+(playing ? int((now/120)%3)*3-3 : 0);
  int y=88+bounce-hop*3;
  int swish=sleeping ? 0 : int((now/180)%5)-2;
  screen.fillRoundRect(86,104,76,5,2,0x4B47);
  // Large curled tail, behind the body.
  screen.fillCircle(x-28+swish,y-17,22,FUR);
  screen.fillCircle(x-33+swish,y-22,16,LIGHT);
  screen.fillCircle(x-30+swish,y-25,9,FUR);
  screen.fillCircle(x-27+swish,y-27,5,LIGHT);
  screen.fillRoundRect(x-27,y-17,24,26,9,FUR);
  screen.fillCircle(x,y,18,FUR);
  screen.fillCircle(x+6,y+2,12,BELLY);
  screen.fillRoundRect(x-12,y+12,17,6,3,FUR);
  screen.fillRoundRect(x+4,y+12,19,6,3,FUR);
  screen.fillTriangle(x+1,y-25,x+4,y-45,x+14,y-28,FUR);
  screen.fillTriangle(x+4,y-28,x+5,y-39,x+10,y-28,LIGHT);
  screen.fillTriangle(x+16,y-28,x+24,y-44,x+27,y-24,FUR);
  screen.fillCircle(x+14,y-22,17,FUR);
  screen.fillCircle(x+25,y-16,9,BELLY);
  screen.fillCircle(x+31,y-20,3,INK);
  bool blink=sleeping || (now%4200>3980);
  if(blink) screen.drawLine(x+17,y-26,x+23,y-26,INK);
  else {
    screen.fillCircle(x+20,y-27,3,INK);
    screen.drawPixel(x+21,y-28,0xFFFF);
  }
  screen.drawLine(x+26,y-11,x+30,y-13,INK);
  screen.fillCircle(x+12,y-5,5,LIGHT);
  if(action==EATING) {
    acorn(x+20,y-5);
    if((now/180)%2) label(x+34,y-6,".");
  }
  if(playing) {
    screen.fillCircle(x+43,y+7-hop*4,7,0xF986);
    screen.drawCircle(x+43,y+7-hop*4,7,INK);
  }
  if(sleeping) label(x+37,y-42,((now/500)%2)?"Z z":"z Z");
}
void render(uint32_t now) {
  screen.fillScreen(SKY);
  screen.fillCircle(218,53,10,0xFFE0);
  screen.fillRoundRect(12,49,35,8,4,0xFFFF);
  screen.fillRoundRect(165,43,29,7,3,0xFFFF);
  screen.fillRect(0,100,240,35,GRASS);
  for(int i=10;i<240;i+=23) screen.drawLine(i,108,i+3,103,0x4B47);
  screen.setTextSize(1);
  label(8,5,"ACORN");
  const char *mood=sleeping?"Sleeping":action==EATING?"Nom nom!":
    action==PLAYING?"Wheee!":food<30?"Hungry":energy<25?"Sleepy":
    joy<30?"Let's play":"Feeling bright";
  label(100,5,mood);
  meter(8,"Food",food,0xDB65);
  meter(87,"Joy",joy,0xEAB8);
  meter(166,"Energy",energy,0x259B);
  squirrel(now);
  screen.fillRect(0,118,240,17,INK);
  label(5,123,"B1 Feed  B2 Play  Both Sleep",0xFFFF);
  lcd.drawRGBBitmap(0,0,screen.getBuffer(),240,135);
}
void setup() {
  pinMode(POWER,OUTPUT); digitalWrite(POWER,HIGH);
  pinMode(LCD_BL,OUTPUT); digitalWrite(LCD_BL,LOW);
  pinMode(FEED,INPUT_PULLUP); pinMode(PLAY,INPUT_PULLUP);
  SPI.setSCK(2); SPI.setTX(3); // LCD is write-only; MISO is unconnected.
  lcd.init(135,240); lcd.setRotation(1);
  digitalWrite(LCD_BL,HIGH);
  if(!screen.getBuffer()) { lcd.fillScreen(ST77XX_RED); while(true) delay(1000); }
  lastTick=millis();
  render(lastTick);
}
void loop() {
  uint32_t now=millis();
  bool fed=feedButton.update(now), played=playButton.update(now);
  if(feedButton.down && playButton.down) {
    if(!chord) { sleeping=!sleeping; action=IDLE; chord=true; }
  } else {
    // Wait until both are released after a chord.
    if(!feedButton.down && !playButton.down) chord=false;
    if(!chord && (fed || played)) {
      sleeping=false; actionAt=now;
      if(fed) { food=min(100,food+18); joy=min(100,joy+3); action=EATING; }
      else { joy=min(100,joy+15); energy=max(0,energy-6); action=PLAYING; }
    }
  }
  if(action!=IDLE && uint32_t(now-actionAt)>2200) action=IDLE;
  if(uint32_t(now-lastTick)>=10000) {
    lastTick=now;
    food=max(0,food-1);
    if(sleeping) energy=min(100,energy+5);
    else { joy=max(0,joy-1); energy=max(0,energy-1); }
  }
  if(uint32_t(now-lastFrame)>=65) { lastFrame=now; render(now); }
  delay(1);
}
