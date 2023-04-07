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
*/


// Includes, constants and global variables
#include "Wire.h"
#include <MPU6050_light.h>
#include <DigiPotX9Cxxx.h>

MPU6050 mpu(Wire);
DigiPot pot(5,6,7);

const int resetSwitch = 8;
int resetSwitchState = 0;
bool resetSwitchPressed = false;
bool debugMode = true;
float minAngle = 180;
float maxAngle = -180;
float currentAngle = 0;
int potPerc = 0;
unsigned long timer = 0;
unsigned long timerLimit = 3000;
float angX;
float angY;
float angZ;

void setup()
{
  if (debugMode)
  {
    Serial.begin(115200);
    Serial.println("Serial monitor started");
  }  
  pot.set(0);
  Wire.begin();
  pinMode(resetSwitch,INPUT_PULLUP);
  byte status = mpu.begin();
  while(status!=0){ } // stop everything if could not connect to MPU6050
  // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets(); // gyro and accelero offset calculation
}

void loop()
{
  mpu.update();     // read MPU6050
  angX = mpu.getAngleX();
  angY = mpu.getAngleY();
  angZ = mpu.getAngleZ();
  currentAngle = angX;      //change this if need to use different axis

  if (currentAngle > maxAngle)
  {
    maxAngle = currentAngle;     // if current angle exceed the previous max, then set max to current
  }
  if (currentAngle < minAngle)
  {
    minAngle = currentAngle;     // if current angle lower than the previous min, then set min to current
  }

  float currentOffset = currentAngle - minAngle;
  float angleRange = maxAngle-minAngle;
  potPerc = currentOffset * 100 / angleRange;
  pot.set(potPerc);      // Set the pot to the current angle as a percentage of the difference between min and max



  if (debugMode)
  {
    if (millis()-timer > timerLimit)
    {
      timer = millis();
      Serial.print("angX: ");
      Serial.println(angX);
      Serial.print("angY: ");
      Serial.println(angY);
      Serial.print("angZ: ");
      Serial.println(angZ);
      Serial.println(" ");
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
    currentAngle = 0;
    potPerc = 0;
    mpu.calcOffsets(); // gyro and accelero offset calculation
  }

}