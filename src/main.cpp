// Includes
#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <Button2.h>

// TTGO Buttons
#define BUTTON_1 35
#define BUTTON_2 0

// Define Serial Port for MIDI (Use TX Pin)
#define RXD2 21
#define TXD2 22

const int ButPin = 32;

// PIN Assignments

// Rotary Encoder
const int CLK = 37;    // Pin 7 to clk on encoder
const int DT = 38;     // Pin 8 to DT on encoder

// Buttons
const int SpeakerBUTPin = 32;     // the number of the pushbutton pin
const int DimBUTPin = 33;     // the number of the pushbutton pin
const int MonoBUTPin = 25;     // the number of the pushbutton pin


// LEDS
const int SpeakerLEDPin =  13;      // the number of the LED pin
const int MonoLEDPin =  11;      // the number of the LED pin
const int DimLEDPin =  12;      // the number of the LED pin


// Global Variables

// Rotary Encoder
int RotPosition = 0; 
int rotation;  
int value;

// Pushbutton Status
int SpeakerState = 0;         // variable for reading the pushbutton status
int DimState = 0;            // variable for reading the pushbutton status
int MonoState = 0;           // variable for reading the pushbutton status

// Globals
int SpeakerB = LOW;
int Mono = LOW;
int MainVol = 0;
int muted = 0;

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




// plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that
// data values are less than 127:
void noteOn(int cmd, int pitch, int velocity) {

  // Checks the data passed to the function
  if (cmd >= 127 || cmd <= 0)
  {
    return;
  }

  if (pitch >= 127 || pitch <= 0)
  {
    return;
  }

  if (velocity >= 127 || velocity <= 0)
  {
    return;
  }

  // Send MIDI
  Serial2.write(cmd);
  Serial2.write(pitch);
  Serial2.write(velocity);
}



// REFRESH THE DISPLAY ;D

void DisplayRefresh(){

  String MainDisplay;
  int BACK = TFT_BLACK;

  // Muted Screen
  if (muted == HIGH) {
    MainDisplay = "Muted";
    BACK = TFT_RED;
  } else {
    MainDisplay = String(MainVol);
    BACK = TFT_BLACK;
  }

  // Draw Muted Screen
  tft.setTextSize(2);
  tft.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, BACK);
  tft.setTextColor(TFT_WHITE, BACK);
  tft.drawString(MainDisplay, TOP_BAR_LOCATION_X, TOP_BAR_Y, 4);

  tft.setTextSize(1);

  // Speaker B Button
  if (SpeakerB == HIGH) {
    tft.setTextColor(TFT_BLUE, TFT_DARKCYAN);
  } else {
    tft.setTextColor(TFT_BLUE, BACK);
  }
  tft.drawString("SPK-B", 120, 80, 4);

  // Mono Button
  if (Mono == HIGH) {
    tft.setTextColor(TFT_GREEN, TFT_DARKGREEN);
  } else {
    tft.setTextColor(TFT_DARKGREEN, BACK);
  }
  tft.drawString("Mono", TOP_BAR_LOCATION_X, 80, 4);



}


void setup()
{

  // Set MIDI Serial Rate using serial 2
  Serial2.begin(31250, SERIAL_8N1, RXD2, TXD2);

  // initialize the LED pin as an output:
  pinMode(SpeakerLEDPin, OUTPUT);
  pinMode(DimLEDPin, OUTPUT);
  pinMode(MonoLEDPin, OUTPUT);
  
  // initialize the pushbutton pin as an input:
  pinMode(SpeakerBUTPin, INPUT);
  pinMode(DimBUTPin, INPUT);
  pinMode(MonoBUTPin, INPUT);

  // initialize the rotary encoder
  pinMode (CLK,INPUT);
  pinMode (DT,INPUT);
  rotation = digitalRead(CLK);

  // send the zero voulme command+
  noteOn(0xB8, 0x66, 0X00);   

  int x = 0;
  while (x < 4) {
  digitalWrite(SpeakerLEDPin, HIGH);
  delay(100);
  digitalWrite(SpeakerLEDPin, LOW);
  digitalWrite(DimLEDPin, HIGH);
  delay(100);
  digitalWrite(DimLEDPin, LOW);
  digitalWrite(MonoLEDPin, HIGH);
  delay(100);
  digitalWrite(MonoLEDPin, LOW);
  x++;
  }


  tft.init();
  tft.setSwapBytes(true); // Swap the byte order for pushImage() - corrects endianness
  tft.setRotation(1);
  tft.setTextDatum(TL_DATUM); // Top Left
  tft.setTextColor(TEXT_COLOR);


  DisplayRefresh();

}





