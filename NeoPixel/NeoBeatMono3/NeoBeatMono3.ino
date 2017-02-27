/*********************************************************************
 This is an example of using NeoPixel Digital LED Strip from Adafruit 
 and reading an audio value from the LEFT_CHANNEL.  Based on the value
 read it is then sent through a FFT in order to determine a color to
 display for a single channel and then display it.

 Copyright (c) 2017 Tim Bass

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

// NeoPixels
#define PIN 10
#define NUM_OF_PIXELS 120  // 30
#define HALF 60           // 15

// Audio
#define LEFT_CHANNEL 3
#define NUM_OF_CHANNELS 1

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_OF_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

//Set up arrays for cycling through all the pixels.
uint32_t LEFT_ARRAY[HALF];
uint32_t RIGHT_ARRAY[HALF];
int valL, valR;
uint32_t NOTTA = strip.Color(0,0,0);

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
  
  
  //Shift the current values.
  for (int i = 0; i < HALF - 1; i++) {
    LEFT_ARRAY[i] = LEFT_ARRAY[i+1];
    RIGHT_ARRAY[i] = RIGHT_ARRAY[i+1];
  }
  
  // determine color
  uint32_t daColor;
  if (value < 15) {
    daColor = Color(0,0,0);
  } else if (value < 40) {
    daColor = Color(0,0,50);
  } else if (value < 78) {
    daColor = Color(0,0,100);
  } else {
    daColor = Color(100,0,0);
  }
  
  //Fill in the new value at the end of each array
  LEFT_ARRAY[HALF-1] = daColor;
  RIGHT_ARRAY[HALF-1] = daColor;
  
  // Go through each Pixel on the strip and set its color
  for (int i=0; i < HALF; i++) {
    //set pixel color
    strip.setPixelColor(i, LEFT_ARRAY[i]);
    strip.setPixelColor(NUM_OF_PIXELS-i-1, RIGHT_ARRAY[i]);
  }

  //Display the new values
  strip.show();
  
  //sample delay
  delay(40);
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
