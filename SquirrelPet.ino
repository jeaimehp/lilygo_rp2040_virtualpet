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
  int x=playing ? 65+int((now-actionAt)/45)%100 : 117;
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
// Front view for mealtimes and grooming; side view remains for running.
void frontSquirrel(uint32_t now, bool eating) {
  int y=79+int((now/280)%2);
  screen.fillCircle(86,y+4,20,FUR);
  screen.fillCircle(81,y,13,LIGHT);
  screen.fillCircle(86,y-3,7,FUR);
  screen.fillRoundRect(103,y,37,29,13,FUR);
  screen.fillRoundRect(110,y+5,23,22,9,BELLY);
  screen.fillTriangle(99,y-9,102,y-32,114,y-17,FUR);
  screen.fillTriangle(129,y-17,141,y-32,143,y-9,FUR);
  screen.fillCircle(121,y-9,23,FUR);
  screen.fillCircle(113,y,10,BELLY);
  screen.fillCircle(129,y,10,BELLY);
  bool blink=now%4200>3980;
  for(int x=111;x<=131;x+=20) {
    if(blink) screen.drawLine(x-3,y-14,x+3,y-14,INK);
    else { screen.fillCircle(x,y-14,3,INK); screen.drawPixel(x+1,y-15,0xFFFF); }
  }
  screen.fillTriangle(117,y-3,125,y-3,121,y+2,INK);
  screen.drawLine(121,y+2,121,y+6,INK);
  screen.fillRoundRect(104,y+23,15,6,3,FUR);
  screen.fillRoundRect(125,y+23,15,6,3,FUR);
  int paw=eating ? int((now/200)%2)*3 : int((now/180)%3)*5;
  screen.fillCircle(107,y+10-paw,5,LIGHT);
  screen.fillCircle(135,y+10-paw,5,LIGHT);
  if(eating) {
    acorn(121,y+10-paw);
    acorn(53,104); acorn(65,104);
    if((now/200)%2) { screen.drawPixel(143,y+5,BELLY); screen.drawPixel(147,y+8,BELLY); }
  } else {
    // One paw reaches the cheek during a grooming animation.
    screen.drawLine(135,y+10-paw,139,y-5,FUR);
    label(166,64,"* *",0xFFFF);
  }
}
void sleepScene(uint32_t now) {
  // Curled body in a leaf nest, gently rising with each breath.
  int breath=int((now/700)%2);
  screen.fillRoundRect(65,94,111,17,8,0xB426);
  for(int i=72;i<174;i+=13) screen.drawLine(i,103,i+7,97,BELLY);
  screen.fillRoundRect(88,74-breath,67,30,14,FUR);
  screen.fillCircle(100,85-breath,23,LIGHT);
  screen.fillCircle(101,85-breath,15,FUR);
  screen.fillCircle(105,83-breath,8,LIGHT);
  screen.fillCircle(143,89-breath,15,FUR);
  screen.fillTriangle(132,79-breath,136,66-breath,145,77-breath,FUR);
  screen.fillCircle(152,93-breath,7,BELLY);
  screen.drawLine(142,86-breath,149,89-breath,INK);
  screen.fillCircle(158,92-breath,2,INK);
  label(167,65,((now/700)%2)?"Z z":"z Z",0xFFFF);
}
void digScene(uint32_t now) {
  int bob=int((now/140)%3);
  screen.fillCircle(127,100,17,INK);
  screen.fillCircle(102,72+bob,21,FUR);
  screen.fillCircle(94,65+bob,15,LIGHT);
  screen.fillCircle(98,63+bob,9,FUR);
  screen.fillRoundRect(105,76+bob,29,25,10,FUR);
  screen.fillCircle(137,91+bob,14,FUR);
  screen.fillTriangle(132,82+bob,135,68+bob,143,82+bob,FUR);
  screen.fillCircle(149,96+bob,6,BELLY);
  screen.fillCircle(154,93+bob,2,INK);
  screen.drawLine(143,87+bob,148,88+bob,INK);
  screen.fillCircle(134,103,5,LIGHT);
  acorn(127,102);
  for(int i=0;i<4;i++) screen.fillCircle(153+i*8,99-((now/90+i*3)%14),2,FUR);
}
void render(uint32_t now) {
  int idleView=int((now/8000)%3); // Meadow, digging, and grooming.
  bool picnic=action==EATING, chase=action==PLAYING;
  screen.fillScreen(sleeping?0x192B:SKY);
  if(sleeping) {
    screen.fillCircle(214,55,10,BELLY);
    screen.fillCircle(219,51,9,0x192B);
    for(int i=0;i<8;i++) screen.drawPixel(12+i*28,43+(i*17)%30,0xFFFF);
  } else {
    screen.fillCircle(218,53,10,0xFFE0);
    screen.fillRoundRect(12,49,35,8,4,0xFFFF);
    screen.fillRoundRect(165,43,29,7,3,0xFFFF);
    // Trees frame the different activity views.
    screen.fillRect(28,68,7,34,FUR);
    screen.fillCircle(31,62,19,0x44C7);
    screen.fillRect(202,72,7,30,FUR);
    screen.fillCircle(206,66,17,0x44C7);
  }
  screen.fillRect(0,100,240,18,sleeping?0x2245:GRASS);
  if(picnic) {
    screen.fillRect(66,99,108,18,0xF4B2);
    for(int x=66;x<174;x+=12) screen.fillRect(x,99,6,18,0xFFFF);
    screen.drawFastHLine(66,108,108,0xF4B2);
  }
  screen.setTextSize(1);
  const char *mood=sleeping?"Cozy nest":picnic?"Acorn picnic":chase?"Chase the ball":
    idleView==1?"Burying treasure":idleView==2?"Cleaning whiskers":"In the meadow";
  // Separate header keeps the same dark text contrast in day and night scenes.
  screen.fillRect(0,0,240,39,SKY);
  label(8,5,"ACORN"); label(100,5,mood);
  meter(8,"Food",food,0xDB65);
  meter(87,"Joy",joy,0xEAB8);
  meter(166,"Energy",energy,0x259B);
  if(sleeping) sleepScene(now);
  else if(picnic) frontSquirrel(now,true);
  else if(chase) squirrel(now);
  else if(idleView==1) digScene(now);
  else if(idleView==2) frontSquirrel(now,false);
  else squirrel(now);
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
  if(action!=IDLE && uint32_t(now-actionAt)>5000) action=IDLE;
  if(uint32_t(now-lastTick)>=10000) {
    lastTick=now;
    food=max(0,food-1);
    if(sleeping) energy=min(100,energy+5);
    else { joy=max(0,joy-1); energy=max(0,energy-1); }
  }
  if(uint32_t(now-lastFrame)>=65) { lastFrame=now; render(now); }
  delay(1);
}
