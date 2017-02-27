/*********************************************************************
 This is an example of using DotStar Digital LED Strip from Adafruit 
 and running a digital audio input through a FFT in order to drive the
 LED strip from music.  Effect 1 is a work light for when you need to
 work but the other 3 effects are for when you want to have some fun.

 Copyright (c) 2017 Tim Bass

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
 
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution

 Tim Bass - Extended to use the Bluefruit logic to receive commands from 
 the Adafruit Bluefruit app to control a DotStar L.E.D. light strip for 
 controlling color and pattern of the lights using the FastLED library.
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

#include <fix_fft.h>
#include "FastLED.h"


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
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         0
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

#define DATA_PIN          11  // RED WIRE
#define CLK_PIN           12  // GREEN WIRE
#define LED_TYPE          DOTSTAR
#define COLOR_ORDER       BGR
#define NUM_LEDS          140
#define CONFETTI_TRYS     100
#define CENTER_PIXEL      30
#define BRIGHTNESS        130  // MAX == 255
#define FRAMES_PER_SECOND 160
#define AUDIO_PIN         3
#define NUM_OF_CHANNELS   1
#define CH_0              0
#define NUM_SHOW          8
#define BOUNCE            75  //17
CRGB leds[NUM_LEDS];
static const CRGB mInsideColor = CRGB::DarkViolet, mOutsideColor = CRGB::Blue;

// base colors
uint8_t baseRed = 0, baseGreen = 0, baseBlue = 50;
int flareThreshold = 130;
// variables for the Fast Fourier Transform (FFT)
char im[128], data[128];
// manipulation variables to translate FFT for display
// we are going to create 14 channels of audio
char data_avgs[14];       
int i, val;

// pattern control
uint8_t pattern = 1;

// knightRiderEffect
int mIrisLoc = 1, mStepper = 1;
// fireAndIceEffect
int mIrisLocR = 1, mIrisLocB = NUM_LEDS - 1, mStepperR = 1, mStepperB = 1;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns e.g. 0-255
bool bRefresh = true;


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


/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void) {
  /*while (!Serial);  // required for Flora & Micro
  delay(500); //*/

  FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit App Controller Example"));
  Serial.println(F("-----------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) ) {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE ) {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
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
  /*while (! ble.isConnected()) {
      delay(500);
  } //*/

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
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
    data_avgs[i] = data[i*4] + data[i*4 + 1] + data[i*4 + 2] + data[i*4 + 3];
    // remap values for display range: input 0-30 & output 0-5 ... originally 0-9
    data_avgs[i] = map(data_avgs[i], 0, 30, 0, 255);
  }
  
  // read the bass and SHOW IT!
  int value = data_avgs[CH_0];

  //Serial.println(value);

  if (pattern == 1) {
    //knightRiderEffect(value);
    //uvMeter(value);
    workLightEffect();
  } else if (pattern == 2) {
    rainEffect(ColorWheel(value));
  } else if (pattern == 3) {
    showFromCenterEffect(ColorWheel(value));
  } else {
    fireAndIceEffect(value);
  }
}

void workLightEffect() {
  // only refresh the worklight if something changed, we don't want the light to flicker/change
  // while we work
  if (bRefresh == true) {
    // clear all of the leds on the string setting them to show nothing
    FastLED.clear();
    // show it!
    FastLED.show();

    // randomly pick an led and set the color of that led to the selected color
    for (i = 0; i < CONFETTI_TRYS; i++ ) {
      leds[ random16(NUM_LEDS) ] += CRGB(baseRed, baseGreen, baseBlue);
      // show it!
      FastLED.show();
      // a small delay for the lights up effect so it can be enjoyed
      FastLED.delay(7);
    }

    // now lets set all the leds in the strand to the selected color so we can work
    for (i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(baseRed, baseGreen, baseBlue);
    }
    
    // show it! and set the refresh to false (no refresh means no flicker/changes)
    FastLED.show();
    bRefresh = false;
  }
}

void fireAndIceEffect(int value) {
  // Dim by 42/256ths
  fadeToBlackBy(leds, NUM_LEDS, 42);

  if (mIrisLocR >= NUM_LEDS - 1 || mIrisLocR < 1 || value > BOUNCE) {
    mStepperR = -mStepperR;
  }
  mIrisLocR += mStepperR;

  
  if (mIrisLocB >= NUM_LEDS - 1 || mIrisLocB < 1 || value > BOUNCE) {
    mStepperB = -mStepperB;
  }
  mIrisLocB += mStepperB;

  leds[mIrisLocR] += CRGB::Red;
  leds[mIrisLocB] += CRGB::Blue;
  FastLED.show(); 
}

