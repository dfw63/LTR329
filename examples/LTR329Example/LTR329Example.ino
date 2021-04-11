/*
	LTR329 library example sketch
	dfw63
*/
#include <senseBoxIO.h>       // test was performed on the sensebox. Probably not required on most systems
#include <LTR329.h>
#include <Wire.h>

LTR329 ALS ;

// Global variables:

// Meaning of integration times (input values from 0 to 7, times in ms):
// Input :   0  1   2   3   4   5   6   7
// Value:   100 50 200 400 150 250 300 350
// startup default: 0 (100 ms)

// Meaning of measurementRate (input values from 0 to 7, times in ms):
// Input :   0   1   2   3    4    5    6    7
// Value:   50 100 200 500 1000 2000 2000 2000
// startup default: 3 (500 ms)

byte gain = 7;            // Gain setting, values = 0 through 7
byte integrationTime = 6
; // Integration time setting
byte measurementRate = 4; // Time interval between DATA_REGISTERS update

void setup()
{
  Serial.begin(9600);
  delay(5000);
  Serial.println("LTR329-ALS example sketch");

  // Initialize the LTR329 library
  // 100ms 	initial startup time required
  delay(100);

  if ( ALS.begin() )
    Serial.println("LTR329 started.");
  else
    Serial.println("LTR329 failed");

  delay(100);

  // Read IDs (not required to operate the sensor)
  // 
  if (ALS.getPartID() )
  {
    Serial.print("Sensor Part ID: 0x0");
    Serial.println(ALS.readPartID(), HEX);
    Serial.print("Sensor Revision ID: 0x0");
    Serial.println(ALS.readRevisionID(), HEX);
  }
  // Example for error handling for member functions returning boolean
  // values indicating error conditions:
  else
  {
    byte error = ALS.getError();
    printError(error);
  }

  if (ALS.readManufacID() )
  {
    Serial.print("Manufacturer ID: 0x0");
    Serial.println(ALS.readManufacID(), HEX);
  }
  else
  {
    byte error = ALS.getError();
    printError(error);
  }

  // The sensor has a default integration time of 100ms,
  // and a default gain of low (1X).

  Serial.println("setGain:");
  if ( !ALS.setGain(gain) )
    Serial.println( "setGain failed" );

  Serial.println("Setting integration time and measurement rate");
  ALS.setMeasurementRate(integrationTime, measurementRate);

  // Upon startup, the sensor is in standby mode, so we need the following call
  Serial.println("Activation");
  ALS.activate();   // required:

  // Test of reset function. After reset, gain and times are in their default values and 
  // need to be set to desired values again:
  ALS.reset();
  ALS.setGain(gain);
  ALS.setMeasurementRate(integrationTime, measurementRate);
  ALS.activate();
}

void loop()
{
  // This sketch uses the LTR329's built-in integration timer.
  // Once integration is complete, we'll retrieve the data.
  // Retrieve the data from the device:
  double lux;   // Resulting lux value
  // readlux will wait until a new value is available
  // this happens after the time selected in Measurement Rate
  // values between 50 ms and 2000 ms, default is 500 ms.
  lux = ALS.readLux();
  Serial.print(" lux: ");
  Serial.println(lux);
}

void printError(byte error)
{
  // If there's an I2C error, this function will
  // print out an explanation.

  Serial.print("I2C error: ");
  Serial.print(error, DEC);
  Serial.print(", ");

  switch (error)
  {
    case 0:
      Serial.println("No error");
      break;
    case 1:
      Serial.println("Data too long for transmit buffer");
      break;
    case 2:
      Serial.println("Received NACK on address");
      break;
    case 3:
      Serial.println("Received NACK on data");
      break;
    case 4:
      Serial.println("Other error");
      break;
    default:
      Serial.println("Unknown error");
  }
}
