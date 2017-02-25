#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_WS2801.h>

static const int DATA_PIN = 2;    // Yellow wire on Adafruit Pixels
static const int CLOCK_PIN = 3;   // Green wire on Adafruit Pixels
static const int AUDIO_PIN = 3;
static const int NUM_OF_PIXELS = 25;
static const int CENTER_PIXEL = 11;

// variables for the Adafruit Pixels
Adafruit_WS2801 mStrip = Adafruit_WS2801(NUM_OF_PIXELS, DATA_PIN, CLOCK_PIN);
static const uint32_t mClearColor = Color(0, 0, 0), mRedColor = Color(25, 0, 0), mBrightRedColor = Color(255, 0, 0), mBlueColor = Color(0, 0, 20), mBrightBlueColor = Color(0, 0, 255), mGreenColor = Color(0, 25, 0), mBrightGreenColor = Color(0, 255, 0), mPurpleColor = Color(15, 0, 20), mBrightPurpleColor = Color(160, 0, 255), mNathalieColor = Color(2, 200, 255), Pink = Color(200, 10, 100);
static const uint32_t mInsideColor = mPurpleColor, mOutsideColor = mClearColor;
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
  // initialize the LED strip
  mStrip.begin();
  mStrip.show();
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

  // this gets the absolute value of the values in the array, so we're only dealing with positive numbers
  for (i = 0; i < 128; i++) {    
    data[i] = sqrt(data[i] * data[i] + im[i] * im[i]);  
  }

  // average bars together
  for (i = 0; i < 14; i++) {
    // average together
    data_avgs[i] = data[i*4] + data[i*4 + 1] + data[i*4 + 2] + data[i*4 + 3];
    // remap values for display range: input 0-30 & output 0-5 ... originally 0-9
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 5);
  }
  // read the bass and SHOW IT!
  int value = data_avgs[mCh0];
  //Serial.println(value);
  if (value > CENTER_PIXEL) {
    display(CENTER_PIXEL);
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
  mStrip.setPixelColor(CENTER_PIXEL, mInsideColor);
  
  // set the pixels below center
  for (i = CENTER_PIXEL - input; i < CENTER_PIXEL; i++) {
    mStrip.setPixelColor(i, mInsideColor);
  }
  // set the pixels above center
  for (i = CENTER_PIXEL + input; i > CENTER_PIXEL; i--) {
    mStrip.setPixelColor(i, mInsideColor);
  }
  
  mStrip.show();
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
