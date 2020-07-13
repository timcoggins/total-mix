/*

TOTAL MIX ESP32 Controller

*/

// INCLUDES
#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <Button2.h>


// DEFINES

// Midi Serial Pins
#define MIDI_RXD2 21
#define MIDI_TXD2 22    // Connect MIDI Port Here

// Midi Messages
#define MIDI_MONO 0x2A
#define MIDI_MONO_CH 0x90

#define MIDI_SPEAKERB 0x32
#define MIDI_SPEAKERB_CH 0xB8

#define MIDI_MAINVOL 0x66
#define MIDI_MAINVOL_CH 0x90

// Encoder Pins
#define ENCODER_CLOCK 37
#define ENCODER_DT 38
#define ENCODER_BUTTON 37

// Button Pins
#define BUTTON_MUTE 33
#define BUTTON_MONO 25
#define BUTTON_SPEAKERB 32

#define TIME_DELAY 100


// GLOBALS

// Rotary Encoder
int RotPosition = 0; 
int rotation;  

// Pushbutton Status


// Globals
bool GLOB_SPEAKERB = LOW;
bool GLOB_MONO = LOW;
bool GLOB_MUTED = LOW;
int GLOB_MAINVOLUME = 0;


// Use hardware SPI OLED Display
auto tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT);
//Button2 button1(BUTTON_1);
//Button2 button2(BUTTON_2);

// Screen is 240 * 135 pixels (rotated)
#define BACKGROUND_COLOR TFT_BLACK
#define TEXT_COLOR TFT_WHITE


// General Info bar => Location, WiFi
#define TOP_BAR_Y 0
#define TOP_BAR_HEIGHT 26
// Location
#define TOP_BAR_LOCATION_X 0
#define TOP_BAR_LOCATION_WIDTH 140
// Time
#define TOP_BAR_TIME_X TOP_BAR_LOCATION_WIDTH
#define TOP_BAR_TIME_WIDTH (TFT_HEIGHT - TOP_BAR_LOCATION_WIDTH)

// Bottom bar => Weather description
#define BOTTOM_BAR_Y (TFT_WIDTH - BOTTOM_BAR_HEIGHT)
#define BOTTOM_BAR_HEIGHT 16
// Weather description
#define BOTTOM_BAR_DESCRIPTION_X 0
#define BOTTOM_BAR_DESCRIPTION_WIDTH TFT_HEIGHT

// What remains is main screen
#define MAIN_BAR_Y TOP_BAR_HEIGHT
#define MAIN_BAR_HEIGHT (TFT_WIDTH - TOP_BAR_HEIGHT - BOTTOM_BAR_HEIGHT)


/*
  Function to send MIDI serial data
*/

void sendMIDI(int cmd, int pitch, int velocity) {

  // Check to see if the data is in range
  if (cmd >= 127 || cmd <= 0) {
    return;
  }

  if (pitch >= 127 || pitch <= 0) {
    return;
  }

  if (velocity >= 127 || velocity <= 0) {
    return;
  }

  // Send MIDI
  Serial2.write(cmd);
  Serial2.write(pitch);
  Serial2.write(velocity);
}

/*
  Update the screen with the current values
*/

void DisplayRefresh(){

  String MainDisplay;
  int BACK = TFT_BLACK;

  // Muted Screen
  if (GLOB_MUTED == HIGH) {
    MainDisplay = "Muted";
    BACK = TFT_RED;
  } else {
    MainDisplay = String(GLOB_MAINVOLUME);
    BACK = TFT_BLACK;
  }

  // Draw Muted Screen
  tft.setTextSize(2);
  tft.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, BACK);
  tft.setTextColor(TFT_WHITE, BACK);
  tft.drawString(MainDisplay, TOP_BAR_LOCATION_X, TOP_BAR_Y, 4);

  tft.setTextSize(1);

  // Speaker B Button
  if (GLOB_SPEAKERB == HIGH) {
    tft.setTextColor(TFT_BLUE, TFT_DARKCYAN);
  } else {
    tft.setTextColor(TFT_BLUE, BACK);
  }
  tft.drawString("SPK-B", 120, 80, 4);

  // Mono Button
  if (GLOB_MONO == HIGH) {
    tft.setTextColor(TFT_GREEN, TFT_DARKGREEN);
  } else {
    tft.setTextColor(TFT_DARKGREEN, BACK);
  }
  tft.drawString("Mono", TOP_BAR_LOCATION_X, 80, 4);
}

