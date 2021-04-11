
# LTR329 Arduino Library

---
Arduino Library for the LTR329ALS ambient light sensor

## Repository Contents

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/** - Source files for the library (.cpp, .h).
* **library.properties** - General library properties for the Arduino package manager.
* **library.json** - General library properties for the Arduino package manager in JSON format
* **keywords.txt** - Contains the keywords for Arduino IDE.

## Conclusions from testing the sensor

* Reading the four sensor registers should be done after testing the status bit by reading the status register 0x8c - reading should only take place if new data have been collected
* After reading the four data registers (0x88, 0x89, 8x8a, 0x8b), the status register should be checked again for validity
* The status register may indicate invalid data even if the registers do not have any overflow condition
* It seems that the data are flagged as invalid as as the sum of both data channels (CH0 + CH1) exceeds 0xFFFF.
* After setting the reset bit in the control register, the device will be set to Stand-by mode
* Subsequently, the mode bit in the control register needs to be set to restart the device.

## References
1. Data Sheet: https://optoelectronics.liteon.com/upload/download/DS86-2014-0006/LTR-329ALS-01_DS_V1.5.PDF

2. Python version: https://github.com/mcqn/pycom-libraries/blob/master/pysense/lib/LTR329ALS01.py (includes lux calculation according to Appendix A)


## TODO:
* Some kind of auto-ranging?

