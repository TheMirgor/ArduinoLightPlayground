/*********************************************************************
 This is an example of using NeoPixels Digital LED Strip 
 from Adafruit and a proximity censor for wiring up a bar so when
 someone stands in front of the bar the lights will turn on and when
 they leave the bar e.g. after preparing their drink, the lights
 will fade off.  Use Case: a bar in a movie theatre room with a
 projector to minimize wash out for other viewers due to in room
 lighting coming on.

 Copyright (c) 2017 Tim Bass

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

//---------------------------------------------------------
// ADAFRUIT PIXELS
//---------------------------------------------------------
static const int DATA_PIN = 6;
static const int NUM_OF_PIXELS = 119; //119
static const int CENTER_PIXEL = 59; //59
static const int COLOR_STEP = 2;
static const int MAX_INTENSITY = 100;
static const int DELAY = 15; // 15
static const int ADD_PIXEL_ON = 3;
static const int C_RED = 1;
static const int C_GREEN = 2;
static const int C_BLUE = 3;
static const int C_WHITE = 4;
static const int C_PURPLE = 5;
static const int C_LIGHT_BLUE = 6;
static const int C_YELLOW = 7;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel mStrip = Adafruit_NeoPixel(NUM_OF_PIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);

int mLEDArrayMap[NUM_OF_PIXELS];

//---------------------------------------------------------
// SENSOR
//---------------------------------------------------------
static const int PROX_PIN = 0;
static const int PROX_TRIGGER_DIST = 175;

//---------------------------------------------------------
// STATE MANAGEMENT
//---------------------------------------------------------
int FADE_COUNT = 0;
boolean B_ON = false;

void setup() {
  // initialize the led map  
  for (int i = 0; i < NUM_OF_PIXELS; i++) {
    mLEDArrayMap[i] = 0;
  }
  // Initialize all pixels to 'off'
  mStrip.begin();
  mStrip.show();
  
  // debug
  Serial.begin(9600);
}

void loop() {
  // runThis();
  bar();
}

void bar() {
  int val = readProx();
  
  if ((val < PROX_TRIGGER_DIST) && (FADE_COUNT <= 12)) {
    FADE_COUNT++;
  } else if ((val >= PROX_TRIGGER_DIST) && (FADE_COUNT >= 0)) {
    FADE_COUNT--;
    delay(200);
  }
    
  // turn it on!
  if ((B_ON == false) && (FADE_COUNT >= 3)) {
    Serial.println("ON!");
    B_ON = true;
    display(1, C_LIGHT_BLUE, DELAY, MAX_INTENSITY / 2);
  }
 
  // turn it off...
  if ((B_ON == true) && (FADE_COUNT <= 0)) {
    Serial.println("OFF!");
    B_ON = false;
    display(0, C_LIGHT_BLUE, DELAY, MAX_INTENSITY / 2);
  }
  
  delay(100);
}

// test method to cycle through different colors, you must add this to the
// loop method in order to see the affect
void runThis() {
  display(1, C_RED, DELAY, MAX_INTENSITY);
  delay(500);
  display(0, C_RED, DELAY, MAX_INTENSITY);
  delay(2500);
  display(1, C_YELLOW, DELAY, MAX_INTENSITY / 2);
  delay(500);
  display(0, C_YELLOW, DELAY, MAX_INTENSITY / 2);
  delay(2500);
  display(1, C_GREEN, DELAY, MAX_INTENSITY);
  delay(500);
  display(0, C_GREEN, DELAY, MAX_INTENSITY);
  delay(2500);
  display(1, C_LIGHT_BLUE, DELAY, MAX_INTENSITY / 2);
  delay(500);
  display(0, C_LIGHT_BLUE, DELAY, MAX_INTENSITY / 2);
  delay(2500);
  display(1, C_BLUE, DELAY, MAX_INTENSITY);
  delay(500);
  display(0, C_BLUE, DELAY, MAX_INTENSITY);
  delay(2500);
  display(1, C_PURPLE, DELAY, MAX_INTENSITY / 2);
  delay(500);
  display(0, C_PURPLE, DELAY, MAX_INTENSITY / 2);
  delay(2500);
//  display(1, C_WHITE, DELAY, MAX_INTENSITY / 3);
//  delay(500);
//  display(0, C_WHITE, DELAY, MAX_INTENSITY / 3);
//  delay(2500);
}

// read from the proximity sensor in order to determine if the light
// should be turned on.  Uses an average of 8 reads to determine
// real distance.
int readProx() {
  int distsensor, i;
  long time;
  
  distsensor = 0;
  for (i=0; i<8; i++) {
    distsensor += analogRead(PROX_PIN);
    delay(50);
  }
  distsensor /= 8;
  
  Serial.print(F("Sensor = ")); Serial.println(distsensor);

  return distsensor;
}

void display(int pON, int pCOLOR, int pDelay, int INTENSITY) {
  int myNumOfPixelsToEffect = 0;
  boolean mChanged;
  int mAddPixel = 0;
  
  if (pON == 1) {
    do {
      mChanged = false;
      // CENTER
      if (mLEDArrayMap[CENTER_PIXEL] < INTENSITY) {
        mChanged = true;
        mLEDArrayMap[CENTER_PIXEL] += COLOR_STEP;
      }
      // WINGS
      for (int i = 1; i <= myNumOfPixelsToEffect; i++) {
        // LEFT
        if (mLEDArrayMap[CENTER_PIXEL - i] < INTENSITY) {
          mChanged = true;
          mLEDArrayMap[CENTER_PIXEL - i] += COLOR_STEP;
        }
        // RIGHT
        if (mLEDArrayMap[CENTER_PIXEL + i] < INTENSITY) {
          mChanged = true;
          mLEDArrayMap[CENTER_PIXEL + i] += COLOR_STEP;
        }
      }
      
      // set the pixels to the array map
      for (int i = 0; i < NUM_OF_PIXELS; i++) {
        switch(pCOLOR) {
          case C_RED:
            mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], 0, 0));
            break;
          case C_GREEN:
            mStrip.setPixelColor(i, mStrip.Color(0, mLEDArrayMap[i], 0));
            break;
          case C_BLUE:
            mStrip.setPixelColor(i, mStrip.Color(0, 0, mLEDArrayMap[i]));
            break;
          case C_PURPLE:
            mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], 0, mLEDArrayMap[i]));
            break;
          case C_LIGHT_BLUE:
            mStrip.setPixelColor(i, mStrip.Color(0, mLEDArrayMap[i], mLEDArrayMap[i]));
            break;
          case C_YELLOW:
            mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], mLEDArrayMap[i], 0));
            break;
          default:
            mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], mLEDArrayMap[i], mLEDArrayMap[i]));
            break;
        }
      }
      
      mStrip.show();
      delay(pDelay);

      if ((mAddPixel == ADD_PIXEL_ON) && (myNumOfPixelsToEffect < ((NUM_OF_PIXELS - 1) / 2))) {
        mAddPixel = 0;
        myNumOfPixelsToEffect++;
      } else {
        mAddPixel += 1;
      }
    } while (mChanged == true);
  } else {
    do {
      mChanged = false;
      // CENTER
      if (mLEDArrayMap[CENTER_PIXEL] > 0) {
        mChanged = true;
        mLEDArrayMap[CENTER_PIXEL] -= COLOR_STEP;
      }
      // WINGS
      for (int i = 1; i <= myNumOfPixelsToEffect; i++) {
        // LEFT
        if (mLEDArrayMap[CENTER_PIXEL - i] > 0) {
          mChanged = true;
          mLEDArrayMap[CENTER_PIXEL - i] -= COLOR_STEP;
        }
        // RIGHT
        if (mLEDArrayMap[CENTER_PIXEL + i] > 0) {
          mChanged = true;
          mLEDArrayMap[CENTER_PIXEL + i] -= COLOR_STEP;
        }
      }
      
      // set the pixels to the array map
      for (int i = 0; i < NUM_OF_PIXELS; i++) {
        switch(pCOLOR) {
          case C_RED:
            mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], 0, 0));
            break;
          case C_GREEN:
            mStrip.setPixelColor(i, mStrip.Color(0, mLEDArrayMap[i], 0));
            break;
          case C_BLUE:
            mStrip.setPixelColor(i, mStrip.Color(0, 0, mLEDArrayMap[i]));
            break;
          case C_PURPLE:
            mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], 0, mLEDArrayMap[i]));
            break;
          case C_LIGHT_BLUE:
            mStrip.setPixelColor(i, mStrip.Color(0, mLEDArrayMap[i], mLEDArrayMap[i]));
            break;
          case C_YELLOW:
            mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], mLEDArrayMap[i], 0));
            break;
          default:
            mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], mLEDArrayMap[i], mLEDArrayMap[i]));
            break;
        }
      }
      
      mStrip.show();
      delay(pDelay);

      if ((mAddPixel == ADD_PIXEL_ON) && (myNumOfPixelsToEffect < ((NUM_OF_PIXELS - 1) / 2))) {
        mAddPixel = 0;
        myNumOfPixelsToEffect++;
      } else {
        mAddPixel += 1;
      }
    } while (mChanged == true);
  }
}


void fadeAll(int pON, int pCOLOR, int pDelay) {
  if (pON == 1) {
    for (int c = 0; c < 150; c += 2) {
      for (int i = 0; i < NUM_OF_PIXELS; i++) {
        switch(pCOLOR) {
          case C_RED:
            mStrip.setPixelColor(i, mStrip.Color(c, 0, 0));
            break;
          case C_GREEN:
            mStrip.setPixelColor(i, mStrip.Color(0, c, 0));
            break;
          case C_BLUE:
            mStrip.setPixelColor(i, mStrip.Color(0, 0, c));
            break;
          default:
            mStrip.setPixelColor(i, mStrip.Color(c, c, c));
            break;
        }
      }
      mStrip.show();
      delay(pDelay);
    }
  } else {
    for (int c = 150; c >= 0; c -= 2) {
      for (int i = 0; i < NUM_OF_PIXELS; i++) {
        switch(pCOLOR) {
          case C_RED:
            mStrip.setPixelColor(i, mStrip.Color(c, 0, 0));
            break;
          case C_GREEN:
            mStrip.setPixelColor(i, mStrip.Color(0, c, 0));
            break;
          case C_BLUE:
            mStrip.setPixelColor(i, mStrip.Color(0, 0, c));
            break;
          default:
            mStrip.setPixelColor(i, mStrip.Color(c, c, c));
            break;
        }
      }
      mStrip.show();
      delay(pDelay);
    }
  }
}

