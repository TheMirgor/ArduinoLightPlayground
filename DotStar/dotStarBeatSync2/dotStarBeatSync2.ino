/*********************************************************************
 Tim Bass - This is an example of using DotStart Digital LED Strip 
 from Adafruit and the FastLED library to control LEDs based on an 
 audio input.  The Audio is run through an FFT and split into a 
 NUM_OF_CHANNELS and then a single channel is used to determine the 
 display value.

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <fix_fft.h>
#include "FastLED.h"

#define DATA_PIN          11  // RED
#define CLK_PIN           12  // GREEN
#define LED_TYPE          DOTSTAR
#define COLOR_ORDER       BGR
#define NUM_LEDS          60
#define CENTER_PIXEL      30
#define BRIGHTNESS        20  // MAX == 255
#define FRAMES_PER_SECOND 160
#define AUDIO_PIN         3
#define NUM_OF_CHANNELS   1

CRGB leds[NUM_LEDS];
static const CRGB mInsideColor = CRGB::DarkViolet, mOutsideColor = CRGB::Blue;



// variables for the Fast Fourier Transform (FFT)
char im[128], data[128];
// manipulation variables to translate FFT for display
// we are going to create 14 channels of audio
char data_avgs[14];       
// Display channel to display, 0 == Bass, 13 == Treble
static const int mCh0 = 0;
int i, val;

void setup() {
  delay(3000); // 3 second delay for recovery
  FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  // read in 128 samples from the AUDIOPIN
  for (i = 0; i < 128; i++){                                   
    val = analogRead(AUDIO_PIN);
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
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 9);
  }
  
  // read the bass and SHOW IT!
  int value = data_avgs[mCh0];
  
  // but only show up to the max number of pixels we have attached
  if (value > CENTER_PIXEL) {
    display(CENTER_PIXEL);
  } else {
    display(value);
  }
}

void display(int input) {
  int i;
  // set the pixels below center
  for (i = 0; i < CENTER_PIXEL - input; i++) {
    //mStrip.setPixelColor(i, mOutsideColor);
    leds[i] = mOutsideColor;
  }
  for (; i < CENTER_PIXEL; i++) {
    //mStrip.setPixelColor(i, mInsideColor);
    leds[i] = mInsideColor;
  }
  // set the pixels above center
  for (; i < CENTER_PIXEL + input; i++) {
    //mStrip.setPixelColor(i, mInsideColor);
    leds[i] = mInsideColor;
  }
  for (; i < NUM_LEDS; i++) {
    //mStrip.setPixelColor(i, mOutsideColor);
    leds[i] = mOutsideColor;
  }
  
  //mStrip.show();
  FastLED.show();

  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
}
