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

// Audio
#define LEFT_CHANNEL 3
#define NUM_OF_CHANNELS 1

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
  int bounce = 80;  //map(analogRead(POT_R), 0, 1024, 75, 125);

  Serial.print("value = ");
  Serial.println(value);
  
  display();

  if (IrisLocRed >= NUMPIXELS || IrisLocRed < 0 || value > bounce) {
    StepperRed = -StepperRed;
  }
  IrisLocRed += StepperRed; //*/

  if (IrisLocBlue >= NUMPIXELS || IrisLocBlue < 0 || value > bounce) {
    StepperBlue = -StepperBlue;
  }
  IrisLocBlue += StepperBlue; //*/

  delay(50);
}



void display() {
  Serial.println("display");
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
