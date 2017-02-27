/*********************************************************************
 This is an example of using NeoPixel Digital LED Strip from Adafruit 
 and creating a Knight Rider/Cylon effect.

 Copyright (c) 2017 Tim Bass

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/  
#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

// NeoPixels
#define PIN 9
#define NUM_OF_PIXELS 60  // 30

// Audio
#define LEFT_CHANNEL 3
#define NUM_OF_CHANNELS 1

// Pot
#define POT_B 2
#define POT_W 1
#define POT_R 0

int mIrisLoc = 0;
int mTailLength = 6;
int mFadeStep = 50;

int mStepper = 1;

int mRed = 255;
int mGreen = 0;
int mBlue = 0;


// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_OF_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// variables for the Fast Fourier Transform (FFT)
char im[128], data[128];

// manipulation variables to translate FFT for display
// we are going to create 14 channels of audio
char data_avgs[14];
// Display channel to display, 0 == Bass, 13 == Treble
static const int mCh0 = 0, mCh1 = 1, mCh2 = 2, mCh3 = 3, mCh4 = 4, mCh5 = 5, mCh6 = 6, mCh7 = 7;
static const int mCh8 = 8, mCh9 = 9, mCh10 = 10, mCh11 = 11, mCh12 = 12, mCh13 = 13;
int i, val;


void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // debug
  Serial.begin(9600);
}

void loop() {
  // read in 128 samples from the AUDIOPIN
  for (i = 0; i < 128; i++){                                   
    val = analogRead(LEFT_CHANNEL);
    data[i] = val;
    im[i] = 0;
  }
  // run the FFT on the samples to transform from P(t) vs. time > P(v) vs. freq.
  fix_fft(data,im,7,0);

  // this gets the absolute value of the values in the array, so we're only dealing with positive numbers
  // Since we only use Channel 0 and average 4 values from the data area we only need to process that many
  for (i = 0; i < 4; i++) {  // 64 (max 124 channels depending on how many we want to use)
    data[i] = sqrt(data[i] * data[i] + im[i] * im[i]);  
  }

  // average bars together
  for (i = 0; i < NUM_OF_CHANNELS; i++) {
    // average together
    data_avgs[i] = data[i*4] + data[i*4 + 1] + data[i*4 + 2] + data[i*4 + 3];
    // remap values for display range: input 0-30 & output 0-5 ... originally 0-9
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 80);  //80
  }
  
  // read the bass and SHOW IT!
  int value = data_avgs[mCh0];
  Serial.print(value);
  Serial.print(" > ");

  //mIrisLoc = map(value, 0, 125, 0, NUM_OF_PIXELS);
  
  
  display();

  int bounce = map(analogRead(POT_R), 0, 1024, 75, 125);
  Serial.println(bounce);

  if ((mIrisLoc > NUM_OF_PIXELS) || (mIrisLoc < 0) || (value > bounce)) {
    mStepper = -mStepper;
  }
  
  mIrisLoc += mStepper;
  
  delay(30);
}

int numFromCenter() {
   return mIrisLoc - (NUM_OF_PIXELS / 2);
}

void display() {
  // clear the pixels
  for (int i = 0; i < NUM_OF_PIXELS; i++) {
    strip.setPixelColor(i, Color(0,0,0));
  }
  // set the iris
  strip.setPixelColor(mIrisLoc, Color(mRed, mGreen, mBlue));
  
  if (mStepper > 0) {
    for (int i = 1; i < mTailLength && mIrisLoc - i >= 0; i++) {
      strip.setPixelColor(mIrisLoc - i, Color(mRed-(i*mFadeStep), mGreen, mBlue));
    }
  } else {
    for (int i = 1; i < mTailLength && mIrisLoc + i < NUM_OF_PIXELS; i++) {
      strip.setPixelColor(mIrisLoc + i, Color(mRed-(i*mFadeStep), mGreen, mBlue));
    }
  }
  
  strip.show();
}


/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b) {
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
uint32_t Wheel(byte WheelPos) {
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

uint32_t BlueWheel(byte WheelPos) {
  if (WheelPos < 15) {
    return Color(0,0,0);
  } else if (WheelPos < map(analogRead(POT_B), 0, 1024, 100, 200)) {
    WheelPos -= 85;
    return Color(0, 0, WheelPos);
  } else {
    WheelPos -= 40;
    return Color(WheelPos, WheelPos, WheelPos);
  }
}

uint32_t RedWheel(byte WheelPos) {
  if (WheelPos < 15) {
    return Color(0,0,0);
  } else if (WheelPos < map(analogRead(POT_R), 0, 1024, 100, 200)) {
    WheelPos -= 85;
    return Color(WheelPos, 0, 0);
  } else {
    WheelPos -= 40;
    return Color(WheelPos, WheelPos, WheelPos);
  }
}

uint32_t GreenWheel(byte WheelPos) {
  if (WheelPos < 15) {
    return Color(0,0,0);
  } else if (WheelPos < map(analogRead(POT_W), 0, 1024, 100, 200)) {
    WheelPos -= 85;
    return Color(0, WheelPos, 0);
  } else {
    WheelPos -= 40;
    return Color(WheelPos, WheelPos, WheelPos);
  }
}

