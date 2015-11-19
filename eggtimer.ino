/* eggtimer - A simple egg timer using a 4-digit LED display
 * (C)2015 Vik Olliver, vik@diamondage.co.nz
 * Licenced under the GPLv3.0 or greater.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include "TM1637Display.h"

// Digital Pins
#define FLASH_LED    12
#define CLK          2
#define DIO          3
#define SPEAKER_PIN  11
#define ALARM_PIN    10
#define SWITCH_A_PIN  4
#define SWITCH_B_PIN  5
#define RELAY_PIN     6
#define STALL_PIN     7

// Timer states
#define ST_INIT         0
#define ST_FLASH_WAIT   1
#define ST_TIMER_START  2
#define ST_TIMEOUT      3
#define ST_TIMER_SET    4
#define ST_TIMER_SET2   5
#define ST_TIMER_SET3   6
#define ST_TIMER_SET4   7

// Number of times we loop round flashing the initial LED
#define FLASH_LOOPS  10

const uint8_t SEG_BANG[] = {
	SEG_F | SEG_G | SEG_E | SEG_C | SEG_D,           // b
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,   // a
	SEG_A | SEG_B | SEG_C |SEG_E | SEG_F,            // n
	SEG_A | SEG_C | SEG_D | SEG_E | SEG_F            // G
	};

TM1637Display display(CLK, DIO);
unsigned long timerStart;
int timeTarget=300;  // Actual seconds
int state=ST_INIT;
boolean switchAdown;
boolean switchBdown;
  
void setup()
{
  pinMode(FLASH_LED,OUTPUT);
  pinMode(RELAY_PIN,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(SWITCH_A_PIN,INPUT_PULLUP);
  pinMode(SWITCH_B_PIN,INPUT_PULLUP);
  pinMode(STALL_PIN,INPUT_PULLUP);
  switchAdown=false;
  switchBdown=false;
  state=ST_INIT;
  // See if our signature is in EEPROM
  if ((EEPROM.read(0)==0x0b)&&(EEPROM.read(1)=='u')&&(EEPROM.read(2)=='g')&&(EEPROM.read(3)==0x0a)) {
    timeTarget=EEPROM.read(4)+EEPROM.read(5)*256;
  }
}

boolean switchApressed() {
  if (digitalRead(SWITCH_A_PIN)==LOW) {
    // If it was down last time, it is debounced.
    if (switchAdown) {
      // Wait for switch to go up, then return.
      while(digitalRead(SWITCH_A_PIN)==LOW);
      switchAdown=false;
      return(true);
    }
    switchAdown=true;
    delay(10);  // Debounce delay
  }
  return(false);
}  

boolean switchBpressed() {
  if (digitalRead(SWITCH_B_PIN)==LOW) {
    // If it was down last time, it is debounced.
    if (switchBdown) {
      // Wait for switch to go up, then return.
      while(digitalRead(SWITCH_B_PIN)==LOW);
      switchBdown=false;
      return(true);
    }
    switchBdown=true;
    delay(10);  // Debounce delay
  }
  return(false);
}  

void loop()
{
  uint8_t segto;
  int t=0;
  int count,oldcount;
  int digit;

  while(1) {
    switch(state) {
      case ST_INIT:
        digitalWrite(RELAY_PIN,LOW);
        display.setBrightness(0x0f);
        count=(100*(timeTarget/60))+(timeTarget%60);
        display.showNumberDec(count,true);
        segto = 0x80 | display.encodeDigit((count / 100)%10);  // Set colon
        display.setSegments(&segto, 1, 1);
        t=0;
        state=ST_FLASH_WAIT;
        break;
        
      // Make beeps, flash lights etc. until a switch is pressed.
      case ST_FLASH_WAIT:
        if (t>FLASH_LOOPS/2) {
          digitalWrite(FLASH_LED,HIGH);
          digitalWrite(13,LOW);
          tone(SPEAKER_PIN,2500,100);
          delay(50);
        } else {
          digitalWrite(FLASH_LED,LOW);
          digitalWrite(13,HIGH);
          segto = 0x80|display.encodeDigit((count / 100)%10);  // Flash colon
          display.setSegments(&segto, 1, 1);
          delay(50);
        }
        if (switchApressed())
          state=ST_TIMER_START;
        if (switchBpressed())
          state=ST_TIMER_SET;

        if (t++>FLASH_LOOPS) t=0;
        break;
  
      // Set the first digit.
      case ST_TIMER_SET:
        if (switchBpressed()) {
          display.showNumberDec(count, true);
          state=ST_TIMER_SET2;
        }
        else {
          digit=(count/1000);
          if (millis()%500UL>250) {
            segto = 8;
          } else {
            segto = display.encodeDigit(digit);
          }
          display.setSegments(&segto, 1, 0);
          if (switchApressed()) {
            if (digit==9) count-=9000;
            else count+=1000;
            display.showNumberDec(count, true);
          }
        }
        break;
        
      case ST_TIMER_SET2:
        if (switchBpressed()) {
          display.showNumberDec(count, true);
          state=ST_TIMER_SET3;
        }
        else {
          digit=(count/100)%10;
          if (millis()%500UL>250) {
            segto = 8;
          } else {
            segto = display.encodeDigit(digit);
          }
          display.setSegments(&segto, 1, 1);
          if (switchApressed()) {
            if (digit==9) count-=900;
            else count+=100;
            display.showNumberDec(count, true);
          }
        }
        break;
  
      case ST_TIMER_SET3:
        if (switchBpressed()) {
          display.showNumberDec(count, true);
          state=ST_TIMER_SET4;
        }
        else {
          digit=(count/10)%10;
          if (millis()%500UL>250) {
            segto = 8;
          } else {
            segto = display.encodeDigit(digit);
          }
          display.setSegments(&segto, 1, 2);
          if (switchApressed()) {
            if (digit==5) count-=50;
            else count+=10;
            display.showNumberDec(count, true);
          }
        }
        break;

      case ST_TIMER_SET4:
         if (switchBpressed()) {
          display.showNumberDec(count, true);
          // Convert displayed MMSS into seconds
          timeTarget=60*(count/100)+count%100;
          // Write time and signature to EEPROM
          EEPROM.write(0,0x0b);
          EEPROM.write(1,'u');
          EEPROM.write(2,'g');
          EEPROM.write(3,0x0a);
          EEPROM.write(4,timeTarget%256);
          EEPROM.write(5,timeTarget/256);
          state=ST_TIMER_START;
        }
        else {
          digit=count%10;
          if (millis()%500UL>250) {
            segto = 8;
          } else {
            segto = display.encodeDigit(digit);
          }
          display.setSegments(&segto, 1, 3);
          if (switchApressed()) {
            if (digit==9) count-=9;
            else count++;
            display.showNumberDec(count, true);
          }
        }
        break;

     case ST_TIMER_START:
        timerStart=millis();
        count=0,oldcount=-1;
        while(timeTarget-t>=0) {
          t=int((millis()-timerStart)/1000UL);
          // Only update display if things change to stop flickering.
          count=timeTarget-t;
          // Our count is in mins & sec, so munge it as appropriate
          count=(100*(count/60))+(count%60);
          if (oldcount!=count) {
            oldcount=count;
            display.showNumberDec(count, true);
            segto = (((count%2==0))?0:0x80) | display.encodeDigit((count / 100)%10);  // Flash colon
            display.setSegments(&segto, 1, 1);
          }
          delay(100);
          if (switchApressed() || switchBpressed()) {
            state=ST_INIT;
            break;
          }
        }
        if (state==ST_TIMER_START) state=ST_TIMEOUT;
        break;
        
      case ST_TIMEOUT:
        // Done!
        display.setSegments(SEG_BANG);
        digitalWrite(RELAY_PIN,HIGH);
        for (int k=0; k<20; k++) {
          tone(ALARM_PIN,2500,250);
          delay(500);
        }
        if (digitalRead(STALL_PIN)==HIGH) state=ST_INIT;
        break;
      } // End of state switch
  } // End of infinite while

}
