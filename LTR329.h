/*
	LTR329 (C) Dirk Wilmer 2021
	
The MIT License (MIT)

Copyright (c) 2021 Dirk Wilmer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef LTR329_h
#define LTR329_h

#include "Arduino.h"

#define LTR329_ADDR 0x29 // default address

// LTR329 register addresses
#define LTR329_CONTR 		0x80
#define LTR329_MEAS_RATE	0x85
#define LTR329_PART_ID 		0x86
#define LTR329_MANUFAC_ID 	0x87
#define LTR329_DATA_CH1_0 	0x88
#define LTR329_DATA_CH1_1 	0x89
#define LTR329_DATA_CH0_0 	0x8A
#define LTR329_DATA_CH0_1 	0x8B
#define LTR329_STATUS 		0x8C


const byte _i2c_address = LTR329_ADDR;
const byte gain_1 = 0x00;
const byte gain_2 = 0x01;
const byte gain_4 = 0x02;
const byte gain_8 = 0x03;
const byte gain_48 = 0x06;
const byte gain_96 = 0x07;

class LTR329
{
public:
	LTR329(void);
	// LTR329 object

	boolean begin(void);
	// Initialize LTR329 library with default address (0x29)
	// returns true if the device could be identified, false otherwise.

	boolean activate( void );
	// switches device to Active mode.
	// returns false in case of I2C error, true if successful

	boolean standBy( void );
	// switches device to Stand-by mode.
	// returns false in case of I2C error, true if successful

	boolean reset( void );
	// SW reset of device. Gain, mode and time settings will be back to default values, i.e.,
	// Gain: 0 (1x)
	// Mode: Stand-by
	// integration time: 0 (100 ms)
	// repetition time: 3 (500 ms)
	// returns false in case of I2C error, true if successful

	boolean setGain(byte gain);
	// Sets the gain
	// gain: bits 4:2
	// returns false in case of I2C error, true if successful

	boolean getControl();
	// reads the device control register and updates the private class data
	// returns false in case of I2C error, true if successful

	boolean setMeasurementRate(byte integrationTime, byte measurementRate);
	// Sets the integration time and measurement rate of the sensor
	// integrationTime is the measurement time for each ALs cycle
	// measurementRate is the interval between DATA_REGISTERS update
	// measurementRate must be set to be equal or greater than integrationTime
	// Default value at startup is 0x03
	// returns false in case of I2C error, true if successful

	boolean getMeasurementRate( void );
	// Gets the value of Measurement Rate register
	// Updates internal object parameters
	// returns false in case of I2C error, true if successful

	boolean getPartID( void );
	// Gets the part number ID and revision ID of the chip
	// Default value is 0xA0
	// part number ID = 0x0A (default)
	// Revision ID = 0x00
	// returns false in case of I2C error, true if successful

	byte readPartID( void );
	// Returns the PartID

	byte readRevisionID( void );
	// returns the Revision ID;

	boolean getManufacID();
	// Gets the Manufacturers ID
	// Default value is 0x05
	// returns false in case of I2C error, true if successful

	byte readManufacID( void );
	// returns the Manufacturer ID

	boolean isvalid( void ); 
	// returns the validity status of measurement.

	boolean getData();
	// Gets the 16-bit channel 0 and channel 1 data, stores in _ch0 and _ch1
	// returns false in case of I2C error, true if successful

	boolean readStatus();
	// Reads status information of LTR329
	// returns false in case of I2C error, true if successful

	boolean readData();
	// Reads all four data registers 0x88, 0x89, 0x8A, 0x8B
	// and stores values in _ch1 and _ch0
	// returns false in case of I2C error, true if successful

	double readLux();
	// Convert raw data to lux
	// calls member function to read raw data
	// returns 0.0 for invalid data

	void autoGain();
	// selects best gain value for the most recent lux value

	byte getError(void);
	// Upon command failure, corresponding error codes from the Wire library are stored in _error:
	// 0 = SUCCESS
	// 1 = DATA EXCEEDS BUFFER LENGTH
	// 2 = NACK ON ADDRESS TRANSMIT
	// 3 = NACK ON DATA TRANSMIT
	// 4 = OTHER ERROR

private:
	boolean readByte(byte address, byte &value);
	// Reads a byte from an LTR329 address
	// returns false in case of I2C error, true if successful

	boolean writeByte(byte address, byte value);
	// Write a byte to a LTR329 address
	// returns false in case of I2C error, true if successful

	const double _gainfactor[8] = { 1.0, 2.0, 4.0, 8.0, 0.0, 0.0, 48.0, 96.0};
	const double _timefactor[8] = { 1, 0.5, 2.0, 4.0, 1.5, 2.5, 3.0, 3.5 };
	const double _intTimes[8] = { 100.0, 50.0, 200.0, 400.0, 150.0, 250.0, 300.0, 350.0 };
	const byte intTimes_index[8] = { 1, 0, 4, 2, 5, 6, 7, 4};
	const double _measTimes[8] = { 50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 2000.0, 2000.0 };

	uint16_t _ch0, _ch1;
	byte _error;
	byte _gain;
	byte _control;
	byte _partID;
	byte _revisionID;
	byte _manufacID;
	byte _integrationTime;
	byte _measurementRate;
	boolean _valid;
	boolean _mode;
	boolean _reset;
	boolean _newData;
	boolean _autogain;
};

#endif