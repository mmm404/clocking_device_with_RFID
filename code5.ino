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
#define GREY 0XC0C0C0

unsigned long enrollText();
void createRects(uint16_t color);
unsigned long topicText();

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

boolean buttonEnabled = true;

void setup(void) {
  Serial.begin(9600);
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
   
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  createRects(WHITE);
  enrollText();
  homeText();
}
void loop(void) {
  static boolean wasTouched = false;
  static boolean hasProcessedTouch = false;
  TSPoint touch = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (touch.z > 10) {
    int touchX = map(touch.x, TS_MINX, TS_MAXX, 0, tft.width());
    int touchY = map(touch.y, TS_MINY, TS_MAXY, 0, tft.height());

    if (!hasProcessedTouch) {
      if (touchX > 10 && touchX < 10 + 100 && touchY > 220 && touchY < 220 + 35) {
        Serial.println("DELETE button touched");
        hasProcessedTouch = true;
      }

      if (touchX > 240 && touchX < 240 + 100 && touchY > 220 && touchY < 220 + 35) {
        Serial.println("ENROLL button touched");
        hasProcessedTouch = true;
      }
    }

    wasTouched = true;
  } else {
    if (wasTouched) {
      buttonEnabled = true;
      wasTouched = false;
      hasProcessedTouch = false;
    }
  }

  yield();
}

unsigned long enrollText() {
  tft.setCursor(240, 220);
  tft.setTextColor(WHITE);
  tft.setTextSize(2.5);
  tft.println("ENROLL");

  tft.setCursor(10, 220);
  tft.setTextColor(WHITE);
  tft.setTextSize(2.5);
  tft.println("DELETE");
}


unsigned long homeText() {
  tft.fillScreen(BLACK);
  createRects(WHITE);
  enrollText();
  //topicText(); 
  unsigned long start = micros();
  tft.setCursor(50,60);
  tft.setTextColor(WHITE);  
  tft.setTextSize(3.8);
  tft.println("TAP YOUR CARD");
  
  tft.setCursor(60,130);
  tft.setTextColor(WHITE);  
  tft.setTextSize(3.8);
  tft.println("TO CLOCK IN");
}


unsigned long grantedAccessText() {
  tft.fillScreen(BLACK);
  createRects(WHITE);
  enrollText();
  //topicText();
  tft.setCursor(90,60);
  tft.setTextColor(GREEN);  
  tft.setTextSize(3.5);
  tft.println("ACCESS");
  tft.setCursor(85,130);
  tft.setTextColor(GREEN);  
  tft.setTextSize(3.5);
  tft.println("GRANTED");
}

unsigned long deniedAccessText() {
  tft.fillScreen(BLACK);
  createRects(WHITE);
  enrollText();
  //topicText();
  tft.setCursor(90,60);
  tft.setTextColor(RED);  
  tft.setTextSize(3.5);
  tft.println("ACCESS");
  tft.setCursor(90,130);
  tft.setTextColor(RED);  
  tft.setTextSize(3.5);
  tft.println("DENIED");

}


void createRects(uint16_t color) {
  int centerX = tft.width() / 4;
  int centerY = tft.height() / 4;
  tft.fillRect(centerX + 140, centerY + 145, 100, 35, GREY);
  tft.fillRect(centerX - 80 , centerY + 145, 100, 35, GREY);

}

unsigned long topicText() {
  tft.setCursor(75, 18);
  tft.setTextColor(WHITE);
  tft.setTextSize(1.9);
  tft.println("   CDED   ACCESS  CONTROL");
  tft.setCursor(40, 20);
  tft.setTextColor(WHITE);
  tft.setTextSize(2.5);
  tft.println("   _________________");
}