void loop()
{


  // MAIN VOLUME ENCODER
   value = digitalRead(CLK);
   if (muted == 0) {
     if (value != rotation){ // we use the DT pin to find out which way we turning.
     if (digitalRead(DT) != value) {  // Clockwise
      if (RotPosition <= 127)
      {
       RotPosition ++;
      }
       //LeftRight = true;
     } else { //Counterclockwise
       //LeftRight = false;
       if (RotPosition >= 0)
      {
       RotPosition--;
      }
     }
     //if (LeftRight){ // turning right will turn on red led.
       //Serial.println ("clockwise");
     //}else{        // turning left will turn on green led.   
       //Serial.println("counterclockwise");
     //}
     //Serial.print("Encoder RotPosition: ");
     //Serial.println(RotPosition);
     // this will print in the serial monitor.
     
   } 
   rotation = value;

   if(RotPosition != MainVol)
   {
    if(RotPosition >= 0 && RotPosition <= 127)
    {
    MainVol = RotPosition;
    noteOn(0xB8, 0x66, MainVol);
    DisplayRefresh();
    }
   
   }
     /*  if (MainVol <= 0)
    {
      digitalWrite(DimLEDPin, HIGH);
    } else {
      digitalWrite(DimLEDPin, LOW);
    }*/
   }


  // READ BUTTONS
  SpeakerState = digitalRead(SpeakerBUTPin);
  DimState = digitalRead(DimBUTPin);
  MonoState = digitalRead(MonoBUTPin);

  // SPEAKER B
  if (SpeakerState == HIGH) {
    
    // turn LED on:
    if (SpeakerB == HIGH) {
      digitalWrite(SpeakerLEDPin, LOW);
      SpeakerB = LOW;
    } else {
      digitalWrite(SpeakerLEDPin, HIGH);
      SpeakerB = HIGH;
    }
    
    
    int note = 0x32; // SPEAKER B
    DisplayRefresh();
    //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
    noteOn(0x90, note, 0x7F);
    delay(100);
    
    //Note on channel 1 (0x90), some note value (note), silent velocity (0x00):
    noteOn(0x90, note, 0x00);
    delay(200);
    
  }

  // DIM
   if (DimState == HIGH) {

    if (muted == 0)
    {
      muted = 1;

      // turn LED on:
      digitalWrite(DimLEDPin, HIGH);
      noteOn(0xB8, 0x66, 0);
      delay(200);
      DisplayRefresh();
  
    } else {
      muted = 0;
      digitalWrite(DimLEDPin, LOW);
      noteOn(0xB8, 0x66, MainVol);
      delay(200);
      DisplayRefresh();
    }
  }

  // MONO
   if (MonoState == HIGH) {
    
    // turn LED on:
    //digitalWrite(MonoLEDPin, HIGH);

    if (Mono == HIGH) {
      digitalWrite(MonoLEDPin, LOW);
      Mono = LOW;
    } else {
      digitalWrite(MonoLEDPin, HIGH);
      Mono = HIGH;
    }
    
    int note = 0x2A; // MONO
    
    //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
    noteOn(0x90, note, 0x7F);
    delay(100);
    
    //Note on channel 1 (0x90), some note value (note), silent velocity (0x00):
    noteOn(0x90, note, 0x00);
    delay(200);
    DisplayRefresh();
    
  } else {
    //digitalWrite(MonoLEDPin, LOW);
  }


}


// this is a test brannch