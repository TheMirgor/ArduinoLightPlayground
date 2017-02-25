#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

// NeoPixels
#define PIN 10
#define NUMPIXELS 60
#define NUM_SHOW 8

#define RED 0
#define GREEN 1
#define BLUE 2

int IrisLocRed = 0;
int StepperRed = 1;

int IrisLocBlue = NUMPIXELS;
int StepperBlue = 1;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int pixelColors[NUMPIXELS + 1][3];

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // debug
  Serial.begin(9600);
}

void loop() {
  display();

  if (IrisLocRed >= NUMPIXELS || IrisLocRed < 0) {
    StepperRed = -StepperRed;
  }
  IrisLocRed += StepperRed; //*/

  if (IrisLocBlue >= NUMPIXELS || IrisLocBlue < 0) {
    StepperBlue = -StepperBlue;
  }
  IrisLocBlue += StepperBlue; //*/

  delay(50);
}



void display() {
  // clear the pixels
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, Color(0,0,0));
  }

  for (int i = 0; i < NUMPIXELS; i++) {
    for (int j = 0; j < 3; j++) {
      pixelColors[i][j] = 0;
    }
  }

  // set the iris
  int baseRed = 255, baseBlue = 255;
  pixelColors[IrisLocRed][RED] = baseRed;
  pixelColors[IrisLocBlue][BLUE] = baseBlue;

  // set the tails
  int red = baseRed, blue = baseBlue;
  int rCompare = baseRed/NUM_SHOW, bCompare = baseBlue/NUM_SHOW;

  for (int i = 0; IrisLocRed - i >= 0; i++) {
    if (red >= rCompare)
      red -= rCompare;
    pixelColors[IrisLocRed - i*StepperRed][RED] = red;
    if (red <= rCompare)
      break;
  }

  for (int i = 0; IrisLocBlue - i >= 0; i++) {
    if (blue >= bCompare)
      blue -= bCompare;
    pixelColors[IrisLocBlue - i*StepperBlue][BLUE] = blue;
    if (blue <= bCompare)
      break;
  }

  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, Color(pixelColors[i][RED], pixelColors[i][GREEN], pixelColors[i][BLUE]));
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
