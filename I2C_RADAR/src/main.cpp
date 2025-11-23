#include <Arduino.h>
#include <Wire.h>

void scanI2C(void)
{
	byte error, address;
	int nDevices;
	uint8_t r;

	//  ======  Scan I2C Address Space  ====================================
	Serial.println("Scanning all I2C address seen ...");
	nDevices = 0;
	for (address = 1; address <= 127; address++)
	{
	  // The i2c_scanner uses the return value of
	  // the Write.endTransmisstion to see if
	  // a device did acknowledge to the address.
	  Wire.beginTransmission(address);
	  error = Wire.endTransmission();

    switch(error) {
      case 0:           //OK
        Serial.printf("0x%x -- I2C device found\n", address);
        nDevices++;
        break;
      case 2:           //Fail
        break;
      case 5:           //TIMEOUT
	      Serial.printf("0x%x -- Timeout\n", address);
        break;
      case 4:           //Unknown
	      Serial.printf("0x%x -- Unknown error\n", address);
        break;
    }
	}

	if (nDevices == 0)
	  Serial.println("No I2C devices found\n");
	else
	  Serial.println("done\n");
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(1);

  Wire.begin();

  scanI2C();


}

void loop()
{
}