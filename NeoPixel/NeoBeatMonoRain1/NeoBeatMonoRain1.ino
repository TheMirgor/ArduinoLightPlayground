#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

// NeoPixels
#define PIN_L 9
#define NUM_OF_PIXELS_L 60    // 30

// Audio
#define AUDIO_PIN 3
#define NUM_OF_CHANNELS 14   // 14

// Pot
#define POT_B 2
#define POT_W 1
#define POT_R 0

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel stripL = Adafruit_NeoPixel(NUM_OF_PIXELS_L, PIN_L, NEO_GRB + NEO_KHZ800);

// Set up arrays for cycling through all the pixels.
uint32_t ARRAY_L[NUM_OF_PIXELS_L];

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
  stripL.begin();
  stripL.show(); // Initialize all pixels to 'off'
  
  // debug
  Serial.begin(9600);
}

void loop() {  
  // read in 128 samples from the AUDIOPIN
  for (i = 0; i < 128; i++){                                   
    val = analogRead(AUDIO_PIN);
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
    data_avgs[i] = (data[i*4] + data[i*4 + 1] + data[i*4 + 2] + data[i*4 + 3]) /4;

    if (i == 0) {
      Serial.println(data_avgs[i]);
    }
    // remap values for display range: input 0-30 & output 0-5 ... originally 0-9
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 80);  //80
  }
  
  // read the bass and SHOW IT!
  showLow(BlueWheel(data_avgs[mCh1]));  
  
  //sample delay  e.g. 40  
  delay(30);
}

/* Helper functions */

void showLow(uint32_t value) {
  //Shift the current values.
  for (int i = 0; i < NUM_OF_PIXELS_L - 1; i++) {
    ARRAY_L[i] = ARRAY_L[i+1];
  }
  
  //Fill in the new value at the end of each array
  ARRAY_L[NUM_OF_PIXELS_L-1] = value;
  
  // Go through each Pixel on the strip and set its color
  for (int i=0; i < NUM_OF_PIXELS_L; i++) {
    //set pixel color
    //stripL.setPixelColor(i, ARRAY_L[i]);
    stripL.setPixelColor(NUM_OF_PIXELS_L-i-1, ARRAY_L[i]);
  }

  //Display the new values
  stripL.show();
}

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

uint32_t RedWheel(byte WheelPos) {
  int readR = map(analogRead(POT_R), 0, 1024, 160, 260); // 170
  //Serial.println(readR);
  
  if (WheelPos < 15) {
    return Color(0,0,0);
  } else if (WheelPos < readR) {
    WheelPos -= readR;
    return Color(255 - WheelPos, 0, 0);
  } else {
    WheelPos -= 40;
    return Color(255 - WheelPos, 255 - WheelPos, 255 - WheelPos);
  }
}

uint32_t GreenWheel(byte WheelPos) {
  int readG = map(analogRead(POT_W), 0, 1024, 160, 260);
  Serial.println(readG);

  if (WheelPos < 15) {
    return Color(0,0,0);
  } else if (WheelPos < readG) {
    WheelPos -= readG;
    return Color(0, 255 - WheelPos, 0);
  } else {
    WheelPos -= 40;
    return Color(255 - WheelPos, 255 - WheelPos, 255 - WheelPos);
  }
}

uint32_t BlueWheel(byte WheelPos) {
  int readB = map(analogRead(POT_B), 0, 1024, 0, 200);
  //Serial.print(WheelPos);
  //Serial.print(" < ");
  //Serial.println(readB);

  if (WheelPos < readB) {
    return Color(0,0,0);
  } else {
    WheelPos -= readB;
    return Color(0, 0, 255 - WheelPos);
  }
}
