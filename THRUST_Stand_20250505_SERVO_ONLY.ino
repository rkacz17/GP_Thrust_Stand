// Thrust Stand Code v1
// Ryan Kaczmarczyk et al.
// Team 517- FAMU-FSU College of Engineering
// 2025-03-20 CE


// Initialize Libraries
#include <Wire.h>               // I2C Interface Library
#include <LiquidCrystal_I2C.h>  // Library for LCD
#include <Servo.h>              // Servo Library
#include <Stepper.h>            // Stepper Library
#include <SPI.h>                // Serial and Peripheral Interface Guide
#include <SD.h>                 // SD Card Library

// FORCE SENSOR
#define FS_pin A0                    // Force Sensor Button Pin
#define bit_resolution 5 / 0.015625  // resolution of 14 bit ADC

// EMERGENCY BUTTON
#define button_EM 9  // emergency stop button pin

// STEPPER MOTOR
#define ST_P1 2  // stepper pin
#define ST_P2 3  // stepper pin
#define ST_P3 4  // stepper pin
#define ST_P4 5  // stepper pin

#define stepsPerRevolution 200  // change this to fit the number of steps per revolution

// SERVO MOTOR
#define SRV_P 6  // servo signal pin

// NAVIGATION BUTTONS
#define button_L_pin 7  // left control button pin
#define button_R_pin 8  // right control button pin

// SD CARD


// SDO - pin 11

// SDI - pin 12

// CLK - pin 13

// CS - pin 10 (likely)


// DEFINE VARIABLES

// SD String
String dataString = "";

// State
int state = 0;  // state variable
// int state = 99;  // state variable

// Force Sensor Processing
float FS_cali;   // Calibration Value (bits)
int FS_zero;     // Force Sensor Reading with Preload (zero value)
float FS_volts;  // Force Sensor Reading in volts
float FS_mN;     // Force Sensor Reading in mN

int FS_time_start;  // starting time of Force Test


const int chipSelect = 10;  // CS PIN for SD Card

// Declare LCD

LiquidCrystal_I2C lcd(0x27, 20, 4);  // I2C address 0x27, 20 column and 4 rows

// Declare Stepper
Stepper myStepper(stepsPerRevolution, ST_P1, ST_P2, ST_P3, ST_P4);

// Declare Servo
Servo myservo;  // create Servo object to control a servo


void setup() {

  // LCD Initialization
  lcd.init();       //initialize the lcd
  lcd.backlight();  //open the backlight
  lcd.clear();

  // Button Initialization
  pinMode(button_L_pin, INPUT_PULLUP);
  pinMode(button_R_pin, INPUT_PULLUP);
  pinMode(button_EM, INPUT_PULLUP);


  // Servo Initialization
  myservo.attach(SRV_P);  // attaches the servo on pin 9 to the Servo object
  myservo.write(60);     // sets servo to vertical position

  // Stepper Initialization
  myStepper.setSpeed(200);  //rpm


  // Force Sensor Initialization
  pinMode(FS_pin, INPUT);
  analogReadResolution(14);
  //Serial.begin();
  delay(2000);
  if (!SD.begin(chipSelect)) {
          lcd.setCursor(0, 3);
          lcd.print("NO SD");
        } else {
          lcd.setCursor(0, 3);
          lcd.print("SD RDY");
        }
  delay(2000);
  lcd.clear();
myStepper.step(72);
}

void loop() {
  // put your main code here, to run repeatedly:

  int button_L = digitalRead(button_L_pin);
  int button_R = digitalRead(button_R_pin);


  switch (state) {
    case 0:  // Startup
      {
        lcd.setCursor(2, 0);
        lcd.print("THRUST STAND  v1");
        lcd.setCursor(5, 1);
        lcd.print("NASA  MSFC");
        //lcd.setCursor(2, 2);
        //lcd.print("MADE BY TEAM 517");
        lcd.setCursor(11, 3);
        lcd.print("Calibrate");

        

        if (button_R == 0) {
          state = 1;
          delay(400);
          lcd.clear();
        }
      }
      break;

    case 1:  // calibration start screen

      lcd.setCursor(4, 0);
      lcd.print("CALIBRATION");
      //lcd.setCursor(5, 1);
      //lcd.print("1. PULL VACUUM 2. START");
      lcd.setCursor(0, 3);
      lcd.print("Back           Start");

      if (button_R == 1 && button_L == 0) {  // Back (Nav)
        state = 0;
        lcd.clear();
        delay(400);
      }

      if (button_R == 0 && button_L == 1) {  // Start Calibration (Nav)
        state = 2;
        delay(400);
        lcd.clear();
      }

      break;

    case 2:  // run Calibration sequence

      FS_zero = analogRead(FS_pin);  // Force Sensor Reading with Preload (zero value)
      delay(300);

      myservo.write(20);
      delay(9000);
      FS_cali = (53.812 * 9.81) / ((analogRead(FS_pin) - FS_zero) * bit_resolution);  // Calibration Value with Weight
      delay(1000);
      myservo.write(50);
      delay(1000);
      state = 3;
      break;
    case 3:  // Start Force Reading Screen

      lcd.setCursor(4, 0);
      lcd.print("READY TO TEST");
      lcd.setCursor(0, 3);
      lcd.print("Cancel         Start");

      if (button_R == 1 && button_L == 0) {  // Cancel (Nav)
        state = 0;
        lcd.clear();
        delay(400);
      } else if (button_R == 0 && button_L == 1) {  // Start Force Reading (Nav)
        state = 4;
        lcd.clear();
        delay(400);
      }
      break;

    case 4: // Force Sensor Test
      FS_time_start = micros();  
      File dataFile = SD.open("testData.csv", FILE_WRITE);
      dataFile.println(dataString);
      dataFile.println(dataString);
      dataFile.println("t (microseconds), bit-value voltage, approx. Force (mN)")

      while (millis() - FS_time_start/1000 < 30000) {  // 30 second test time
        FS_volts = (analogRead(FS_pin) - FS_zero) * bit_resolution;
        FS_mN = FS_volts * FS_cali;  // mN
        

        dataString += String(micros() - FS_time_start); // timestamp (microseconds)
        dataString += ",";
        dataString += String(analogRead(FS_pin)); // bit value of voltage reading
        dataString += ",";
        dataString += String(FS_mN); // approx. force value
        dataString += ",";
        dataFile.println(dataString);
        dataString = "";
      }
      dataFile.close();
      state = 0;
      break;
  }  // end of state




}  // end of main loop

// Calibration Sequence function

void place(int steps) {
  // Place
  lcd.setCursor(5, 0);
  lcd.print("Placing...");
  myservo.write(40);
  myStepper.step(steps);
  delay(500);
  myservo.write(60);
  delay(500);
  myStepper.step(-steps);
  myservo.write(42);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Waiting...");
}

void pick(int steps) {
  // Pick
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Picking...");

  myservo.write(42);
  myStepper.step(steps);
  delay(500);
  myservo.write(30);
  delay(500);
  myStepper.step(-steps);
  lcd.clear();
}