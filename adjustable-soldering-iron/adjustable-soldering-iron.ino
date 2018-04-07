//Adjustable soldering iron
//Software written by:
// - Jelmer van Arnhem
// - Camiël vd Niet
// - Sieuwe Elferink
//Copyright (c) 2018 DJO Amersfoort
//MIT Licensed


//Display library
//https://github.com/avishorp/TM1637
#include <TM1637Display.h>
//Temperature library
//https://github.com/adafruit/MAX6675-library
#include <max6675.h>


//Customise your stuff here
double curveValue = 0.5;
int roundTemperatureValue = 5;
int temperatureReadDelay = 500;
int configuredTemperature = 350;


//Pins
int rotaryClockPin = 2;
int rotaryDataPin = 3;
int rotaryToggleButtonPin = 4;
int mosfetPin = 6;
int displayClockPin = 9;
int displayDataPin = 10;
int thermoSOPin = A0;
int thermoCSPin = A1;
int thermoClockPin = A2;


//Other variables
unsigned long temperatureReadTimer;
int valueToDisplay = 0;
int currentTemp = 0;
bool showActualTemp = true;
bool rotaryButtonIsReleased = true;
int rotaryState = 0;
int rotaryPrevState = 0;
int nextEncoderState[4] = { 2, 0, 3, 1 };
int prevEncoderState[4] = { 1, 3, 0, 2 };


//Initialise components
TM1637Display display(displayClockPin, displayDataPin);
MAX6675 thermocouple(thermoClockPin, thermoCSPin, thermoSOPin);


void setup() {
  //Activate the display
  for (byte i = 0; i < 4; i++) {
    display.setSegments(0, 1, i);
  }
  display.setBrightness(0x0C);
  //Activate the pins
  pinMode(mosfetPin, OUTPUT);
  pinMode(rotaryClockPin, INPUT);
  pinMode(rotaryDataPin, INPUT);
  pinMode(rotaryToggleButtonPin, INPUT_PULLUP);
  digitalWrite(mosfetPin, HIGH);
  digitalWrite(rotaryDataPin, HIGH);
  digitalWrite(rotaryClockPin, HIGH);
  //Start temperature timer
  temperatureReadTimer = millis();
}


void loop() {
  //Update the current temperature on a slow timer (not every loop)
  if (millis() > temperatureReadTimer + temperatureReadDelay) {
    updateActualTemp();
  }
  //Toggle between actual current temperature and the configured temperature
  if (digitalRead(rotaryToggleButtonPin) == HIGH) {
    rotaryButtonIsReleased = true;
  } else {
    if (rotaryButtonIsReleased) {
      showActualTemp = !showActualTemp;
      rotaryButtonIsReleased = false;
    }
  }
  //Show the actual current temperature or the configured temperature
  if (showActualTemp) {
    display.showNumberDec(currentTemp);
  } else {
    updateConfiguredTemp();
    display.showNumberDec(configuredTemperature, true);
  }
  //Enable or disable the soldering iron
  if (currentTemp >= configuredTemperature) {
    digitalWrite(mosfetPin, LOW);
  } else {
    digitalWrite(mosfetPin, HIGH);
  }
}


void updateConfiguredTemp() {
  //Read the configured temperature from the rotary
  rotaryState = (digitalRead(rotaryClockPin) << 1) | digitalRead(rotaryDataPin);
  if (rotaryState != rotaryPrevState) {
    if (rotaryState == nextEncoderState[rotaryPrevState]) {
      configuredTemperature = configuredTemperature + 5;
    } else if (rotaryState == prevEncoderState[rotaryPrevState]) {
      configuredTemperature = configuredTemperature - 5;
    }
    if (configuredTemperature > 420) {
      configuredTemperature = 420;
    }
    if (configuredTemperature < 0) {
      configuredTemperature = 0;
    }
    rotaryPrevState = rotaryState;
  }
}


void updateActualTemp() {
  //read and round the temperature
  int celsius = thermocouple.readCelsius();
  currentTemp = round(celsius * curveValue * roundTemperatureValue) / roundTemperatureValue;
  temperatureReadTimer = millis();
}

