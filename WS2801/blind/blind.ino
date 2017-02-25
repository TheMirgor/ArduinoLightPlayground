#include "SPI.h"
#include "Adafruit_WS2801.h"

uint8_t mDataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t mClockPin = 3;    // Green wire on Adafruit Pixels

int mStepper = 1;
const int mNumberOfLEDs = 22;
int mWait = 35;

int mRedArrayMap[mNumberOfLEDs], mGreenArrayMap[mNumberOfLEDs], mBlueArrayMap[mNumberOfLEDs];
Adafruit_WS2801 mStrip = Adafruit_WS2801(mNumberOfLEDs, mDataPin, mClockPin);


void setup() {
  mStrip.begin();
  mStrip.show();
}


void loop() {
  display(140, 140, 170);
  delay(mWait);

}

void display(int pColor) {
  for (int i = 0; i < mNumberOfLEDs; i++) {
    setPixel(i, pColor, pColor, pColor);
  }
  
  mStrip.show();
}

void display(int pRed, int pGreen, int pBlue) {
  for (int i = 0; i < mNumberOfLEDs; i++) {
    setPixel(i, pRed, pGreen, pBlue);
  }
  
  mStrip.show();
}

/* Helper functions */
void setPixel(int p, int r, int g, int b) {
  if (r > 255) { r = 255; }
  if (r < 0) { r = 0; }
  if (g > 255) { g = 255; }
  if (g < 0) { g = 0; }
  if (b > 255) { b = 255; }
  if (b < 0) { b = 0; }
  
  mStrip.setPixelColor(p, r, g, b);
}

