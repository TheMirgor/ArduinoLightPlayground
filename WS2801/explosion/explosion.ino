#include "SPI.h"
#include "Adafruit_WS2801.h"

uint8_t dataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t clockPin = 3;    // Green wire on Adafruit Pixels

static uint16_t mWidth = 5;
static uint16_t mHeight = 5;
int mDelay = 500;
int mRed = 0;
int mGreen = 0;
int mBlue = 51;

Adafruit_WS2801 strip = Adafruit_WS2801(mWidth, mHeight, dataPin, clockPin);

void setup() {
  strip.begin();

  // Update LED contents, to start they are all 'off'
  strip.show();
}


void loop() {
  // Some example procedures showing how to display to the pixels
  //drawX(5, 5, 100);
  //bounce(7, 6, 50);
  
  resetPixels(mWidth, mHeight);
  strip.setPixelColor(2, 2, mRed, mGreen, mBlue);
  strip.show();
  delay(mDelay);
  
  resetPixels(mWidth, mHeight);
  strip.setPixelColor(2, 1, mRed, mGreen, mBlue);
  strip.setPixelColor(2, 3, mRed, mGreen, mBlue);
  strip.setPixelColor(1, 2, mRed, mGreen, mBlue);
  strip.setPixelColor(3, 2, mRed, mGreen, mBlue);
  strip.show();
  delay(mDelay);

  resetPixels(mWidth, mHeight);
  strip.setPixelColor(1, 1, mRed, mGreen, mBlue);
  strip.setPixelColor(1, 3, mRed, mGreen, mBlue);
  strip.setPixelColor(3, 1, mRed, mGreen, mBlue);
  strip.setPixelColor(3, 3, mRed, mGreen, mBlue);
  strip.show();
  delay(mDelay);

  resetPixels(mWidth, mHeight);
  strip.setPixelColor(2, 0, mRed, mGreen, mBlue);
  strip.setPixelColor(2, 4, mRed, mGreen, mBlue);
  strip.setPixelColor(0, 2, mRed, mGreen, mBlue);
  strip.setPixelColor(4, 2, mRed, mGreen, mBlue);
  strip.show();
  delay(mDelay);

  resetPixels(mWidth, mHeight);
  strip.setPixelColor(0, 1, mRed, mGreen, mBlue);
  strip.setPixelColor(0, 3, mRed, mGreen, mBlue);
  strip.setPixelColor(1, 0, mRed, mGreen, mBlue);
  strip.setPixelColor(1, 4, mRed, mGreen, mBlue);
  strip.setPixelColor(3, 0, mRed, mGreen, mBlue);
  strip.setPixelColor(3, 4, mRed, mGreen, mBlue);
  strip.setPixelColor(4, 1, mRed, mGreen, mBlue);
  strip.setPixelColor(4, 3, mRed, mGreen, mBlue);
  strip.show();
  delay(mDelay);

  resetPixels(mWidth, mHeight);
  strip.setPixelColor(0, 0, mRed, mGreen, mBlue);
  strip.setPixelColor(0, 4, mRed, mGreen, mBlue);
  strip.setPixelColor(4, 0, mRed, mGreen, mBlue);
  strip.setPixelColor(4, 4, mRed, mGreen, mBlue);
  strip.show();
  delay(mDelay);
}

void resetPixels(int pWidth, int pHeight) {
  for (int x = 0; x < pWidth; x++) {
    for (int y = 0; y < pHeight; y++) {
      strip.setPixelColor(x, y, 0, 0, 0);
    }
  }
  strip.show();
}



void drawX(uint8_t w, uint8_t h, uint8_t wait) {
  uint16_t x, y;
  for (x = 0; x < w; x++) {
    strip.setPixelColor(x, x, 255, 0, 0);
    strip.show();
    delay(wait);
  }
  for (y = 0; y < h; y++) {
    strip.setPixelColor(w-1-y, y, 0, 0, 255);
    strip.show();
    delay(wait);
  }
}

void bounce(uint8_t w, uint8_t h, uint8_t wait) {
  int16_t x = 1;
  int16_t y = 2;
  int8_t xdir = +1;
  int8_t ydir = -1;
  int j;
  for (j=0; j < 256; j++) {
     x = x + xdir;
     y = y + ydir;
     if (x < 0) {
       x = -x;
       xdir = - xdir;
     }
     if (y < 0) {
       y = -y;
       ydir = - ydir;
     }
     if (x == w) {
       x = w-2;
       xdir = - xdir;
     }
     if (y == h) {
       y = h-2;
       ydir = - ydir;
     }
     strip.setPixelColor(x, y, Wheel(j));
     strip.show();
     delay(wait);
     strip.setPixelColor(x, y, 0, 0, 0);
  }
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void drawPixel(uint8_t x, uint8_t y, int r, int g, int b) {
    strip.setPixelColor(x, y, r, g, b);
    strip.show();
    delay(50);
}
