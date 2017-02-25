#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6

//static const int DATA_PIN = 2;    // Yellow wire on Adafruit Pixels
//static const int CLOCK_PIN = 3;   // Green wire on Adafruit Pixels
static const int AUDIO_PIN = 3;
static const int NUM_OF_PIXELS = 120;  // 120
static const int CENTER_PIXEL_L = 59; // 59
static const int CENTER_PIXEL_R = 60; // 60

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel mStrip = Adafruit_NeoPixel(NUM_OF_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

static const uint32_t mBlueColor = mStrip.Color(0, 0, 20), mGreenColor = mStrip.Color(0, 25, 0);
static const uint32_t mInsideColor = mBlueColor, mOutsideColor = mGreenColor;
// variables for the Fast Fourier Transform (FFT)
char im[128], data[128];

// manipulation variables to translate FFT for display
// 14 channels of audio
char data_avgs[14];       
// Display channel to display, 0 == Bass, 14 == Treble
static const int mCh0 = 0, mCh1 = 1, mCh2 = 2, mCh3 = 3, mCh4 = 4, mCh5 = 5, mCh6 = 6, mCh7 = 7;
static const int mCh8 = 8, mCh9 = 9, mCh10 = 10, mCh11 = 11, mCh12 = 12, mCh13 = 13, mCh14 = 14;
int i, val;

void setup() {
  mStrip.begin();
  mStrip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // read in data from the AUDIOPIN
  for (i = 0; i < 128; i++){                                   
    val = analogRead(AUDIO_PIN);
    data[i] = val;
    im[i] = 0;
  }
  // run the FFT
  fix_fft(data,im,7,0);

  for (i = 0; i < 64; i++) {
    // this gets the absolute value of the values in the array, so we're only dealing with positive numbers
    data[i] = sqrt(data[i] * data[i] + im[i] * im[i]);  
  }

  // average bars together
  for (i = 0; i < 14; i++) {
    // average together
    data_avgs[i] = data[i*4] + data[i*4 + 1] + data[i*4 + 2] + data[i*4 + 3];
    // remap values for display range: input 0-30 & output 0-5 ... originally 0-9
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 40);
  }
  // read the bass and SHOW IT!
  int value = data_avgs[mCh0];
  //Serial.println(value);
  if (value > CENTER_PIXEL_L) {
    display(CENTER_PIXEL_L);
  } else {
    display(value);
  }
}

void display(int input) {
  // clear the pixels
  for (int i = 0; i < NUM_OF_PIXELS; i++) {
    mStrip.setPixelColor(i, mOutsideColor);
  }
  
  // set the center pixel
  mStrip.setPixelColor(CENTER_PIXEL_L, mInsideColor);
  mStrip.setPixelColor(CENTER_PIXEL_R, mInsideColor);
  
  // set the pixels below center
  for (i = CENTER_PIXEL_L - input; i < CENTER_PIXEL_L; i++) {
    mStrip.setPixelColor(i, mInsideColor);
  }
  // set the pixels above center
  for (i = CENTER_PIXEL_R + input; i > CENTER_PIXEL_R; i--) {
    mStrip.setPixelColor(i, mInsideColor);
  }
  
  mStrip.show();
}

