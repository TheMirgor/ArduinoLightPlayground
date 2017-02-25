/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution

 Additional coding performed by Tim Bass and distributed as is with
 no warrenty, use at your own risk.  tim.bass.cos@gmail.com
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

#define DATA_PIN          10  // RED WIRE
#define LED_TYPE          WS2811
#define COLOR_ORDER       GRB
#define NUM_LEDS          120
#define CONFETTI_TRYS     240
#define NUM_SHOW          8
#define BOUNCE            75
int BRIGHTNESS = 165;  // MAX == 255
CRGB leds[NUM_LEDS];
static const CRGB mInsideColor = CRGB::DarkViolet, mOutsideColor = CRGB::Blue;

// base colors
uint8_t baseRed = 255, baseGreen = 255, baseBlue = 255;
int flareThreshold = 130;
// variables for the Fast Fourier Transform (FFT)
char im[128], data[128];
// manipulation variables to translate FFT for display
// we are going to create 14 channels of audio
char data_avgs[14];       
int i, val;

// pattern control
uint8_t pattern = 1;

// sinelonEffect
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

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
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

  if (pattern == 1) {
    workLightEffect();
  } else if (pattern == 2) {
    sinelonEffect();
  } else if (pattern == 3) {
    fireAndIceEffect(0);
  } else {
    juggleEffect();
  }
}

// a solid work light by which to work
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

// a colored dot sweeping back and forth, with fading trails
void sinelonEffect() {
  // set the leds to fade to black by decreasing the brightness by approximatly 10% or 25/256ths
  fadeToBlackBy(leds, NUM_LEDS, 25);
  
  // determine the next position (led) to light up, 
  // beatsin16 generates a 16-bit sine wave at a given BPM, that oscillates within a given range
  int pos = beatsin16(13,0,NUM_LEDS);
  
  // set the color of the led at the determined position
  leds[pos] += CRGB(baseRed, baseGreen, baseBlue);
  
  // show it!
  FastLED.show(); 
}

// inspired by thinking of two battling dragons one ice, one fire
void fireAndIceEffect(int value) {
  // set the leds to fade to black by decreasing the brightness by approximatly 16.4% or 42/256ths
  fadeToBlackBy(leds, NUM_LEDS, 42);

  // set the Red dragons position
  if (mIrisLocR >= NUM_LEDS - 1 || mIrisLocR < 1 || value > BOUNCE) {
    mStepperR = -mStepperR;
  }
  mIrisLocR += mStepperR;

  // set the Blue dragons position
  if (mIrisLocB >= NUM_LEDS - 1 || mIrisLocB < 1 || value > BOUNCE) {
    mStepperB = -mStepperB;
  }
  mIrisLocB += mStepperB;

  // set the color of the led at the dragons positions
  leds[mIrisLocR] += CRGB::Red;
  leds[mIrisLocB] += CRGB::Blue;

  // show it!
  FastLED.show(); 
}

void juggleEffect() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 4; i++) {
    leds[beatsin16(i+3,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    // number of colors divided by the number of weaving lines e.g. 256 / 4 = 64
    dothue += 64;
  }

  // show it!
  FastLED.show();
}

void readColor() {
  // Wait for new data to arrive from the bluetooth connection to a phone
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;

  // Recieved a packet of data!
  printHex(packetbuffer, len);

  // Test to see if a button was pushed
  if (packetbuffer[1] == 'B') {
    // Which button?
    uint8_t buttnum = packetbuffer[2] - '0';
    
    // Was it pressed or released?
    boolean pressed = packetbuffer[3] - '0';
    
    Serial.print ("Button "); Serial.print(buttnum);
    if (pressed) {
      Serial.println(" pressed");
      switch(buttnum) {
        case 1: case 2: case 3: case 4:
          // set the refresh flag to true, used by work light.
          bRefresh = true;
          // set the pattern to be used based on the button that was pressed
          pattern = buttnum;
          break;
        case 5: // up
          break;
        case 6: // down
          break;
        case 7: // left
          break;
        case 8: // right
          break;
        default:
          break;
      }
    } else {
      Serial.println(" released");
    }
  }

  // Test to see if a Color was selected
  if (packetbuffer[1] == 'C') {
    // read the Red value
    baseRed = packetbuffer[2];
    // read the Green value
    baseGreen = packetbuffer[3];
    // read the Blue value
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

