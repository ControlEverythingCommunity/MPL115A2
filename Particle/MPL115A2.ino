// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// MPL115A2
// This code is designed to work with the MPL115A2_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/products

#include <application.h>
#include <spark_wiring_i2c.h>

// MPL115A2 I2C address is 0x60(96)
#define Addr 0x60

unsigned int data[8];
float a1 = 0.0, b1 = 0.0, b2 = 0.0, c12 = 0.0;
float pressure = 0.0, cTemp = 0.0, fTemp = 0.0;
float presComp = 0.0;
int temp = 0, pres = 0;

void setup()
{
  // Set variable
  Particle.variable("i2cdevice", "MPL115A2");
  Particle.variable("pressure", pressure);
  Particle.variable("cTemp", cTemp);
  
  // Initialise I2C communication
  Wire.begin();
  // Initialise Serial Communication, set baud rate = 9600
  Serial.begin(9600);

  for (int i = 0; i < 8; i++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select data register
    Wire.write(4 + i);
    // Stop I2C Transmission
    Wire.endTransmission();

    // Request 1 byte of data
    Wire.requestFrom(Addr, 1);

    // Read 1 byte of data
    // A10 msb, A10 lsb, Bb1 msb, Bb1 lsb, B2 msb, B2 lsb, C12 msb, C12 lsb
    if (Wire.available() == 1)
    {
      data[i] = Wire.read();
    }
  }

  // Convert the data to floating points
  a1 = ((data[0] * 256.0) + data[1]) / 8.0;
  b1 = ((data[2] * 256) + data[3]);
  if (b1 > 32767)
  {
    b1 -= 65536;
  }
  b1 = b1 / 8192.0;
  b2 = ((data[4] * 256) + data[5]);
  if (b2 > 32767)
  {
    b2 -= 65536;
  }
  b2 = b2 / 16384.0;
  c12 = ((data[6] * 256.0 + data[7]) / 4.0) / 4194304.0;
  delay(300);
}

void loop()
{
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Send Pressure measurement command
  Wire.write(0x12);
  // Start conversion
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(300);

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Request 4 bytes of data
  Wire.requestFrom(Addr, 4);

  // Read 4 bytes of data
  // pres msb, pres lsb, temp msb, temp lsb
  if (Wire.available() == 4)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
  }

  // Convert the data to 10-bits
  pres = (data[0] * 256 + (data[1] & 0xC0)) / 64;
  temp = (data[2] * 256 + (data[3] & 0xC0)) / 64;

  // Calculate pressure compensation
  presComp = a1 + (b1 + c12 * temp) * pres + b2 * temp;

  // Convert the data
  pressure = (65.0 / 1023.0) * presComp + 50.0;
  cTemp = (temp - 498) / (-5.35) + 25.0;
  fTemp = cTemp * 1.8 + 32.0;

  // Output data to dashboard
  Particle.publish("Pressure : ", String(pressure));
  delay(100);
  Particle.publish("Temperature in Celsius : ", String(cTemp));
  delay(100);
  Particle.publish("Temperature in Fahrenheit : ", String(fTemp));
  delay(1000);
}
