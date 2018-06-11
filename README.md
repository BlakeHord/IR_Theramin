# IR_Theramin

Works with Simblee™ RFD77402 IoT 3D ToF Sensor Module
(Infrared Distance Measurement)

A basic theremin to play a range of sounds based on the distance
measurement from the sensor.

3 Parts to this code:
1. Distance-only mode. Prints the distance the sensor is reading,
plays no sound
2. PWM mode. Uses a line drawing function to approximate a pwm for the
speaker.
3. Custom pitch mode. Uses single function to play pitch based on
distance with no interrupts used.

Uses Raspberry Pi B+
pin 21 - trigger on sensor
pin 20 - echo on sensor
pin 26 - speaker output