/*
  Setup
*/

void setup()
{

  // Set MIDI Serial using Serial 2
  Serial2.begin(31250, SERIAL_8N1, MIDI_RXD2, MIDI_TXD2);

  
  // Initialie the buttona
  pinMode(BUTTON_SPEAKERB, INPUT);
  pinMode(BUTTON_MUTE, INPUT);
  pinMode(BUTTON_MONO, INPUT);

  // Initialize the encoder
  pinMode (ENCODER_CLOCK,INPUT);
  pinMode (ENCODER_DT,INPUT);
  rotation = digitalRead(ENCODER_CLOCK);

  // Send the main volume to Zero
  sendMIDI(MIDI_MAINVOL_CH, MIDI_MAINVOL, 0X00);   

  // Initilaize the TFT Display
  tft.init();
  tft.setSwapBytes(true); // Swap the byte order for pushImage() - corrects endianness
  tft.setRotation(1);
  tft.setTextDatum(TL_DATUM); // Top Left
  tft.setTextColor(TEXT_COLOR);

  // Update the Display
  DisplayRefresh();
}


/*
  Main Loop
*/

void loop()
{
  // MAIN VOLUME ENCODER
  int encClock = digitalRead(ENCODER_CLOCK);
  if (GLOB_MUTED == 0) {
    if (encClock != rotation) { // Use the DT pin to find out which way we turning
      if (digitalRead(ENCODER_DT) != encClock) {  // Clockwise
        if (RotPosition <= 127) {
          RotPosition ++;
        }
      } else { // Counter clockwise
        if (RotPosition >= 0)
        {
          RotPosition--;
        }
      }  
    } 
    rotation = encClock;

    if(RotPosition != GLOB_MAINVOLUME) {
      if(RotPosition >= 0 && RotPosition <= 127) {
        GLOB_MAINVOLUME = RotPosition;
        sendMIDI(MIDI_MAINVOL_CH, MIDI_MAINVOL, GLOB_MAINVOLUME);
        DisplayRefresh();
      }
    }
  }

  // READ BUTTONS
  bool stateMuted = digitalRead(BUTTON_MUTE);
  bool stateSpeakerB = digitalRead(BUTTON_SPEAKERB);
  bool stateMono = digitalRead(BUTTON_MONO);

  // MUTE
  if (stateMuted == HIGH) {

    if (GLOB_MUTED == 0) {
      GLOB_MUTED = 1;

      sendMIDI(MIDI_MAINVOL_CH, MIDI_MAINVOL, 0);
      DisplayRefresh();
  
    } else {
      GLOB_MUTED = 0;

      sendMIDI(MIDI_MAINVOL_CH, MIDI_MAINVOL, GLOB_MAINVOLUME);
      DisplayRefresh();
    }
  }

  // SPEAKER B
  if (stateSpeakerB == HIGH) {

    // Flip the state
    if (GLOB_SPEAKERB == HIGH) {
      GLOB_SPEAKERB = LOW;
    } else {
      GLOB_SPEAKERB = HIGH;
    }
    
    DisplayRefresh();

    // Send MIDI Note ON
    sendMIDI(MIDI_SPEAKERB_CH, MIDI_SPEAKERB, 0x7F);
    delay(TIME_DELAY);
    
    // Send MIDI Note OFF
    sendMIDI(MIDI_SPEAKERB_CH, MIDI_SPEAKERB, 0x00);
  }

  // MONO
  if (stateMono == HIGH) {
    
    // Flip the state
    if (GLOB_MONO == HIGH) {
      GLOB_MONO = LOW;
    } else {
      GLOB_MONO = HIGH;
    }

    DisplayRefresh();
    
    // Send MIDI Note ON
    sendMIDI(MIDI_MONO_CH, MIDI_MONO, 0x7F);
    delay(TIME_DELAY);
    
    // Send MIDI Note OFF
    sendMIDI(MIDI_MONO_CH, MIDI_MONO, 0x00);
  }
}