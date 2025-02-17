/* ------------------------------------------------------------------------- *
 * Name   : GAW-measure
 * Author : Gerard Wassink
 * Date   : February 2025
 * Purpose: Monitor Voltage and current for two power supplies (5V and 12V)
 *          and monitor current for a DCC track supply (Roco Z21)
 * Versions:
 *   0.1  : Initial code base               
 *   0.2  : Code cleanup and comments
 *   0.3  : Code improvements
 *   0.4  : calibration completed
 *   1.0  : Code cleanup and comments, first release
 *   1.1  : Minor most textual improvements
 *   1.2  : Solved different readings when connected or not connected to USB
 *            by connecting the separate Arduino power supply to the 5V pin
 *            Carefull, this may absolutely not be much more than 5 volts!
 *   1.3  : Changed README file
 *
 *------------------------------------------------------------------------- */
#define progVersion "1.3"              // Program version definition
/* ------------------------------------------------------------------------- *
 *             GNU LICENSE CONDITIONS
 * ------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * ------------------------------------------------------------------------- *
 *       Copyright (C) May 2024 Gerard Wassink
 * ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- *
 *                                                        DEBUGGING ON / OFF
 * Compiler directives to switch debugging on / off
 * Do not enable DEBUG when not needed, Serial coms takes space and time!
 * ------------------------------------------------------------------------- */
#define DEBUG 0

#if DEBUG == 1
  #define debugstart(x) Serial.begin(x)
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
#else
  #define debugstart(x)
  #define debug(x)
  #define debugln(x)
#endif


/* ------------------------------------------------------------------------- *
 *                                             Pin definitions for measuring
 * ------------------------------------------------------------------------- */
#define V5VoltagePin  A0
#define V05CurrentPin A1

#define V12VoltagePin A2
#define V12CurrentPin A3

#define DccCurrentPin A7


/* ------------------------------------------------------------------------- *
 *                                             Include headers for libraries
 * ------------------------------------------------------------------------- */
#include <Wire.h>                           // I2C comms library
#include <LiquidCrystal_I2C.h>              // LCD library


/* ------------------------------------------------------------------------- *
 *                                        Create objects for the LCD screens
 * ------------------------------------------------------------------------- */
LiquidCrystal_I2C display1(0x25, 16, 2);    // Initialize display 1 for 5V
LiquidCrystal_I2C display2(0x26, 16, 2);    // Initialize display 2 for 12V
LiquidCrystal_I2C display3(0x27, 16, 2);    // Initialize display 3 for DCC


/* ------------------------------------------------------------------------- *
 *                                  Constants and variables for measurements
 * ------------------------------------------------------------------------- */
const int count = 10;                       // how many samples to average
int       sum = 0;                          // variable holds sum
const int sensitivity = 185;                // for 5A current sensors
int       rawValue = 0;                     // Value read from Analog pins
float     Voltage  = 0;                     // used to calculate
float     milliVolts  = 0;                  // used to calculate
float     Current  = 0;                     // used to calculate


/* ------------------------------------------------------------------------- *
 *           Measured voltages for Arduino supply, 5V and 12V power supplies
 * ------------------------------------------------------------------------- */
float myArduinoVoltage = 4910.0;            // Voltage from Arduino pwr supply
                                            //   in milliVolts
float myVoltage_05V = 5.25;                 // measured voltage from 5V supply
float myVoltage_12V = 12.33;                // measured voltage from 12V supply


/* ------------------------------------------------------------------------- *
 *                                      Experimentally established ACSoffset
 * ------------------------------------------------------------------------- */
float ACSoffset = 2465;                     // Offset for current sensors
                                            //   expermimentally established


/* ------------------------------------------------------------------------- *
 *                                       Buffers to build values for display
 * ------------------------------------------------------------------------- */
char strBuf1[20];                           // stringbuffers to build up
char strBuf2[20];                           // numbers for display



/* ------------------------------------------------------------------------- *
 *                                          Initialization routine - setup()
 * ------------------------------------------------------------------------- */
void setup() {
  debugstart(115000);                       // make debug output fast
                                            //   don't forget to set monitor 
                                            //     to the same speed!

  Wire.begin();                             // Start I2C

                                            // Initialize displays
                                            //   backlights on by default
  display1.init(); display1.backlight();
  display2.init(); display2.backlight();
  display3.init(); display3.backlight();

                                            // Initial text on all displays
  LCD_display(display1, 0, 0, F("GAW Measure     "));
  LCD_display(display1, 1, 0, F("Version:        "));
  LCD_display(display1, 1, 9, String(progVersion));

  LCD_display(display2, 0, 0, F("GAW Measure     "));
  LCD_display(display2, 1, 0, F("Version:        "));
  LCD_display(display2, 1, 9, String(progVersion));

  LCD_display(display3, 0, 0, F("GAW Measure     "));
  LCD_display(display3, 1, 0, F("Version:        "));
  LCD_display(display3, 1, 9, String(progVersion));


#if DEBUG == 0                              // When not debugging
  delay(3000);                              //   time to read banner
#endif

}



