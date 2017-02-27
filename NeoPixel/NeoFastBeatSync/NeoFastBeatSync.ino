/*********************************************************************
 This is an example of using NeoPixel Digital LED Strip from Adafruit 
 and running a digital audio input through a FFT in order to drive the
 LED strip from music. Added some additional logic for performance.

 Copyright (c) 2017 Tim Bass

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/ 

#include "FastLED.h"
#include <fix_fft.h>

#define DATA_PIN            10
#define LED_TYPE            WS2811
#define COLOR_ORDER         GRB
#define NUM_LEDS            120
#define CENTER_PIXEL        60
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND   30
#define AUDIO_PIN           3
#define NUM_OF_CHANNELS     1
#define CH_0                0

static const CRGB mInsideColor = CRGB::DarkViolet, mOutsideColor = CRGB::Blue;
char data_avgs[14];  
int i, val;
// variables for the Fast Fourier Transform (FFT)
char im[128], data[128];
uint8_t baseRed = 0, baseGreen = 0, baseBlue = 50;
int flareThreshold = 220;

void setup() {
  delay(3000); // 3 second delay for recovery
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
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
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 255);
  }
  
  // read the bass and SHOW IT!
  int value = data_avgs[CH_0];

  //uvMeter(value);
  showFromCenterEffect(ColorWheel(value));
}

void showFromCenterEffect(CRGB value) {
  int i;
  for (i = 0; i < NUM_LEDS/2 - 1; i++) {
    leds[i] = leds[i+1];
  }
  leds[i] = value;
  
  for (i = NUM_LEDS - 1; i > NUM_LEDS/2; i--) {
    leds[i] = leds[i-1];
  }
  leds[i] = value;

  //Display the new values
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);
}

void uvMeter(int input) {
  Serial.print("input: ");
  Serial.print(input);
  // translate input 0 - 255 to position 0 - CENTER_PIXEL
  input = map(input, 0, 255, 0, CENTER_PIXEL);
  if (input < 0) {
    input = 0;
  }
  if (input > CENTER_PIXEL) {
    input = CENTER_PIXEL;
  }
  Serial.print("  out: ");
  Serial.println(input);
  
  int pos;
  // set the pixels below center
  for (pos = 0; pos < CENTER_PIXEL - input; pos++) {
    leds[pos] = mOutsideColor;
  }
  for (; pos < CENTER_PIXEL; pos++) {
    leds[pos] = mInsideColor;
  }
  // set the pixels above center
  for (; pos < CENTER_PIXEL + input; pos++) {
    leds[pos] = mInsideColor;
  }
  for (; pos < NUM_LEDS; pos++) {
    leds[pos] = mOutsideColor;
  }
  
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

CRGB ColorWheel(byte WheelPos) {
  if (WheelPos < 5) {
    return CRGB::Black;
  } else if (WheelPos < flareThreshold) {
    WheelPos -= flareThreshold;
    return CRGB(WheelPos * baseRed / 255, WheelPos * baseGreen / 255, WheelPos * baseBlue / 255);
  } else {
    WheelPos -= 40;
    return CRGB(255 - WheelPos, 255 - WheelPos, 255 - WheelPos);
  }
}
