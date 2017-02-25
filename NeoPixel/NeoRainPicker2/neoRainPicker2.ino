/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#include <Adafruit_NeoPixel.h>

#include <fix_fft.h>

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    PIN                       Which pin on the Arduino is connected to the NeoPixels?
    NUMPIXELS                 How many NeoPixels are attached to the Arduino?
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE     1

    #define PIN                     10
    #define NUMPIXELS               60
/*=========================================================================*/

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];

// Audio
#define AUDIO_PIN 3
#define NUM_OF_CHANNELS 2   // 14

#define CH_0 0
#define CH_1 1

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void) {
  //while (!Serial);  // required for Flora & Micro
  //delay(500);

  // turn off neopixel
  strip.begin(); // This initializes the NeoPixel library.
  clearStrip();

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Neopixel Color Picker Example"));
  Serial.println(F("------------------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) ) {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE ) {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  //while (! ble.isConnected()) {
  //    delay(500);
  //}

  Serial.println(F("***********************"));

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("***********************"));

}

// base colors
uint8_t baseRed = 0, baseGreen = 0, baseBlue = 50;

// variables for the Fast Fourier Transform (FFT)
char im[128], data[128];
int i, val;
// manipulation variables to translate FFT for display
// we are going to create 14 channels of audio
char data_avgs[14];

// knightRiderEffect
int mIrisLoc = 0, mStepper = 1;

// rainEffect
uint32_t ARRAY_L[NUMPIXELS];

// showFromCenter
uint32_t LEFT_ARRAY_L[NUMPIXELS/2], RIGHT_ARRAY_L[NUMPIXELS/2];

// fireAndIceEffect
#define RED 0
#define GREEN 1
#define BLUE 2
#define NUM_SHOW 8
int IrisLocRed = 0, StepperRed = 1, IrisLocBlue = NUMPIXELS, StepperBlue = 1, pixelColors[NUMPIXELS + 1][3];

// pattern control
uint8_t pattern = 1;

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data and visualize it
*/
/**************************************************************************/
void loop(void) {
  readColor();

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

    // remap values for display range: input 0-30 & output 0-5 ... originally 0-9
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 255);  //80
  }
  int bounce = 80;  //map(analogRead(POT_R), 0, 1024, 75, 125);
  if (pattern == 1) {
    knightRiderEffect();
    if (mIrisLoc > NUMPIXELS || mIrisLoc < 0 || data_avgs[CH_0] > bounce) {
      mStepper = -mStepper;
    }
    mIrisLoc += mStepper;
  } else if (pattern == 2) {
    rainEffect(strip.Color(data_avgs[CH_0] * baseRed / 255, data_avgs[CH_0] * baseGreen / 255, data_avgs[CH_0] * baseBlue / 255));
  } else if (pattern == 3) {
    showFromCenter(strip.Color(data_avgs[CH_0] * baseRed / 255, data_avgs[CH_0] * baseGreen / 255, data_avgs[CH_0] * baseBlue / 255));
  }  else {
    fireAndIceEffect();
    if (IrisLocRed >= NUMPIXELS || IrisLocRed < 0) { // || data_avgs[CH_0] > bounce) {
      StepperRed = -StepperRed;
    }
    IrisLocRed += StepperRed;   
    if (IrisLocBlue >= NUMPIXELS || IrisLocBlue < 0) { // || data_avgs[CH_0] > bounce) {
      StepperBlue = -StepperBlue;
    }
    IrisLocBlue += StepperBlue;
  }
}

void knightRiderEffect() {
  // clear the pixels
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0,0,0));
  }
  // set the iris
  strip.setPixelColor(mIrisLoc, strip.Color(baseRed, baseGreen, baseBlue));

  int red = baseRed, green = baseGreen, blue = baseBlue;
  int rCompare = baseRed/NUM_SHOW, gCompare = baseGreen/NUM_SHOW, bCompare = baseBlue/NUM_SHOW;
  
  for (int i = 1; mIrisLoc - i >= 0; i++) {
    if (red >= rCompare)
      red -= rCompare;
    if (green >= gCompare)
      green -= gCompare;
    if (blue >= bCompare)
      blue -= bCompare;
    strip.setPixelColor(mIrisLoc - i*mStepper, strip.Color(red, green, blue));
    if (red <= rCompare && green <= gCompare && blue <= bCompare)
      break;
  }

  strip.show();
}

void rainEffect(uint32_t value) {
  //Shift the current values.
  for (int i = 0; i < NUMPIXELS - 1; i++) {
    ARRAY_L[i] = ARRAY_L[i+1];
  }
  
  //Fill in the new value at the end of each array
  ARRAY_L[NUMPIXELS-1] = value;
  
  // Go through each Pixel on the strip and set its color
  for (int i=0; i < NUMPIXELS; i++) {
    //set pixel color
    strip.setPixelColor(NUMPIXELS-i-1, ARRAY_L[i]);
  }

  //Display the new values
  strip.show();
}

void showFromCenter(uint32_t value) {
  //Shift the current values.
  for (int i = 0; i < NUMPIXELS/2 - 1; i++) {
    LEFT_ARRAY_L[i] = LEFT_ARRAY_L[i+1];
    RIGHT_ARRAY_L[i] = RIGHT_ARRAY_L[i+1];
  }
  
  //Fill in the new value at the end of each array
  LEFT_ARRAY_L[NUMPIXELS/2-1] = value;
  RIGHT_ARRAY_L[NUMPIXELS/2-1] = value;
  
  // Go through each Pixel on the strip and set its color
  for (int i=0; i < NUMPIXELS/2; i++) {
    //set pixel color
    strip.setPixelColor(i, LEFT_ARRAY_L[i]);
    strip.setPixelColor(NUMPIXELS-i-1, RIGHT_ARRAY_L[i]);
  }

  //Display the new values
  strip.show();
}

void fireAndIceEffect() {
  // clear the pixels
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0,0,0));
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
    strip.setPixelColor(i, strip.Color(pixelColors[i][RED], pixelColors[i][GREEN], pixelColors[i][BLUE]));
  }

  strip.show();
}




void readColor() {
  /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;

  /* Got a packet! */
  printHex(packetbuffer, len);

  // Button
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
    Serial.print ("Button "); Serial.print(buttnum);
    if (pressed) {
      Serial.println(" pressed");
      pattern = buttnum;
    } else {
      Serial.println(" released");
    }
  }

  // Color
  if (packetbuffer[1] == 'C') {
    baseRed = packetbuffer[2];
    baseGreen = packetbuffer[3];
    baseBlue = packetbuffer[4];
    /*Serial.print ("RGB #");
    if (baseRed < 0x10) Serial.print("0");
    if (baseRed < 100) Serial.print("0");
    Serial.print(baseRed);
    Serial.print(" ");
    if (baseGreen < 0x10) Serial.print("0");
    if (baseGreen < 100) Serial.print("0");
    Serial.print(baseGreen);
    Serial.print(" ");
    if (baseBlue < 0x10) Serial.print("0");
    if (baseBlue < 100) Serial.print("0");
    Serial.println(baseBlue); //*/
  }
}

void clearStrip() {
  for(uint8_t i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0,0,0)); // off
  }
  strip.show();
}