void showFromCenterEffect(CRGB value) {
  int i;
  for (i = 0; i < NUM_LEDS/2 - 1; i++) {
    leds[i] = leds[i+1];
  }
  leds[i] = value;
  
  for (i = NUM_LEDS - 1; i > NUM_LEDS/2; i--) {
    leds[i] = leds[i-1];
  }
  leds[i] = value;

  //Display the new values
  FastLED.show();
}

void rainEffect(CRGB value) {
  //Shift the current values.
  int i = 0;
  for (; i < NUM_LEDS - 1; i++) {
    leds[i] = leds[i+1];
  }

  //Fill in the new value at the end of each array
  leds[i] = value;
  
  //Display the new values
  FastLED.show();
}

void uvMeter(int input) {
  Serial.print("input: ");
  Serial.print(input);
  // translate input 0 - 255 to position 0 - CENTER_PIXEL
  input = map(input, 0, 255, 0, CENTER_PIXEL);
  if (input < 0) {
    input = 0;
  }
  if (input > CENTER_PIXEL) {
    input = CENTER_PIXEL;
  }
  Serial.print("  out: ");
  Serial.println(input);
  
  int pos;
  // set the pixels below center
  for (pos = 0; pos < CENTER_PIXEL - input; pos++) {
    leds[pos] = mOutsideColor;
  }
  for (; pos < CENTER_PIXEL; pos++) {
    leds[pos] = mInsideColor;
  }
  // set the pixels above center
  for (; pos < CENTER_PIXEL + input; pos++) {
    leds[pos] = mInsideColor;
  }
  for (; pos < NUM_LEDS; pos++) {
    leds[pos] = mOutsideColor;
  }
  
  FastLED.show();
  // insert a delay to keep the framerate modest
  //FastLED.delay(1000/FRAMES_PER_SECOND); 
}

// a colored dot sweeping back and forth, with fading trails
void knightRiderEffect(int value) {
  // Dim by 42/256ths
  fadeToBlackBy(leds, NUM_LEDS, 42);

  //int pos = beatsin16(13,0,NUM_LEDS);
  //Serial.println(value);
  //Serial.println(mIrisLoc);
  if (mIrisLoc >= NUM_LEDS - 1 || mIrisLoc < 1 || value > BOUNCE) {
    mStepper = -mStepper;
  }
  mIrisLoc += mStepper;

  // approximate conversion to HSV color in order to get a more accurate fade
  leds[mIrisLoc] = rgb2hsv_approximate(CRGB(baseRed, baseGreen, baseBlue)); //CHSV( gHue, 255, 192);
  FastLED.show();  
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
      switch(buttnum) {
        case 1: case 2: case 3: case 4:
          bRefresh = true;
          pattern = buttnum;
          break;
        case 5:
          if (flareThreshold >= 55)
            flareThreshold -= 5;
          Serial.print("flareThreshold = ");
          Serial.println(flareThreshold);
          break;
        case 6:
          if (flareThreshold <= 270)
            flareThreshold += 5;
          Serial.print("flareThreshold = ");
          Serial.println(flareThreshold);
          break;
        case 7: // left
          if (pattern > 0)
            pattern--;
          else
            pattern = 4;
          break;
        case 8: // right
          if (pattern < 4)
            pattern++;
          else
            pattern = 1;
          break;
        default:
          break;
      }
    } else {
      Serial.println(" released");
    }
  }

  // Color
  if (packetbuffer[1] == 'C') {
    baseRed = packetbuffer[2];
    baseGreen = packetbuffer[3];
    baseBlue = packetbuffer[4];
    bRefresh = true;
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

// CRGB mInsideColor = CRGB::DarkViolet
CRGB ColorWheel(byte WheelPos) {
  if (WheelPos < 5) {
    return CRGB::Black;
  } else if (WheelPos < flareThreshold) {
    WheelPos -= flareThreshold;
    return CRGB(WheelPos * baseRed / 255, WheelPos * baseGreen / 255, WheelPos * baseBlue / 255);
  } else {
    WheelPos -= 40;
    return CRGB(255 - WheelPos, 255 - WheelPos, 255 - WheelPos);
  }
}
