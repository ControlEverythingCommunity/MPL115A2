# Distributed with a free-will license.
# Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
# MPL115A2
# This code is designed to work with the MPL115A2_I2CS I2C Mini Module available from ControlEverything.com.
# https://www.controleverything.com/products

import smbus
import time

# Get I2C bus
bus = smbus.SMBus(1)

# MPL115A2 address, 0x60(96)
# Reading Coefficents for compensation
# Read data back from 0x04(04), 8 bytes
# A0 MSB, A0 LSB, B1 MSB, B1 LSB, B2 MSB, B2 LSB, C12 MSB, C12 LSB
data = bus.read_i2c_block_data(0x60, 0x04, 8)

# Convert the data to floating points
A0 = (data[0] * 256 + data[1]) / 8.0
B1 = (data[2] * 256 + data[3])
if B1 > 32767 :
	B1 -= 65536
B1 = B1 / 8192.0
B2 = (data[4] * 256 + data[5])
if B2 > 32767 :
	B2 -= 65535
B2 = B2 / 16384.0
C12 = ((data[6] * 256 + data[7]) / 4) / 4194304.0

# MPL115A2 address, 0x60(96)
# Send Pressure measurement command, 0x12(18)
#		0x00(00)	Start conversion
bus.write_byte_data(0x60, 0x12, 0x00)

time.sleep(0.5)

# MPL115A2 address, 0x60(96)
# Read data back from 0x00(00), 4 bytes
# pres MSB, pres LSB, temp MSB, temp LSB
data = bus.read_i2c_block_data(0x60, 0x00, 4)

# Convert the data to 10-bits
pres = ((data[0] * 256) + (data[1] & 0xC0)) / 64
temp = ((data[2] * 256) + (data[3] & 0xC0)) / 64

# Calculate pressure compensation
presComp = A0 + (B1 + C12 * temp) * pres + B2 * temp

# Convert the data
pressure = (65.0 / 1023.0) * presComp + 50
cTemp = (temp - 498) / (-5.35) + 25
fTemp = cTemp * 1.8 + 32

# Output data to screen
print "Pressure : %.2f kPa" %pressure
print "Temperature in Celsius : %.2f C" %cTemp
print "Temperature in Fahrenheit : %.2f F" % fTemp
