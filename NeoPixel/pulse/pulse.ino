#include <fix_fft.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

static const int DATA_PIN = 6;    // Yellow wire on Adafruit Pixels
static const int NUM_OF_PIXELS = 120;
static const int DELAY = 5;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel mStrip = Adafruit_NeoPixel(NUM_OF_PIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);

int i, val;

int mLEDArrayMap[NUM_OF_PIXELS];

void setup() {
  // initialize the led map  
  for (int i = 0; i < NUM_OF_PIXELS; i++) {
    mLEDArrayMap[i] = 0;
  }
  
  mStrip.begin();
  mStrip.show(); // Initialize all pixels to 'off'
}

const int C_RED = 1;
const int C_GREEN = 2;
const int C_BLUE = 3;
const int C_WHITE = 4;

void loop() {
  //pulse(C_GREEN, DELAY);
  pulseFAST();
}

void pulse(int pCOLOR, int pDelay) {
  int irus = 0;
  while (true) {
    // initialize the led map  
    for (int i = 0; i < NUM_OF_PIXELS; i++) {
      mLEDArrayMap[i] = 50;
    }
    
    for (int i = 0; i < 3; i++) {
      if (irus - i >= 0) {
        mLEDArrayMap[irus - i] = 200;
      }
    }

    // set the pixels to the array map
    for (int i = 0; i < NUM_OF_PIXELS; i++) {
      switch(pCOLOR) {
        case C_RED:
          mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], 0, 0));
          break;
        case C_GREEN:
          mStrip.setPixelColor(i, mStrip.Color(0, mLEDArrayMap[i], 0));
          break;
        case C_BLUE:
          mStrip.setPixelColor(i, mStrip.Color(0, 0, mLEDArrayMap[i]));
          break;
        default:
          mStrip.setPixelColor(i, mStrip.Color(mLEDArrayMap[i], mLEDArrayMap[i], mLEDArrayMap[i]));
          break;
      }
    }
      
    mStrip.show();
    delay(pDelay);
    
    if (irus < NUM_OF_PIXELS) {
      irus++;
    } else {
      irus = 0;
    }
  }
}

void pulseFAST() {
  int irus = 0;
  uint32_t lColor = mStrip.Color(0,0,50);
  uint32_t hColor = mStrip.Color(0,0,200);
  
  while (true) {
    // initialize the led map  
    for (int i = 0; i < NUM_OF_PIXELS; i++) {
      mStrip.setPixelColor(i, lColor);
    }
    
    for (int i = 0; i < 3; i++) {
      int j = irus - i;
      if (j >= 0) {
        mStrip.setPixelColor(j, hColor);
      }
    }
      
    mStrip.show();
    
    if (irus < NUM_OF_PIXELS) {
      irus++;
    } else {
      irus = 0;
    }
  }
}