/* ------------------------------------------------------------------------- *
 *                                                   Repeating loop - loop()
 * ------------------------------------------------------------------------- */
void loop() {

  monitor5V();

  monitor12V();

  monitorDCC();

  debugln(" ");                             // When debugging, 'print' newline

  delay(200);                               // Monitor every 100 milliseconds
}


/* ------------------------------------------------------------------------- *
 *       Monitor the 5V power supply                             monitor5V()
 * ------------------------------------------------------------------------- */
void monitor5V() {

  debug(" --5V-- ");

  rawValue = analogRead(V5VoltagePin);      // pin for 5V voltage
  Voltage = rawValue * myVoltage_05V / 1023;
  dtostrf( Voltage, 6, 2, strBuf1);

  #if DEBUG == 1
    show_V_Values();
  #endif

  sum = 0;                  // Calculate average over 'count' measurements
  for (int i=0; i<count; i++) {
    sum += analogRead(V05CurrentPin);       // pin for 5V current
  }
  rawValue = sum / count;                   // Calc average
  milliVolts = (rawValue / 1023.0) * myArduinoVoltage;
  Current = ( ( milliVolts - ACSoffset )  / (sensitivity) );
  dtostrf( Current, 6, 2, strBuf2);

  #if DEBUG == 1
    show_A_Values();
  #endif

  LCD_display(display1, 0, 0, F("Voeding 5 Volt--"));
  LCD_display(display1, 1, 0,  strBuf1 );
  LCD_display(display1, 1, 6,  "V" );
  LCD_display(display1, 1, 9,  strBuf2 );
  LCD_display(display1, 1,15,  "A" );
}


/* ------------------------------------------------------------------------- *
 *       Monitor the 12V power supply                           monitor12V()
 * ------------------------------------------------------------------------- */
void monitor12V() {

  debug(" --12V-- ");

  rawValue = analogRead(V12VoltagePin);        // pin for 12V voltage
  Voltage = rawValue * myVoltage_12V / 1023; // 427.3 found experimentally
  dtostrf( Voltage, 6, 2, strBuf1);           //  to obtain proper value

  #if DEBUG == 1
    show_V_Values();
  #endif

  sum = 0;                  // Calculate average over 'count' measurements
  for (int i=0; i<count; i++) {
    sum += analogRead(V12CurrentPin);       // pin for 5V current
  }
  rawValue = sum / count;                   // Calc average
  milliVolts = (rawValue / 1023.0) * myArduinoVoltage;   // yields milliVolts
  Current = ( ( milliVolts - ACSoffset )  / sensitivity );
  dtostrf( Current, 6, 2, strBuf2);

  #if DEBUG == 1
    show_A_Values();
  #endif

  LCD_display(display2, 0, 0, F("Voeding 12 Volt-"));
  LCD_display(display2, 1, 0,  strBuf1 );
  LCD_display(display2, 1, 6,  "V" );
  LCD_display(display2, 1, 9,  strBuf2 );
  LCD_display(display2, 1,15,  "A" );
}


/* ------------------------------------------------------------------------- *
 *       Monitor the DCC power supply                           monitorDCC()
 * ------------------------------------------------------------------------- */
void monitorDCC() {

  debug(" --DCC-- ");

  sum = 0;                  // Calculate average over 'count' measurements
  for (int i=0; i<count; i++) {
    sum += analogRead(DccCurrentPin);       // pin for 5V current
  }
  rawValue = sum / count;                   // Calc average
  milliVolts = (rawValue / 1023.0) * myArduinoVoltage;   // yields milliVolts
  Current = ( ( milliVolts - ACSoffset )  / sensitivity );
  dtostrf( Current, 6, 2, strBuf2);

  #if DEBUG == 1
    show_A_Values();
  #endif

  LCD_display(display3, 0, 0, F("DCC voeding-----"));
  LCD_display(display3, 1, 0,  "        " );
  LCD_display(display3, 1, 9,  strBuf2 );
  LCD_display(display3, 1,15,  "A" );
}



/* ------------------------------------------------------------------------- *
 *                  Show Voltage values after measurements - show_V_Values()
 * ------------------------------------------------------------------------- */
void show_V_Values() {
  debug("raw = ");
  debug( rawValue );
  debug(" ");
  debug("Volt = ");
  debug(Voltage);
  debug(" ");
  debug(" - ");
}



/* ------------------------------------------------------------------------- *
 *                  Show Current values after measurements - show_A_Values()
 * ------------------------------------------------------------------------- */
void show_A_Values() {
  debug("raw = ");
  debug( rawValue );
  debug(" ");
  debug("mVolt = ");
  debug(Voltage);
  debug(" ");
  debug("Current = ");
  debug(Current);
  debug(" - ");
}



/* ------------------------------------------------------------------------- *
 *       Routine to display stuff on the display of choice     LCD_display()
 * ------------------------------------------------------------------------- */
void LCD_display(LiquidCrystal_I2C screen, int row, int col, String text) {
  screen.setCursor(col, row);
  screen.print(text);
}
