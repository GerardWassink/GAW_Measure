# GAW_Measure

## Purpose
This setup is used to monitor 2 power supplies and a DCC command station, all of which are to be used on my model railroad layout. This way I can monitor them to see whether more capacity is needed.

## Materials
- 1 x Arduino Nano
- 3 x ACS712M5 :: current sensor for max 5 Amps
- Resistors :: 1 x 82K, 1 x 56K, 2 x 1K, 1 x 330
- 3 x LED :: in my case: Blue (DCC), Groen (12V), Red (5V)
- 1 x LCD1602-I2C :: I2C lcd display, four lines of 20 characters
- 1 x push button for reset over-current condition
- Mean Well :: power supply 15W 5V/3A
- Mean Well :: power supply 36W 12V/3A

## Description
The Arduino is the heart of this measuring setup. It has it's ***own power supply*** of around 5 Volts. Do not use the 5 Volt supply that is connected to the layout. The current sensors and the LCD display are also connected to this separate power supply.

### Connect power supplies
The power supplies are set up to be connected to the model railroad layout. The ground wire of the 5V and 12V supplies are also connected to the ground of the arduino to have a reference for the voltage measurements.

#### 5 Volt layout power supply
The 5 Volt connection of the Layout power supply can be connected to one of the analog pins. MAKE SURE it does not supply much more than 5 volts! Mine delivers 5.25 Volts.

#### 12 Volt layout power supply
The 12 Volt connection of the Layout power supply **cannot** be connected directly to one of the analog pins. It has to be divided by the 82K and 56K resistors. Connect the 56K resistor to ground and to the 82K resistor. The other end of the 82K resistor goes to the output of the 12 Volt power supply. Feed the Arduino analog pin to measure the 12 Volt to the middle of the voltage divider this should give us around 5 Volts. MAKE SURE this does not supply much more than 5 volts! Measure twice, connect once!

#### Separate Arduino 5 Volt power supply
Connect the negative side to the GND pin of the Arduino and, after careful measurements, connect the positive side to the 5V pin of the Arduino.

### Connect the current sensors
The current sensors are connected in-line with the leads to the layout. I use the positive leads to run through the current sensors. Test these to be sure that you don't get negative readings for the currents. When you do, switch the two wires on the sensor.

The other side of the sensors have 3 wires:
- VCC : self-explanatory
- OUT : to be connected to the analog pins of the Arduino
- GND : self-explanatory

## Schematic
A schematic will follow

## Calibration

### Voltages
Be sure to measure the voltages of the layout power supplies (5 Volt and 12 Volt) exactly with a good measuring instrument that gives you reliability up to two digits. Measure the voltage of the separate 5 Volt supply for the Arduino while it's peripherals are connected: Arduino, sensors and display. Insert those values in the following code statements:

float myArduinoVoltage = ***4910.0***; // Voltage from Arduino pwr supply
in milliVolts

float myVoltage_05V = ***5.25***;  // measured voltage from 5V supply

float myVoltage_12V = ***12.33***; // measured voltage from 12V supply


### Currents
I takes a bit of trial and error to calibrate the current sensors. To get proper readings the **ACSoffset** variable in the Arduino sketch is important. I had to fiddle with that to get good results.

float ACSoffset = ***2460***;  // Offset for current sensors

YMMV...
