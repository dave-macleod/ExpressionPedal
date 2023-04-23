/*
Name
  GlitchyExpression
Purpose
  Using Arduino ATMega32u4 5v, aka Pro Micro, control an X9C103S 10kOhm digital potentiometer
  
  This acts as an expression pedal for controlling guitar signals with pedals or devices that accept an expression pedal input.
  In my case I am controlling an Electro Harmonix (EHX) Worm.
Physical connections     
  X9C103S connections
    GND to GND
    VCC to VCC
    INC to pin 5
    U/D to pin 6
    CS to pin 7
Author
  Dave MacLeod damacleod@gmail.com
Date
  18/APR/2023
Credits:
  https://electropeak.com/learn/interfacing-x9c103s-10k-digital-potentiometer-module-with-arduino/
  https://randomnerdtutorials.com/arduino-mpu-6050-accelerometer-gyroscope/
  https://arduino.stackexchange.com/a/50225
  https://maker.pro/arduino/tutorial/how-to-clean-up-noisy-sensor-data-with-a-moving-average-filter for the moving average filter code
*/

#include <DigiPotX9Cxxx.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int modeSwitch = 9;
const byte textSize = 3;
DigiPot pot(5,6,7);
const bool debugMode = false;
int potPerc = 0;
unsigned long timer = 0;
unsigned long timerLimit = 400;
bool potHigh = true;
int modeSwitchState = 0;
bool modeSwitchPressed = false;

// Mode 0 - Glitch
// Mode 1 - Binary
// Mode 2 - Rising
// Mode 3 - Falling
int currentMode = 0;

void setup()
{
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))      // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();     // Clear the buffer
  pot.set(0);     // Initialize digital potentiometer
  pinMode(modeSwitch, INPUT_PULLUP);
  randomSeed(analogRead(4));    // use noise from pin 4  to seed random number generation
}

void displayText (char textToDisplay[], int valueToDisplay)
{
  display.clearDisplay();     // Clear the buffer
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,12);
  display.println(textToDisplay);
  display.setCursor(0,30);
  display.println(valueToDisplay);
  display.display(); 
}

void loop()
{
  delay(20);

  modeSwitchState = digitalRead(modeSwitch);      // read switch state

  if (modeSwitchPressed && modeSwitchState == HIGH)   // switch was pressed and now isn't
  {
    currentMode++;
    if (currentMode > 3)
    {
      currentMode = 0;
    }
    modeSwitchPressed = false; 
  }
  
  if (!modeSwitchPressed && modeSwitchState ==LOW) //switch wasn't pressed and now is
  {
    modeSwitchPressed = true;
  }

//  DOUBLE GLITCH --------------------------------------
  if (currentMode == 0)
  {
  if (millis()-timer > timerLimit)
    {
      timer = millis();
      timerLimit = random(100,700);     //Random gap between changes
      potPerc = random(0,100);      //Change to a random value
      pot.set(potPerc);
      displayText("Glitch",potPerc);
    } 
  }

//  ON/OFF --------------------------------------
  if (currentMode == 1)
  {
    if (millis()-timer > timerLimit)
    {
      timer = millis();
      if (potHigh)
      {
        potPerc = 100;
        potHigh = false;
      }
      else
      {
        potPerc = 0;
        potHigh = true;
      }
      pot.set(potPerc);
    displayText("Binary",potPerc);
    }
  }

//  RISING --------------------------------------
  if (currentMode == 2)
  {
    potPerc = ((millis() - timer) * 100 / timerLimit);
    pot.set(potPerc);
    displayText("Rising",potPerc);
    if (millis()-timer > timerLimit)
    {
      timer = millis();
    }
  }
//  FALLING --------------------------------------
  if (currentMode == 3)
  {
    potPerc = 100 - ((millis() - timer) * 100 / timerLimit);
    pot.set(potPerc);
    displayText("Falling",potPerc);
    if (millis()-timer > timerLimit)
    {
      timer = millis();
    }
  }
}




