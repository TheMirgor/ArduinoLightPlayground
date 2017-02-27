/*********************************************************************
 This is an example of using NeoPixel Digital LED Strip from Adafruit 
 and reading an audio value from the LEFT_CHANNEL.  Based on the value
 read it is then sent through a color wheel in order to determine a
 color to display from the center.  All existing colors will be
 shifted outward before adding the new color to the display.

 Copyright (c) 2017 Tim Bass

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define PIN 10

//static const int DATA_PIN = 2;    // Yellow wire on Adafruit Pixels
//static const int CLOCK_PIN = 3;   // Green wire on Adafruit Pixels
static const int LEFT_CHANNEL = 3;
static const int RIGHT_CHANNEL = 4;
static const int NUM_OF_PIXELS = 120;
static const int HALF = NUM_OF_PIXELS/2;

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

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  int boss = map(analogRead(LEFT_CHANNEL), 0, 400, 0, 255);
  
  uint32_t daColor;
  if (boss < 25 ) {
    daColor = NOTTA;
  } else {
    daColor = Color(boss, boss/2, 0);
  }
  
  //Set the hue (0-255) and 24-bit color depending on left channel value
  //byte hue_left = constrain(map(analogRead(LEFT_CHANNEL), 0, 400, 0, 255), 0, 255);
  //uint32_t color_left = Wheel(hue_left);
  
  //Set the hue (0-255) and 24-bit color depending on right channel value
  //byte hue_right = constrain(map(analogRead(RIGHT_CHANNEL), 0, 400, 0, 255), 0, 255);
  //uint32_t color_right = Wheel(hue_right);
  
  //Shift the current values.
  for (int i = 0; i < HALF - 1; i++) {
    LEFT_ARRAY[i] = LEFT_ARRAY[i+1];
    RIGHT_ARRAY[i] = RIGHT_ARRAY[i+1];
  }
  
  //Fill in the new value at the end of each array
  LEFT_ARRAY[HALF-1] = daColor;
  RIGHT_ARRAY[HALF-1] = daColor; // color_right;
  
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
