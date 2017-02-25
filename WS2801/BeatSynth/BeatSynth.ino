#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_WS2801.h>

// variables for the Adafruit Pixels
uint8_t mDataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t mClockPin = 3;    // Green wire on Adafruit Pixels
int mNumberOfLEDs = 25;
Adafruit_WS2801 mStrip = Adafruit_WS2801(mNumberOfLEDs, mDataPin, mClockPin);
uint32_t mClearColor;
uint32_t mChColors[5];
int mChSteps[5];

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
  //Serial.begin(9600);
  
  mClearColor  = Color(0,  0,  0);
  mChColors[0] = Color( 0,  0, 50);
  mChColors[1] = Color( 0, 50, 50);
  mChColors[2] = Color( 0, 50,  0);
  mChColors[3] = Color(50, 50,  0);
  mChColors[4] = Color(50,  0,  0);
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
  mChSteps[0] = data_avgs[mCh0];
  mChSteps[1] = data_avgs[mCh3];
  mChSteps[2] = data_avgs[mCh6];
  mChSteps[3] = data_avgs[mCh9];
  mChSteps[4] = data_avgs[mCh12];

  //Serial.println(value);
  display();
}

void display() {
  // clear the pixels
  for (int i = 0; i < mNumberOfLEDs; i++) {
    mStrip.setPixelColor(i, mClearColor);
  }
  
  // loop through the 5 channels
  for (int i = 0; i < 5; i++) {
    if (mChSteps[i] > 5) {
      mChSteps[i] = 5;
    }
    for (int j = i * 5; j < mChSteps[i] + (i * 5) ; j++) {
      mStrip.setPixelColor(j, mChColors[i]);
    }
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
