#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_WS2801.h>

// variables for the Adafruit Pixels
uint8_t mDataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t mClockPin = 3;    // Green wire on Adafruit Pixels
const int mNumberOfLEDs = 25;
Adafruit_WS2801 mStrip = Adafruit_WS2801(mNumberOfLEDs, mDataPin, mClockPin);
const uint32_t mClearColor = Color(0, 0, 0), mNathalieColor = Color(2, 200, 255), mRedColor = Color(25, 0, 0), mBlueColor = Color(0, 0, 20), mGreenColor = Color(0, 25, 0), mPurpleColor = Color(15, 0, 20), mYellowColor = Color(25, 25, 0);//Pink Color(200, 10, 100);
uint32_t leds[mNumberOfLEDs];

// variables for the Fast Fourier Transform (FFT)
char im[128], data[128];

// manipulation variables to translate FFT for display
// 14 channels of audio
char data_avgs[14];       
// Display channel to display, 0 == Bass
const int mCh0 = 0, mCh1 = 1, mCh2 = 2, mCh3 = 3, mCh4 = 4, mCh5 = 5, mCh6 = 6, mCh7 = 7;
const int mCh8 = 8, mCh9 = 9, mCh10 = 10, mCh11 = 11, mCh12 = 12, mCh13 = 13, mCh14 = 14;
int i, val;

#define AUDIOPIN 3

void setup() {
  for (i = 0; i < mNumberOfLEDs; i++) {
    leds[i] = mClearColor;
  }
  // initialize the LED strip
  mStrip.begin();
  mStrip.show();
  // serial output for debugging
  //Serial.begin(9600);
}

void loop() {
  // read in data from the AUDIOPIN
  for (i = 0; i < 128; i++){                                   
    val = analogRead(AUDIOPIN);
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
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 5);
  }
  // read the bass and SHOW IT!
  int value = data_avgs[mCh0];
  //Serial.println(value);
  if (value > 12) {
    display(12);
  } else {
    display(value);
  }
}

void display(int input) {
  for (i = 24; i > 0; i--) {
    leds[i] = leds[i - 1];
  }
  
  calcDisplay1(input);
  
  // set the pixels
  for (i = 0; i < mNumberOfLEDs; i++) {
    mStrip.setPixelColor(i, leds[i]);
  }
  
  mStrip.show();
}

/* Helper functions */
// Pattern 1
void calcDisplay1(int pInput) {
  if (pInput < 6) {
    leds[0] = mClearColor;
  } else {
    leds[0] = mPurpleColor;
  }
}

void calcDisplay2(int pInput) {
  if (pInput == 0) {
    leds[0] = mClearColor;
  } else if (pInput > 5) {
    leds[0] = mYellowColor;
  } else if (pInput > 8) {
    leds[0] = mRedColor;
  } else {
    leds[0] = mGreenColor;
  }
}

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
