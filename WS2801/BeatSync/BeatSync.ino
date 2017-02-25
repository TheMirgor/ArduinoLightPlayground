#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_WS2801.h>

// variables for the Adafruit Pixels
uint8_t mDataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t mClockPin = 3;    // Green wire on Adafruit Pixels
int mNumberOfLEDs = 25;
Adafruit_WS2801 mStrip = Adafruit_WS2801(mNumberOfLEDs, mDataPin, mClockPin);
int mRed = 0;
int mGreen = 50;
int mBlue = 0;

// variables for the Fast Fourier Transform (FFT)
char im[128], data[128];

// manipulation variables to translate FFT for display
// 14 channels of audio
char data_avgs[14];       
// Display channel to display, 0 == Bass
int mCh0 = 0, mCh1 = 1, mCh2 = 2, mCh3 = 3, mCh4 = 4, mCh5 = 5, mCh6 = 6, mCh7 = 7;
int mCh8 = 8, mCh9 = 9, mCh10 = 10, mCh11 = 11, mCh12 = 12, mCh13 = 13, mCh14 = 14;
int i,val;

#define AUDIOPIN 3

void setup() {
  // initialize the LED strip
  mStrip.begin();
  mStrip.show();
  // serial output for debugging
  // Serial.begin(9600);
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
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 13);
  }
  // read the bass and SHOW IT!
  int value = data_avgs[mCh0] - 1;
  // Serial.println(value);
  display(value);
}

void display(int input) {
  // clear the pixels
  for (int i = 0; i < mNumberOfLEDs; i++) {
    mStrip.setPixelColor(i, Color(0,0,0));
  }
  // set the iris
  mStrip.setPixelColor(input, Color(mRed, mGreen, mBlue));
  
  for (int i = 0; i < input; i++) {
    mStrip.setPixelColor(i, Color(mRed, mGreen, mBlue));
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
