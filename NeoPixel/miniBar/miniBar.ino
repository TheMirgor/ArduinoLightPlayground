/*********************************************************************
 Tim Bass - This is an example of using NeoPixels Digital LED Strip 
 from Adafruit and a proximity censor for wiring up a bar so when
 someone stands in front of the bar the lights will turn on and when
 they leave the bar e.g. after preparing their drink, the lights
 will fade off.  Use Case: a bar in a movie theatre room with a
 projector to minimize wash out for other viewers due to in room
 lighting coming on.

 Updated Bar project to work on an Adafruit Trinket for installation.

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <Adafruit_NeoPixel.h>
 
#define DATA_PIN     4
#define N_LEDS       39
#define CENTER_LED   19
#define COLOR_STEP   2
#define MAX_INTENSITY 200
#define DELAY 10
#define ADD_PIXEL_ON 3
#define C_RED 1
#define C_GREEN 2
#define C_BLUE 3
#define C_WHITE 4
#define C_PURPLE 5
#define C_LIGHT_BLUE 6
#define C_YELLOW 7


//---------------------------------------------------------
// SENSOR - 175
//---------------------------------------------------------
#define PROX_PIN 0
#define PROX_TRIGGER_DIST 158

//---------------------------------------------------------
// STATE MANAGEMENT
//---------------------------------------------------------
int FADE_COUNT = 0;
boolean B_ON = false;
 
Adafruit_NeoPixel mStrip = Adafruit_NeoPixel(N_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);
int mLEDArrayMap[N_LEDS];
 
void setup() {
  // initialize the led map  
  for (int i = 0; i < N_LEDS; i++) {
    mLEDArrayMap[i] = 0;
  }
  mStrip.begin();
  mStrip.show();
}
 
void loop() {
  bar();
}

void bar() {
  int val = readProx();
  
  if ((val < PROX_TRIGGER_DIST) && (FADE_COUNT <= 10)) {
    FADE_COUNT++;
  } else if ((val >= (PROX_TRIGGER_DIST - 5)) && (FADE_COUNT >= 0)) {
    FADE_COUNT--;
    delay(200);
  }
    
  // turn it on!
  if ((B_ON == false) && (FADE_COUNT >= 3)) {
    B_ON = true;
    display(1, C_LIGHT_BLUE, DELAY, MAX_INTENSITY / 2);
  }
 
  // turn it off...
  if ((B_ON == true) && (FADE_COUNT <= 0)) {
    B_ON = false;
    display(0, C_LIGHT_BLUE, DELAY, MAX_INTENSITY / 2);
  }
  
  delay(100);
}

int readProx() {
  int distsensor, i;
  long time;
  
  distsensor = 0;
  for (i=0; i<8; i++) {
    distsensor += analogRead(PROX_PIN);
    delay(50);
  }
  distsensor /= 8;
  
  return distsensor;
}

void runThis() {
  display(1, C_RED, DELAY, MAX_INTENSITY);
  display(0, C_RED, DELAY, MAX_INTENSITY);

  display(1, C_YELLOW, DELAY, MAX_INTENSITY / 2);
  display(0, C_YELLOW, DELAY, MAX_INTENSITY / 2);

  display(1, C_GREEN, DELAY, MAX_INTENSITY);
  display(0, C_GREEN, DELAY, MAX_INTENSITY);

  display(1, C_LIGHT_BLUE, DELAY, MAX_INTENSITY / 2);
  display(0, C_LIGHT_BLUE, DELAY, MAX_INTENSITY / 2);

  display(1, C_BLUE, DELAY, MAX_INTENSITY);
  display(0, C_BLUE, DELAY, MAX_INTENSITY);

  display(1, C_PURPLE, DELAY, MAX_INTENSITY / 2);
  display(0, C_PURPLE, DELAY, MAX_INTENSITY / 2);
}

void clearIT() {
  display(0, C_RED, DELAY, MAX_INTENSITY);
}

void display(int pON, int pCOLOR, int pDelay, int INTENSITY) {
  int myNumOfPixelsToEffect = 0;
  boolean mChanged;
  int mAddPixel = 0;
  
  if (pON == 1) {
    do {
      mChanged = false;
      // CENTER
      if (mLEDArrayMap[CENTER_LED] < INTENSITY) {
        mChanged = true;
        mLEDArrayMap[CENTER_LED] += COLOR_STEP;
      }
      // WINGS
      for (int i = 1; i <= myNumOfPixelsToEffect; i++) {
        // LEFT
        if (mLEDArrayMap[CENTER_LED - i] < INTENSITY) {
          mChanged = true;
          mLEDArrayMap[CENTER_LED - i] += COLOR_STEP;
        }
        // RIGHT
        if (mLEDArrayMap[CENTER_LED + i] < INTENSITY) {
          mChanged = true;
          mLEDArrayMap[CENTER_LED + i] += COLOR_STEP;
        }
      }
      
      // set the pixels to the array map
      for (int i = 0; i < N_LEDS; i++) {
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

      if ((mAddPixel == ADD_PIXEL_ON) && (myNumOfPixelsToEffect < ((N_LEDS - 1) / 2))) {
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
      if (mLEDArrayMap[CENTER_LED] > 0) {
        mChanged = true;
        mLEDArrayMap[CENTER_LED] -= COLOR_STEP;
      }
      // WINGS
      for (int i = 1; i <= myNumOfPixelsToEffect; i++) {
        // LEFT
        if (mLEDArrayMap[CENTER_LED - i] > 0) {
          mChanged = true;
          mLEDArrayMap[CENTER_LED - i] -= COLOR_STEP;
        }
        // RIGHT
        if (mLEDArrayMap[CENTER_LED + i] > 0) {
          mChanged = true;
          mLEDArrayMap[CENTER_LED + i] -= COLOR_STEP;
        }
      }
      
      // set the pixels to the array map
      for (int i = 0; i < N_LEDS; i++) {
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

      if ((mAddPixel == ADD_PIXEL_ON) && (myNumOfPixelsToEffect < ((N_LEDS - 1) / 2))) {
        mAddPixel = 0;
        myNumOfPixelsToEffect++;
      } else {
        mAddPixel += 1;
      }
    } while (mChanged == true);
  }
}
