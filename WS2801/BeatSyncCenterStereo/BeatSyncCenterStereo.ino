#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_WS2801.h>

// variables for the Adafruit Pixels
uint8_t mDataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t mClockPin = 3;    // Green wire on Adafruit Pixels
int mNumberOfLEDs = 25;
Adafruit_WS2801 mStrip = Adafruit_WS2801(mNumberOfLEDs, mDataPin, mClockPin);
const uint32_t mClearColor = Color(0, 0, 0), mRedColor = Color(25, 0, 0), mBlueColor = Color(0, 0, 20), mGreenColor = Color(0, 25, 0), mPurpleColor = Color(15, 0, 20);
//Pink Color(200, 10, 100);
const uint32_t mNathalieColor = Color(2, 200, 255);

int mCenter = 12;

// variables for the Fast Fourier Transform (FFT)
char imL[128], dataL[128], imR[128], dataR[128];

// manipulation variables to translate FFT for display
// 14 channels of audio
char data_avgsL[14], data_avgsR[14];       
// Display channel to display, 0 == Bass
const int mCh0 = 0, mCh1 = 1, mCh2 = 2, mCh3 = 3, mCh4 = 4, mCh5 = 5, mCh6 = 6, mCh7 = 7;
const int mCh8 = 8, mCh9 = 9, mCh10 = 10, mCh11 = 11, mCh12 = 12, mCh13 = 13, mCh14 = 14;
int i, valL, valR;

#define AUDIOPINR 3
#define AUDIOPINL 4

void setup() {
  // initialize the LED strip
  mStrip.begin();
  mStrip.show();
  // serial output for debugging
  //Serial.begin(9600);
}

void loop() {
  // read in data from the AUDIOPINS
  for (i = 0; i < 128; i++) {
    // read left channel
    valL = analogRead(AUDIOPINL);
    dataL[i] = valL;
    imL[i] = 0;
    // read right channel
    valR = analogRead(AUDIOPINR);
    dataR[i] = valR;
    imR[i] = 0;
  }
  // run the FFT
  fix_fft(dataL,imL,7,0);
  fix_fft(dataR,imR,7,0);

  for (i = 0; i < 64; i++) {
    // this gets the absolute value of the values in the array, so we're only dealing with positive numbers
    dataL[i] = sqrt(dataL[i] * dataL[i] + imL[i] * imL[i]);  
    dataR[i] = sqrt(dataR[i] * dataR[i] + imR[i] * imR[i]);  
  }

  // average bars together
  for (i = 0; i < 14; i++) {
    // average together
    data_avgsL[i] = dataL[i*4] + dataL[i*4 + 1] + dataL[i*4 + 2] + dataL[i*4 + 3];
    data_avgsR[i] = dataR[i*4] + dataR[i*4 + 1] + dataR[i*4 + 2] + dataR[i*4 + 3];
    // remap values for display range: input 0-30 & output 0-5 ... originally 0-9
    data_avgsL[i] = map(data_avgsL[i], 0, 30, 0, 5);
    data_avgsR[i] = map(data_avgsR[i], 0, 30, 0, 5);
  }
  // read the bass and SHOW IT!
  int valueL = data_avgsL[mCh0];
  int valueR = data_avgsR[mCh0];
  //Serial.println(value);
  if ((valueL > mCenter) && (valueR > mCenter)) {
    display(mCenter, mCenter);
  } else if (valueL > mCenter) {
    display(mCenter, valueR);
  } else if (valueR > mCenter) {
    display(valueL, mCenter);
  } else {
    display(valueL, valueR);
  }
}

void display(int inputL, int inputR) {
  // clear the pixels
  for (int i = 0; i < mNumberOfLEDs; i++) {
    mStrip.setPixelColor(i, mBlueColor);
  }
  
  // set the center pixel
  mStrip.setPixelColor(mCenter, mPurpleColor);
  
  // set the pixels below center
  for (i = mCenter - inputL; i < mCenter; i++) {
    mStrip.setPixelColor(i, mPurpleColor);
  }
  // set the pixels above center
  for (i = mCenter + inputR; i > mCenter; i--) {
    mStrip.setPixelColor(i, mPurpleColor);
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
