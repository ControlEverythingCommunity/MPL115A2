// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// MPL115A2
// This code is designed to work with the MPL115A2_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/products

import com.pi4j.io.i2c.I2CBus;
import com.pi4j.io.i2c.I2CDevice;
import com.pi4j.io.i2c.I2CFactory;
import java.io.IOException;

public class MPL115A2
{
	public static void main(String args[]) throws Exception
	{
		// Create I2C bus
		I2CBus Bus = I2CFactory.getInstance(I2CBus.BUS_1);
		// Get I2C device, MPL115A2 I2C address is 0x60(96)
		I2CDevice device = Bus.getDevice(0x60);

		// Reading Coefficents for compensation
		// Read 8 bytes of data from address 0x04(04)
		// A0 msb, A0 lsb, B1 msb, B1 lsb, B2 msb, B2 lsb, C12 msb, C12 lsb
		byte[] data = new byte[8];
		device.read(0x04, data, 0, 8);

		// Convert the data to floating points
		double A0 = (data[0] * 256 + data[1]) / 8.0;
		double B1 = (data[2] * 256 + data[3]) / 8192.0;
		double B2 = (data[4] * 256 + data[5]) / 16384.0;
		double C12 = ((data[6] * 256 + data[7]) / 4) / 4194304.0;

		// Send Pressure measurement command
		device.write(0x12, (byte)0x00);
		Thread.sleep(500);

		// Read 4 bytes of data from address 0x00(00)
		// pres msb, pres lsb, temp msb, temp lsb
		device.read(0x00, data, 0, 4);

		// Convert the data to 10-bits
		int pres = ((data[0] & 0xFF) * 256 + (data[1] & 0xC0)) / 64;
		int temp = ((data[2] & 0xFF) * 256 + (data[3] & 0xC0)) / 64;

		// Calculate pressure compensation
		double presComp = A0 + (B1 + C12 * temp) * pres + B2 * temp;

		// Convert the data
		double pressure = (65.0 / 1023.0) * presComp + 50;
		double cTemp = (temp - 498) / (-5.35) + 25;
		double fTemp = cTemp * 1.8 + 32;

		// Output data to screen
		System.out.printf("Pressure : %.2f kPa %n", pressure);
		System.out.printf("Temperature in Celsius : %.2f C %n", cTemp);
		System.out.printf("Temperature in Fahrenheit : %.2f F %n", fTemp); 
	}
}
