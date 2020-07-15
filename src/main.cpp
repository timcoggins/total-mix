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
#define MIDI_SPEAKERB_CH 0x90

#define MIDI_MAINVOL 0x66
#define MIDI_MAINVOL_CH 0xB8

// Encoder Pins
#define ENCODER_CLOCK 37
#define ENCODER_DT 38
#define ENCODER_BUTTON 37

// Button Pins
#define BUTTON_MUTE 33
#define BUTTON_MONO 25
#define BUTTON_SPEAKERB 32

// Delay between MIDI On and Off messages
#define TIME_DELAY 200

// Settings for display drawing
#define DISPLAY_COLOUR TFT_BLACK
#define DISPLAY_MUTED_COLOUR TFT_RED
#define DISPLAY_TEXT_COLOUR TFT_WHITE

#define DISPLAY_MONO_COLOUR_TEXT TFT_GREEN
#define DISPLAY_MONO_COLOUR_ON TFT_DARKGREEN
#define DISPLAY_SPEAKERB_COLOUR_TEXT TFT_BLUE
#define DISPLAY_SPEAKERB_COLOUR_ON TFT_DARKCYAN

#define DISPLAY_VOLUME_TEXT_X 0
#define DISPLAY_VOLUME_TEXT_Y 0
#define DISPLAY_SPEAKERB_TEXT_X 120
#define DISPLAY_SPEAKERB_TEXT_Y 80
#define DISPLAY_MONO_TEXT_X 0
#define DISPLAY_MONO_TEXT_Y 80


// GLOBALS

// Rotary Encoder
int encPosition = 0; 
int encRotation;  

// Globals
int GLOB_MAINVOLUME = 0;
bool GLOB_SPEAKERB = LOW;
bool GLOB_MONO = LOW;
bool GLOB_MUTED = LOW;

// Use hardware SPI OLED Display
auto tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT);


/*
  Function to send MIDI serial data
*/

void sendMIDI(int cmd, int pitch, int velocity) {

  // Check to see if the data is in range
  /*if (cmd > 127 || cmd < 0) { 
    return; 
  }
  if (pitch > 127 || pitch < 0) { 
    return; 
  }
  if (velocity > 127 || velocity < 0) { 
    return; 
  }
  */

  // Send MIDI
  Serial2.write(cmd);
  Serial2.write(pitch);
  Serial2.write(velocity);
}

/*
  Update the screen with the current values
*/

void DisplayRefresh() {

  String MainDisplay;
  int BACKGROUND = DISPLAY_COLOUR;

  // Check if we are muted
  if (GLOB_MUTED == HIGH) {
    // Muted Screen
    MainDisplay = "Muted";
    BACKGROUND = DISPLAY_MUTED_COLOUR;
  } else {
    // Unmuted Screen
    MainDisplay = String(GLOB_MAINVOLUME);
  }
  
  // Clear the Screen
  tft.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, BACKGROUND);

  // Draw volume level
  tft.setTextColor(DISPLAY_TEXT_COLOUR, BACKGROUND);
  tft.setTextSize(2);
  tft.drawString(MainDisplay, DISPLAY_VOLUME_TEXT_X, DISPLAY_VOLUME_TEXT_Y, 4);



  // Speaker B Button
  if (GLOB_SPEAKERB == HIGH) {
    tft.setTextColor(DISPLAY_SPEAKERB_COLOUR_TEXT, DISPLAY_SPEAKERB_COLOUR_ON);
  } else {
    tft.setTextColor(DISPLAY_SPEAKERB_COLOUR_TEXT, BACKGROUND);
  }
  tft.setTextSize(1);
  tft.drawString("SPK-B", DISPLAY_SPEAKERB_TEXT_X, DISPLAY_SPEAKERB_TEXT_Y, 4);

  // Mono Button
  if (GLOB_MONO == HIGH) {
    tft.setTextColor(DISPLAY_MONO_COLOUR_TEXT, DISPLAY_MONO_COLOUR_ON);
  } else {
    tft.setTextColor(DISPLAY_MONO_COLOUR_TEXT, BACKGROUND);
  }
  tft.drawString("Mono", DISPLAY_MONO_TEXT_X, DISPLAY_MONO_TEXT_Y, 4);
}

/*
  Setup
*/

void setup() {

  // Set MIDI Serial using Serial 2
  Serial2.begin(31250, SERIAL_8N1, MIDI_RXD2, MIDI_TXD2);

  // Initialie the buttona
  pinMode(BUTTON_SPEAKERB, INPUT);
  pinMode(BUTTON_MUTE, INPUT);
  pinMode(BUTTON_MONO, INPUT);

  // Initialize the encoder
  pinMode (ENCODER_CLOCK,INPUT);
  pinMode (ENCODER_DT,INPUT);
  encRotation = digitalRead(ENCODER_CLOCK);

  // Send the main volume to Zero
  sendMIDI(MIDI_MAINVOL_CH, MIDI_MAINVOL, 0X00);   

  // Initilaize the TFT Display
  tft.init();
  tft.setSwapBytes(true); // Swap the byte order for pushImage() - corrects endianness
  tft.setRotation(1);
  tft.setTextDatum(TL_DATUM); // Top Left
  tft.setTextColor(DISPLAY_TEXT_COLOUR);

  // Update the Display
  DisplayRefresh();
}


/*
  Main Loop
*/

void loop() {

  // MAIN VOLUME ENCODER
  int encClock = digitalRead(ENCODER_CLOCK);

  if (GLOB_MUTED == 0) {
    if (encClock != encRotation) { // Use the DT pin to find out which way we turning
      if (digitalRead(ENCODER_DT) != encClock) {  // Clockwise
        if (encPosition <= 127) {
          encPosition ++;
        }
      } else { // Counter clockwise
        if (encPosition >= 0) {
          encPosition--;
        }
      }  
    } 

    encRotation = encClock;

    if(encPosition != GLOB_MAINVOLUME) {
      if(encPosition >= 0 && encPosition <= 127) {
        GLOB_MAINVOLUME = encPosition;
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
    delay(TIME_DELAY);

  }

  // SPEAKER B
  if (stateSpeakerB == HIGH) {

    // Flip the state
    GLOB_SPEAKERB = !GLOB_SPEAKERB;
    
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
    GLOB_MONO = !GLOB_MONO;

    DisplayRefresh();
    
    // Send MIDI Note ON
    sendMIDI(MIDI_MONO_CH, MIDI_MONO, 0x7F);
    delay(TIME_DELAY);
    
    // Send MIDI Note OFF
    sendMIDI(MIDI_MONO_CH, MIDI_MONO, 0x00);
  }
}