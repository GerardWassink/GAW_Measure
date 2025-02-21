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
 *   1.4  : Switched to one display 20x4 instead of 3 displays 16x2
 *          Built in over-current detection, signal thru LEDs, reset button
 *   1.5  : Added DCC voltage
 *
 *------------------------------------------------------------------------- */
#define progVersion "1.5"              // Program version definition
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
 *                                             Include headers for libraries
 * ------------------------------------------------------------------------- */
#include <Wire.h>                           // I2C comms library
#include <LiquidCrystal_I2C.h>              // LCD library


/* ------------------------------------------------------------------------- *
 *                                             Pin definitions for measuring
 * ------------------------------------------------------------------------- */
#define V05VoltagePin A0                    // for 5 Volt power supply
#define V05CurrentPin A1

#define V12VoltagePin A2                    // for 12 Volt power supply
#define V12CurrentPin A3

#define DccVoltagePin A6                    // for DCC Volt power supply
#define DccCurrentPin A7


/* ------------------------------------------------------------------------- *
 *                                          Create object for the LCD screen
 * ------------------------------------------------------------------------- */
LiquidCrystal_I2C display1(0x24, 20, 4);    // Initialize display1 object


/* ------------------------------------------------------------------------- *
 *                                  Constants and variables for measurements
 * ------------------------------------------------------------------------- */
const int count = 5;                        // how many samples to average
int       sum = 0;                          // variable holds sum
const int sensitivity = 185;                // for 5A current sensors
int       rawValue = 0;                     // Value read from Analog pins
float     Voltage  = 0;                     // used to calculate
float     milliVolts  = 0;                  // used to calculate
float     Current  = 0;                     // used to calculate


/* ------------------------------------------------------------------------- *
 *                                          Variables for over-current alarm
 * ------------------------------------------------------------------------- */
bool alarm = false;


/* ------------------------------------------------------------------------- *
 *           Measured voltages for Arduino supply, 5V and 12V power supplies
 * ------------------------------------------------------------------------- */
float myArduinoVoltage = 4910.0;            // Voltage from Arduino pwr supply
                                            //   in milliVolts
float myVoltage_05V = 5.25;                 // measured voltage from 5V supply
float myVoltage_12V = 12.33;                // measured voltage from 12V supply
float myVoltage_DCC = 17.60;                // measured voltage from DCC supply
                                            //   after diode and capacitor


/* ------------------------------------------------------------------------- *
 *                                      Experimentally established ACSoffset
 * ------------------------------------------------------------------------- */
float ACSoffset = 2460;                     // Expermimentally established
                                            //   offset for current sensors


/* ------------------------------------------------------------------------- *
 *                                       Buffers to build values for display
 * ------------------------------------------------------------------------- */
char strBuf1[20];                           // stringbuffers to build up
char strBuf2[20];                           // numbers for display



/* ------------------------------------------------------------------------- *
 *                                          Initialization routine - setup()
 * ------------------------------------------------------------------------- */
void setup() {
  debugstart(115000);   // make debug output fast

  Wire.begin();         // Start I2C

                        // Initialize display backlight on by default
  display1.init(); display1.backlight();

                        // Initial text on display
  LCD_display(display1, 0, 0, F("==GAW Monitor v   =="));
  LCD_display(display1, 0,15, String(progVersion));

  pinMode(PD4, OUTPUT);
  pinMode(PD5, OUTPUT);
  pinMode(PD6, OUTPUT);

  pinMode(PD7, INPUT_PULLUP);

}



/* ------------------------------------------------------------------------- *
 *                                                   Repeating loop - loop()
 * ------------------------------------------------------------------------- */
void loop() {

  monitor5V();

  monitor12V();

  monitorDCC();

  debugln(" ");                             // When debugging, 'print' newline

  checkReset();                             // reset button pushed?

  delay(333);                               // Monitor every 333 milliseconds
}


/* ------------------------------------------------------------------------- *
 *       Monitor the 5V power supply                             monitor5V()
 * ------------------------------------------------------------------------- */
void monitor5V() {

  debug(" --5V-- ");

  getRawValue(V05VoltagePin);

  Voltage = rawValue * myVoltage_05V / 1023;
  dtostrf( Voltage, 6, 2, strBuf1);

  #if DEBUG == 1
    show_V_Values();
  #endif

  getRawValue(V05CurrentPin);

  milliVolts = (rawValue / 1023.0) * myArduinoVoltage;
  Current = ( ( milliVolts - ACSoffset )  / (sensitivity) );
  dtostrf( Current, 6, 2, strBuf2);

  watchDog(Current, 5);

  #if DEBUG == 1
    show_A_Values();
  #endif

  LCD_display(display1, 3, 0,  "05V" );
  LCD_display(display1, 3, 3,  strBuf1 );
  LCD_display(display1, 3, 9,  "V" );
  LCD_display(display1, 3,13,  strBuf2 );
  LCD_display(display1, 3,19,  "A" );
}


