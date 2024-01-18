#include <Adafruit_TFTLCD.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

#define TS_MINX 204
#define TS_MINY 195
#define TS_MAXX 948
#define TS_MAXY 910

#define YP A2
#define XM A3
#define YM 8
#define XP 9

#define BLACK 0X0000
#define BLUE 0X001F
#define RED 0XF000
#define GREEN 0X07E0
#define CYAN 0X07FF
#define MAGENTA 0XF0IF
#define YELLOW 0XFFE0
#define WHITE 0XFFFF
unsigned long testText();
unsigned long grantedAccessText();
unsigned long deniedAccessText();
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

boolean buttonEnabled = true;

void setup(void) {

  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);

}
void loop(void) {
  for(uint8_t rotation=0; rotation<4; rotation++) {
    tft.setRotation(1);
    homeText();
    delay(2000);
    grantedAccessText();
    delay(2000);
    deniedAccessText();
    delay(2000);
  }
}
unsigned long homeText() {
  tft.fillScreen(BLACK);
  unsigned long start = micros();
  tft.setCursor(50,60);
  tft.setTextColor(WHITE);  
  tft.setTextSize(3);
  tft.println("TAP YOUR CARD");
  
  tft.setCursor(60,95);
  tft.setTextColor(WHITE);  
  tft.setTextSize(3);
  tft.println("TO CLOCK IN");
}


unsigned long grantedAccessText() {
  tft.fillScreen(BLACK);
  tft.setCursor(80,60);
  tft.setTextColor(GREEN);  
  tft.setTextSize(4);
  tft.println("ACCESS");
  tft.setCursor(80,110);
  tft.setTextColor(GREEN);  
  tft.setTextSize(4);
  tft.println("GRANTED");
}

unsigned long deniedAccessText() {
  tft.fillScreen(BLACK);
  tft.setCursor(80,60);
  tft.setTextColor(RED);  
  tft.setTextSize(4);
  tft.println("ACCESS");
  tft.setCursor(80,110);
  tft.setTextColor(RED);  
  tft.setTextSize(4);
  tft.println("DENIED");

}