# GAW_Measure

## Purpose
This setup is used to monitor 2 power supplies and a DCC command station, all of which are to be used on my model railroad layout. This way I can monitor them to see whether more capacity is needed.

## Materials
- Arduino Nano
- ACS712M5 :: current sensor for max 5 Amps (3 times)
- Resistors :: 1 x 82K, 1 x 56K
- LCD1602-I2C :: I2C lcd display, two lines of 16 characters (3 times)
- Mean Well :: power supply 15W 5V/3A
- Mean Well :: power supply 36W 12V/3A

## Description
The Arduino is the heart of this measuring setup. It has it's ***own power supply*** of around 5 Volts. Do not use the 5 Volt supply that is connected to the layout. The current sensors and the LCD displays are also connected to this separate power supply.

### Connect power supplies
The power supplies are set up to be connected to the model railroad layout. The ground wire of both supplies are also connected to the ground of the arduino to have a reference for the voltage measurements.

The 5 Volt connection of the Layout power supply can be connected to one of the analog pins. MAKE SURE it does not supply much more than 5 volts! Mine delivers 5.25 Volts.

The 12 Volt connection of the Layout power supply cannot be connected directly to one of the analog pins. It has to be divided by the 2 resistors. Connect the 56K resistor to ground and the other side to the output of the 12 Volt power supply. Feed the Arduino analog pin to measure the 12 Volt to the middels of the voltage divider this should give us around 5 Volts. MAKE SURE this does not supply much more than 5 volts!

### Connect the current sensors
The current sensors are connected in-line with the leads to the layout. I use the positive leads to run through the current sensors. Test these to be sure that you don't get negative readings for the currents. When you do, switch the two wires on the sensor.

The other side of the sensors have 3 wires:
- VCC : self-explanatory
- OUT : to be connected to one of the analog pins of the Arduino
- GND : self-explanatory

## Schematic
A schematic will follow

## Calibration

### Voltages
Be sure to measure the voltages of the power supplies (5 Volt and 12 Volt) exactly with a good measuring instrument that gives you reliability up to two digits. The Voltage of the separate 5 Volt supply for the arduino has to be measured with the connected devices: Arduino, sensors and displays. Insert those values in the following code statements:

float myArduinoVoltage = ***4910.0***; // Voltage from Arduino pwr supply
                                            //   in milliVolts

float myVoltage_05V = ***5.25***;  // measured voltage from 5V supply

float myVoltage_12V = ***12.33***; // measured voltage from 12V supply


### Currents
I found it rather difficult to calibrate the current sensors. The Separate Arduino power supply is now connected to the 5V pin instead of the Vin pin. This eliminates the earlier problem of wildly different current indications dpending on whether the Arduino was connected to the USB cable or not.

To get proper readings the **ACSoffset** variable in the Arduino sketch is important. I had to fiddle with that to get good results.

float ACSoffset = ***2465***;  // Offset for current sensors

