#include "SPI.h"
#include "Adafruit_WS2801.h"

uint8_t mDataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t mClockPin = 3;    // Green wire on Adafruit Pixels

int mIrisLoc = 0;
int mTailLength = 5;
int mFadeStep = 40;

int mStepper = 1;
int mNumberOfLEDs = 25;
int mWait = 50;

int mRed = 255;
int mGreen = 0;
int mBlue = 0;
  
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
  mStrip.setPixelColor(mIrisLoc, Color(mRed, mGreen, mBlue));
  
  if (mStepper > 0) {
    for (int i = 1; i < mTailLength && mIrisLoc - i >= 0; i++) {
      mStrip.setPixelColor(mIrisLoc - i, Color(mRed-(i*mFadeStep), mGreen, mBlue));
    }
  } else {
    for (int i = 1; i < mTailLength && mIrisLoc + i < mNumberOfLEDs; i++) {
      mStrip.setPixelColor(mIrisLoc + i, Color(mRed-(i*mFadeStep), mGreen, mBlue));
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

