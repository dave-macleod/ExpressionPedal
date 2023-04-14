/*
Name
  ExpressionPedal
Purpose
  Using Arduino ATMega32u4 5v, aka Pro Micro,
   read an MPU6050 accelerometer
   control an X9C103S 10kOhm digital potentiometer
  
  This acts as an expression pedal for controlling guitar signals with pedals or devices that accept an expression pedal input.
  In my case I am controlling an Electro Harmonix (EHX) Worm.

Physical connections  

  MPU6050 connections
    GND to GND
    VCC to VCC
    SDA to pin 2
    SCL to pin 3
    
  X9C103S connections
    GND to GND
    VCC to VCC
    INC to pin 5
    U/D to pin 6
    CS to pin 7

Author
  Dave MacLeod damacleod@gmail.com
Date
  07/APR/2023
Credits:
  https://electropeak.com/learn/interfacing-x9c103s-10k-digital-potentiometer-module-with-arduino/
  https://randomnerdtutorials.com/arduino-mpu-6050-accelerometer-gyroscope/
  https://arduino.stackexchange.com/a/50225
  https://maker.pro/arduino/tutorial/how-to-clean-up-noisy-sensor-data-with-a-moving-average-filter for the moving average filter code
*/

// Includes, constants and global variables
#include "Wire.h"
#include <MPU6050_light.h>
#include <DigiPotX9Cxxx.h>

// Devices
MPU6050 mpu(Wire);
DigiPot pot(5,6,7);

// Constants
const bool debugMode = true;
const bool invertedExpression = false;
const int resetSwitch = 8;
const int index = 5;

// Global Variables
String plotValue;
int resetSwitchState = 0;
bool resetSwitchPressed = false;
float minAngle = 180;
float maxAngle = -180;
float currentAngle[index];
float currentAngleAvg = 0;
float angX;
float angY;
float angZ;
int potPerc = 0;
unsigned long timer = 0;
unsigned long timerLimit = 100;

void setup()
{
  if (debugMode)
  {
    Serial.begin(115200);
    Serial.println("Serial monitor started");
  }  
  pot.set(0);     // Initialize digital potentiometer
  Wire.begin();     // start comms with the accelerometer/gyroscope
  pinMode(resetSwitch,INPUT_PULLUP);      // initialise the reset switch
  byte status = mpu.begin();      // initialize the accelerometer/gyroscope
  while(status!=0){ } // stop everything if could not connect to MPU6050
  // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets();      // gyro and accelero offset calculation
  currentAngle[1] = 0;
  currentAngle[2] = 0;
  currentAngle[3] = 0;
  currentAngle[4] = 0;
  currentAngle[5] = 0;
}

void loop()
{
  mpu.update();     // read MPU6050
  angX = mpu.getAngleX();
  //angY = mpu.getAngleY();     //change this if need to use different axis
  //angZ = mpu.getAngleZ();     //change this if need to use different axis
  
  // To reduce noise we calulate the linear weighted average of the last 5 angle readings
  currentAngle[5] = currentAngle[4];
  currentAngle[4] = currentAngle[3];
  currentAngle[3] = currentAngle[2];
  currentAngle[2] = currentAngle[1];
  currentAngle[1] = angX;      //change this if need to use different axis
  currentAngleAvg = ( currentAngle[1] + currentAngle[2] + currentAngle[3] + currentAngle[4] + currentAngle[5]  ) / index;

  if (currentAngleAvg > maxAngle)
  {
    maxAngle = currentAngleAvg;     // if current angle exceed the previous max, then set max to current
  }
  if (currentAngleAvg < minAngle)
  {
    minAngle = currentAngleAvg;     // if current angle lower than the previous min, then set min to current
  }

  float currentOffset = currentAngleAvg - minAngle;     // calculate how far (in degress) we are above the minimum
  float angleRange = maxAngle-minAngle;                 // calculate the breadth of the range between min and max
  potPerc = currentOffset * 100 / angleRange;           // the value to set the pot is the %age of the current angle vs the full range 

  if (invertedExpression)     // switch between heel down being min or max
  {
    potPerc = 100 - potPerc;
  }

  pot.set(potPerc);      // Set the pot to the current angle as a percentage of the difference between min and max

  if (debugMode)
  {
    if (millis()-timer > timerLimit)
    {
      timer = millis();
      Serial.print("currentAngle[1]:");
      Serial.print(currentAngle[1]);
      Serial.print(",currentAngle[2]:");
      Serial.print(currentAngle[2]);
      Serial.print(",currentAngle[3]:");
      Serial.print(currentAngle[3]);
      Serial.print(",currentAngle[4]:");
      Serial.print(currentAngle[4]);
      Serial.print(",currentAngle[5]:");
      Serial.print(currentAngle[5]);
      Serial.print(",potPerc:");
      Serial.println(potPerc);
    }
  }

  /* Reset */
  resetSwitchState = digitalRead(resetSwitch);
  if (resetSwitchPressed && resetSwitchState == HIGH)   // switch was pressed and now isn't
  {
    resetSwitchPressed = false; 
  }
  if (!resetSwitchPressed && resetSwitchState ==LOW) //switch wasn't pressed and now is
  {
    resetSwitchPressed = true;
    if (debugMode)
    {
      Serial.println("Reset");
    }
    minAngle = 180;
    maxAngle = -180;
    currentAngle[1] = 0;
    potPerc = 0;
    mpu.calcOffsets(); // gyro and accelero offset calculation
  }
}