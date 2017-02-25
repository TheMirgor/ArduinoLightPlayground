#include "SPI.h"
#include "Adafruit_WS2801.h"

uint8_t mDataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t mClockPin = 3;    // Green wire on Adafruit Pixels

int mIrisLoc = 0;
int mTailLength = 15;
int mFadeStep = 20;

int mStepper = 1;
int mNumberOfLEDs = 22;
int mWait = 500;

// indigo???
int mRed = 75;
int mGreen = 0;
int mBlue = 130;
  
Adafruit_WS2801 mStrip = Adafruit_WS2801(mNumberOfLEDs, mDataPin, mClockPin);

void setup() {
  mStrip.begin();
  
  mStrip.show();
}

void loop() {
  display();
  
  if ((mIrisLoc > mNumberOfLEDs) || (mIrisLoc < 0)) {
    mStepper = -mStepper;
  }
  
  mIrisLoc += mStepper;
  delay(mWait);
}

void display() {
  // clear the pixels
  for (int i = 0; i < mNumberOfLEDs; i++) {
    mStrip.setPixelColor(i, Color(0,0,0));
  }
  
  // set the iris
  setPixel(mIrisLoc, mRed, mGreen, mBlue);
  
  if (mStepper > 0) {
    for (int i = 1; i < mTailLength && mIrisLoc - i > 0; i++) {
      setPixel(mIrisLoc - i, mRed-(i*mFadeStep), mGreen-(i*mFadeStep), mBlue-(i*mFadeStep));
    }
  } else {
    for (int i = 1; i < mTailLength && mIrisLoc + i < mNumberOfLEDs; i++) {
      setPixel(mIrisLoc + i, mRed-(i*mFadeStep), mGreen-(i*mFadeStep), mBlue-(i*mFadeStep));
    }
  }
  
  mStrip.show();
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

void setPixel(int p, int r, int g, int b) {
  if (r > 255) { r = 255; }
  if (r < 0) { r = 0; }
  if (g > 255) { g = 255; }
  if (g < 0) { g = 0; }
  if (b > 255) { b = 255; }
  if (b < 0) { b = 0; }
  
  mStrip.setPixelColor(p, Color(r, g, b));
}

