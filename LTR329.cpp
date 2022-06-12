/*
	LTR329 (C) 2021 Dirk Wilmer
	
The MIT License (MIT)

Copyright (C) 2021 Dirk Wilmer

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

#include <LTR329.h>
#include <Wire.h>
#include <string>

LTR329::LTR329(void)
{
	// LTR329 object
}

boolean LTR329::begin(void)
{
	// Initialize LTR329 library with default address (0x29)
	// Always returns true

	_gain = 0;
	_valid = true;
	_newData = false;
	Wire.begin();
	getPartID();
	getManufacID();

	if (_partID == 0x0A && _manufacID == 0x05)
	{
		reset();	
		activate();	// required as the sensor is still in standby mode
		readData(); // required since first read always produces wrong data
		return true;
	}
	else
		return false;
}

boolean LTR329::activate()
{
	boolean result = readByte(LTR329_CONTR, _control);
	_control |= 0x01; // set bit 0: Active mode
	result = writeByte(LTR329_CONTR, _control);
	delay(10);
	return result;
}

boolean LTR329::standBy()
{
	boolean result = readByte(LTR329_CONTR, _control);
	_control &= ~0x01; // delete bit 0 -> Standby mode
	result = writeByte(LTR329_CONTR, _control);
	delay(10);
	return result;
}

boolean LTR329::reset()
// SW reset. Note that device will be in Standby mode thereafter. 
{
	boolean result = readByte(LTR329_CONTR, _control);
	{
		_control |= 0x02;  // set bit 1 -> SW reset
	}
	result = writeByte(LTR329_CONTR, _control);
	delay(100);
	return result;
}


boolean LTR329::setGain(byte gain)
{
	// Sets the gain of LTR329
	// Default value is 0x00 (= gain_1)
	// Gain coded in bits 4:2

	boolean result = readByte(LTR329_CONTR, _control);
	// suppress illegal gain values
	if (gain == 4 || gain == 5 || gain > 7)
	{
		gain = gain_1;
	}
	// clear control bits other than gain bits 4:2
	_control &= 0b11;
	// control byte logic: set new gain bits
	_control |= gain << 2;
	result = writeByte(LTR329_CONTR, _control);
	if (result)
		return (getControl());
	return result;
}

boolean LTR329::getControl()
{
	// Gets the control reg values
	// Default value is 0x00 (after hard or soft reset)

	byte reg;

	// Reading the control byte
	if (readByte(LTR329_CONTR, reg))
	{
		// Extract gain
		_gain = (reg & 0x1C) >> 2;

		// Extract reset
		_reset = (reg & 0x02) ? true : false;

		// Extract mode
		_mode = (reg & 0x01) ? true : false;

		// return if successful
		return true;
	}
	return false;
}

boolean LTR329::setMeasurementRate(byte integrationTime, byte measurementRate = 3)
{
	// Sets the integration time and measurement rate of the sensor
	// integrationTime (bits 5:3) is the measurement time for each ALs cycle
	// measurementRate (bits 2:0) is the interval between data register updates
	// measurementRate must be set to be equal or greater than integrationTime
	// Default value is 0x03, i.e., integration time: 100 ms, measurement reate 500 ms

	byte reg = 0x00;

	// allowed values are 0 to 7
	if (integrationTime > 0x07)
	{
		integrationTime = 0x00;
	}

	if (measurementRate > 0x07)
	{
		measurementRate = 0x03;
	}

	reg |= integrationTime << 3;
	reg |= measurementRate;
	_measurementRate = measurementRate;
	_integrationTime = integrationTime;

	return (writeByte(LTR329_MEAS_RATE, reg));
}

boolean LTR329::getMeasurementRate()
{
	// updates measurement repetition and integration times
	// reading from internal register
	// integration time values required for lux calculation

	byte reg = 0x00;

	// Reading the measurement byte
	if (readByte(LTR329_MEAS_RATE, reg))
	{
		// Extract integration Time
		_integrationTime = (reg & 0x38) >> 3;

		// Extract measurement Rate
		_measurementRate = reg & 0x07;

		// return true if successful
		return true;
	}
	return false;
}

boolean LTR329::getPartID()
{
	// Reads the sensor part number ID and revision ID
	// Default value is 0xA0
	// Part number ID = 0x0A
	// Revision ID = 0x00

	byte reg;
	boolean result = readByte(LTR329_PART_ID, reg);
	if (result)
	{
		_partID = (reg & 0xF0) >>4;
		_revisionID = reg & 0x0F;
	}
	return result;
}

boolean LTR329::getManufacID()
{
	// Reads the Manufacturers ID
	// Default value is 0x05 (LITEON)
	byte reg;
	boolean result = readByte(LTR329_MANUFAC_ID, reg);
	if (result)
	{
		_manufacID = reg ;
	}
	return result;
}

byte LTR329::readPartID()
{
	return _partID;
}

byte LTR329::readRevisionID()
{
	return _revisionID;
}

byte LTR329::readManufacID()
{
	return _manufacID;
}

boolean LTR329::isvalid()
{
	return _valid;
}

boolean LTR329::getData()
{
	boolean ret;
	// wait until new data available
	do {
		ret = readStatus();
		delay(10);
	} while (!_newData);

	byte u, v;
	if (!readByte(LTR329_DATA_CH1_0, u) ) return false; 	//data low channel 1
	if (!readByte(LTR329_DATA_CH1_1, v) ) return false; 	//data high channel 1
	_ch1 = u | v << 8;
	
	if (!readByte(LTR329_DATA_CH0_0, u) ) return false; 	//data low channel 0
	if (!readByte(LTR329_DATA_CH0_1, v) ) return false; 	//data high channel 0
	_ch0 = u | v << 8;
		
	if (boolean ret = readStatus()) 
	{
		Serial.print("Gain: "); Serial.print(_gain);
		Serial.print(" Valid: "); Serial.print(_valid);
	}
	
	return true;
}

boolean LTR329::readStatus()
{
	
	byte status = 0x00;

	// Read status byte
	if (readByte(LTR329_STATUS, status))
	{
		// Update validity. 
		_valid = (status & 0x80) ? false : true;

		// Update gain
		_gain = (status & 0x70) >> 4;

		// Update status
		_newData = (status & 0x04) ? true : false;

		// return if successful
		return true;
	}
	return false;
}

boolean LTR329::readData()
{
	// Reads all four data registers (0x88, 0x89, 0x8A, 0x8B) in one go.

	byte high, low;
	boolean ret;
	
	// wait until new data available
	do {
		ret = readStatus();
		delay(10);
	} while (!_newData);

	// Check if sensor present for read
	Wire.beginTransmission(_i2c_address);
	Wire.write(LTR329_DATA_CH1_0);
	_error = Wire.endTransmission();

	// Read two bytes (low and high)
	if (_error == 0)
	{
		Wire.requestFrom(_i2c_address, 4);
		if (Wire.available() == 4)
		{
			low = Wire.read();
			high = Wire.read();
			// Combine bytes into unsigned int
			_ch1 = word(high, low);
			low = Wire.read();
			high = Wire.read();
			_ch0 = word(high, low);
			// return true;
		}
	}
	// check for validity
	ret = readStatus();
	return ret;
}

double LTR329::readLux()
{
	// Reads the raw data by calling readData
	// Converts raw data to lux values
	// For gain settings, see getControl()
	// integrationTime: integration time in ms, from getMeasurementRate()

	double ratio, lux =0.0;

	if ( !readData() )
	{
		return lux;
	}
	
	// if higher gain values resulted in invalid (overflow) data, reset gain to lowest value and read raw data again
	if (!_valid && _gain )
	{
		Serial.print("Reset after invalid data\n");
		reset();
		setMeasurementRate(_integrationTime, _measurementRate);
		activate();
		readData();
	}
	autoGain();
	readData();
	getMeasurementRate(); // update integration time, required for data normalisation

	// The sensor indicates an invalid measurement by setting bit 7 in the control reg 0x8c.
	// This is checked by calling readStatus in the getData member function.

	Serial.print("Gain: ");
	Serial.print(_gain);
	Serial.print(" Valid: ");
	Serial.print(_valid);

	Serial.print(" CH0: ");
	Serial.print(_ch0);
	Serial.print(" CH1: ");
	Serial.print(_ch1);

	if (_valid)
	{
		// lux calculation depends on the ratio of channel results
		ratio = static_cast<double>(_ch1) / (_ch0 + _ch1);
		Serial.print(" Ratio: ");
		Serial.print(ratio);
		Serial.print(" ITime: ");
		Serial.print(_integrationTime);
		Serial.print(" ");

		// calculate normalisation factor
		// depends on gain settings and integration time (100 ms := 1x)
		double factor = 1.0 / _timefactor[_integrationTime];
		factor /= _gainfactor[_gain];

		if (ratio < 0.45)
		{
			lux = (1.7743 * _ch0 + 1.1059 * _ch1) * factor;
			return lux;
		}
		if (ratio < 0.64)
		{
			lux = (4.2785 * _ch0 - 1.9548 * _ch1) * factor;
			return lux;
		}
		if (ratio < 0.85)
		{
			lux = (0.5926 * _ch0 + 0.1185 * _ch1) * factor;
			return lux;
		}
	}
	else // in case of invalid sensor reading:
	{
		Serial.print(" Sum: ");
		Serial.print(_ch1 + _ch0);
		Serial.print(" Overflow: ");
		Serial.print(_ch1 + _ch0 < 0xffff ? false : true);
	}
	return lux;
}

byte LTR329::getError(void)
{
	// Upon command failure, corresonding error codes from the Wire library are stored in _error:
	// 0 = SUCCESS
	// 1 = DATA EXCEEDS BUFFER LENGTH
	// 2 = NACK ON ADDRESS TRANSMIT
	// 3 = NACK ON DATA TRANSMIT
	// 4 = OTHER ERROR
	return (_error);
}

void LTR329::autoGain(void)
{
	uint16_t limits[] = {600, 1300, 8192, 16384, 32768, 65535};
	byte gains[] = {7, 6, 3, 2, 1, 0};

	_gain = gain_1;
	setGain(_gain);
	readData();
	uint16_t chmax = _ch1 + _ch0;

	for (int i = 0; i < 6; i++)
	{
		if (chmax < limits[i])
		{
			_gain = gains[i];
			setGain(_gain);
			readData();
			return;
		}
	}

}
// Private functions:
boolean LTR329::readByte(byte address, byte &value)
{
	// Reads a byte from a LTR329 address

	// Check if sensor present for read
	Wire.beginTransmission(_i2c_address);
	Wire.write(address);
	_error = Wire.endTransmission();

	// Read requested byte
	if (_error == 0)
	{
		Wire.requestFrom(_i2c_address, 1);
		delay(1);
		if (Wire.available() == 1)
		{
			value = Wire.read();
			return true;
		}
	}
	return false;
}

boolean LTR329::writeByte(byte address, byte value)
{
	Wire.beginTransmission(_i2c_address);
	Wire.write(address);
	
	Wire.write(value);
	_error = Wire.endTransmission();
	if (_error == 0)
		return true;

	return false;
}