/* ------------------------------------------------------------------------- *
 *       Monitor the 12V power supply                           monitor12V()
 * ------------------------------------------------------------------------- */
void monitor12V() {

  debug(" --12V-- ");

  getRawValue(V12VoltagePin);

  Voltage = rawValue * myVoltage_12V / 1023; // 427.3 found experimentally
  dtostrf( Voltage, 6, 2, strBuf1);           //  to obtain proper value

  #if DEBUG == 1
    show_V_Values();
  #endif

  getRawValue(V12CurrentPin);

  milliVolts = (rawValue / 1023.0) * myArduinoVoltage;   // yields milliVolts
  Current = ( ( milliVolts - ACSoffset )  / sensitivity );
  dtostrf( Current, 6, 2, strBuf2);

  watchDog(Current, 12);

  #if DEBUG == 1
    show_A_Values();
  #endif

  LCD_display(display1, 2, 0,  "12V" );
  LCD_display(display1, 2, 3,  strBuf1 );
  LCD_display(display1, 2, 9,  "V" );
  LCD_display(display1, 2,13,  strBuf2 );
  LCD_display(display1, 2,19,  "A" );
}


/* ------------------------------------------------------------------------- *
 *       Monitor the DCC power supply                           monitorDCC()
 * ------------------------------------------------------------------------- */
void monitorDCC() {

  debug(" --DCC-- ");

  getRawValue(DccVoltagePin);

  Voltage = 0.4 + myVoltage_DCC * rawValue / 1023;  // 17.60 found experimentally
                                                    //   +0.4 for diode drop
  dtostrf( Voltage, 6, 2, strBuf1);           //  to obtain proper value

  #if DEBUG == 1
    show_V_Values();
  #endif

  getRawValue(DccCurrentPin);

  milliVolts = (rawValue / 1023.0) * myArduinoVoltage;   // yields milliVolts
  Current = ( ( milliVolts - ACSoffset )  / sensitivity );
  dtostrf( Current, 6, 2, strBuf2);

  watchDog(Current, 18);

  #if DEBUG == 1
    show_A_Values();
  #endif

  LCD_display(display1, 1, 0,  "DCC         " );
  LCD_display(display1, 1, 3,  strBuf1 );
  LCD_display(display1, 1, 9,  "V" );
  LCD_display(display1, 1,13,  strBuf2 );
  LCD_display(display1, 1,19,  "A" );
}



/* ------------------------------------------------------------------------- *
 *            repeating code te get rawValue from analog pin - getRawValue()
 * ------------------------------------------------------------------------- */
void getRawValue(int pin) {
  sum = 0;                  // Calculate average over 'count' measurements
  for (int i=0; i<count; i++) {
    sum += analogRead(pin);       // pin for DCC current
  }
  rawValue = sum / count;                   // Calc average
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
 *       LED on when over-current                                 watchDog()
 * ------------------------------------------------------------------------- */
void watchDog(float Cur, int Volt) {
                        // Current highr than treshold, switch LED on
  if (Cur >= 0.5) {
    alarm = true;
    switch (Volt) {
      case 5:
        digitalWrite(PD4, HIGH);
        break;
      case 12:
        digitalWrite(PD5, HIGH);
        break;
      case 18:
        digitalWrite(PD6, HIGH);
        break;
      default:
        break;
    }
                        // Indicated over-current
    LCD_display(display1, 0, 0, F("== OVER-CURRENT!! =="));
  }
}



/* ------------------------------------------------------------------------- *
 *       Reset button pressed? Alarm LEDs off                     watchDog()
 * ------------------------------------------------------------------------- */
void checkReset() {
  int res = digitalRead(PD7);
  if (res == 0) {
      digitalWrite(PD4, LOW);
      digitalWrite(PD5, LOW);
      digitalWrite(PD6, LOW);

      alarm = false;
                         // Restore Initial text on display
      LCD_display(display1, 0, 0, F("==GAW Monitor v   =="));
      LCD_display(display1, 0,15, String(progVersion));
     
  }
}



/* ------------------------------------------------------------------------- *
 *       Routine to display stuff on the display of choice     LCD_display()
 * ------------------------------------------------------------------------- */
void LCD_display(LiquidCrystal_I2C screen, int row, int col, String text) {
  screen.setCursor(col, row);
  screen.print(text);
}
