#include "SPI.h"
#include "Adafruit_WS2801.h"

uint8_t mDataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t mClockPin = 3;    // Green wire on Adafruit Pixels

int mIrisLoc = 0;
int mFadeStep = 50;

int mStepper = 1;
const int mNumberOfLEDs = 25;
int mWait = 35;

int mRed = 255;
int mGreen = 0;
int mBlue = 0;
  
int mRedArrayMap[mNumberOfLEDs], mGreenArrayMap[mNumberOfLEDs], mBlueArrayMap[mNumberOfLEDs];
Adafruit_WS2801 mStrip = Adafruit_WS2801(mNumberOfLEDs, mDataPin, mClockPin);


void setup() {
  // initialize the led map  
  for (int i = 0; i < mNumberOfLEDs; i++) {
    mRedArrayMap[i] = 0;
    mGreenArrayMap[i] = 0;
    mBlueArrayMap[i] = 0;
  }

  mStrip.begin();
  mStrip.show();
}


void loop() {
  display();
  
  if ((mIrisLoc >= mNumberOfLEDs) || (mIrisLoc < 0)) {
    mStepper = -mStepper;
    colorChanger();
  }
  
  mIrisLoc += mStepper;
  delay(mWait);
}

void colorChanger() {
  if (mRed == 255) {
    mRed = 0;
    mGreen = 255;
  } else if (mGreen == 255) {
    mGreen = 0;
    mBlue = 255;
  } else {
    mBlue = 0;
    mRed = 255;
  }
}

void display() {
  // set the array map
  for (int i = 0; i < mNumberOfLEDs; i++) {
    if (i != mIrisLoc) {
      int tRed = mRedArrayMap[i] - mFadeStep;
      if (tRed < 0) {
        tRed = 0;
      }

      int tGreen = mGreenArrayMap[i] - mFadeStep;
      if (tGreen < 0) {
        tGreen = 0;
      }

      int tBlue = mBlueArrayMap[i] - mFadeStep;
      if (tBlue < 0) {
        tBlue = 0;
      }
      
      setArrayMap(i, tRed, tGreen, tBlue);
    } else {
      setArrayMap(i, mRed, mGreen, mBlue);
    }
  }

  // set the pixels to the array map
  for (int i = 0; i < mNumberOfLEDs; i++) {
    setPixel(i, mRedArrayMap[i], mGreenArrayMap[i], mBlueArrayMap[i]);
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

void setArrayMap(int pIndex, int pRed, int pGreen, int pBlue) {
  mRedArrayMap[pIndex] = pRed;
  mGreenArrayMap[pIndex] = pGreen;
  mBlueArrayMap[pIndex] = pBlue;
}
