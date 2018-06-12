# IR_Theramin

This is a rudimentary theremin to make pitched noise based on where
you hold your hand above a distance sensor.

It works using the Simblee™ RFD77402 IoT 3D ToF Sensor Module
for infrared distance measurement

There are 3 parts to this code:
1. Distance-only mode. It prints the distance the sensor is reading,
plays no sound. The distance measurements are supposedly in mm, but
they are wildly inaccurate, although they vary linearly based on
distance as they should.
2. PWM mode. Uses a line drawing function to approximate a pwm for the
speaker. This produces a smooth curve of pitch, but not in the
register that we wanted. 
3. Custom pitch mode. Uses single function to play pitch based on
distance with no interrupts used. This is a little choppier than 

Uses Raspberry Pi B+ with the following  pin connections to the
sensor and speaker

GPIO 26 - Speaker output

GPIO02 - Sensor SDA

GPIO03 - Sensor SCL

