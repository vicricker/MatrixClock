/**
 * Simple clock for the Nootropic RGB Matrix Backpack
 * 
 * This isn't an example of my best code, by far.  I rushed to get it done as a Christmas present for my wife.
 *
 * Some things to note:
 * The Arduino doesn't have enough RAM for double buffering while using the Wire/Chronodot code because it needs some memory for buffer space, so
 * I had to do some unconventional things to keep the display from flickering.  It's likely possible to re-write the code that talks to the Chronodot
 * to reduce the amount of RAM required.
 *
 * The display calls a color generator function to generate the rainbow or other color patters.  Multiple pointers can be put into the array, and it
 * will cycle to a new color generator every minute.
 */
 
 
#include <Wire.h>

#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include "ColorGenerators.h"
#include "Chronodot.h"
#include "button.h"
#include "font.h"
#include "gfx.h"
#include "MatrixClock.h"
#include "editor.h"

char *months[]= {
  "JAN",
  "FEB",
  "MAR",
  "APR",
  "MAY",
  "JUN",
  "JUL",
  "AUG",
  "SEP",
  "OCT",
  "NOV",
  "DEC"
};

Chronodot RTC;

#define BUTTON_PIN (10)

#define CLK 8
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2

#define F_CHAR 10

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

ColorGenerator colorGenerator;

ColorGenerator colorGenerators[] = {rainbowLines};

/**
 * Example solid color generator
 */
int solidColor(uint8_t x, uint8_t y) {
  return matrix.Color888(0,0,255);
}

void setup() {
  colorGenerator=fullRainbow;

  // Set up the button
  pinMode(BUTTON_PIN,INPUT);
  digitalWrite(BUTTON_PIN, HIGH);

  // Begin communication with the Chronodot
  Wire.begin();
  RTC.begin();
  //    if (! RTC.isrunning()) {
  //  Serial.println("RTC is NOT running!");
  // following line sets the RTC to the date & time this sketch was compiled
   //RTC.adjust(DateTime(__DATE__, __TIME__));
  // }

  matrix.begin();
}

void drawColon(uint8_t x, uint8_t y, int state, ColorGenerator cg) {
  matrix.drawPixel(x, y, cg(x, y) * state);
  matrix.drawPixel(x, y+2, cg(x, y+2) * state);
}

void drawDegree(uint8_t x, uint8_t y, ColorGenerator cg) {
  drawPixel(x, y, cg);
  drawPixel(x+1, y, cg);
  drawPixel(x, y+1, cg);
  drawPixel(x+1, y+1, cg);
  
  matrix.drawPixel(x+2, y, 0);
  matrix.drawPixel(x+2, y+1, 0);
  
  matrix.fillRect(x, y+2, 3, 3, 0);
}

void drawSeconds(int seconds, ColorGenerator cg) {
  if (seconds == 0) {
    matrix.fillRect(1, 7, 30, 2, 0);
  } else {
    fillRect(1, 7, seconds/2, 2, cg);
    
    if (seconds & 1) {
      drawPixel(seconds/2+1, 7, colorGenerator);
    }
  }
}


boolean am=true;

void drawAMPM(int hour, ColorGenerator cg) {
  if (hour >= 12) {
    // Draw the P
    drawPixel(26, 3, cg);
    drawPixel(27, 3, cg);
    drawPixel(26, 4, cg);
    drawPixel(27, 4, cg);
    drawPixel(26, 5, cg);
    am = false;
  }
  
  if (hour < 12 && !am) {
    drawPixel(26, 3, 0);
    drawPixel(27, 3, 0);
    drawPixel(26, 4, 0);
    drawPixel(27, 4, 0);
    drawPixel(26, 5, 0);
    am = true;
  }
}

int prevMinute=-1;

void drawTime(DateTime time, boolean amPm, ColorGenerator cg) {
  int hour = time.hour();
  int minute = time.minute();
  int second = time.second();

  if (amPm) {
    drawAMPM(hour, cg);

    hour = hour > 12 ? hour -12 : hour;
    if (hour == 0) {
      hour = 12;
    }
  }

  drawNumber(4, 1, hour, 2, cg);
  
  drawColon(14, 2, second&1, cg);
  
  drawNumber(16, 1, minute, 2, cg);
  
  drawSeconds(second, colorGenerator);

  if (minute != prevMinute) {
    prevMinute = minute;
    colorGenerator= colorGenerators[minute % (sizeof(colorGenerators)/sizeof(colorGenerator))];
  }
}

void drawMonth(uint8_t x, uint8_t y, int month, ColorGenerator cg) {
  drawText(2, 10, months[month-1], cg);
}

void drawDate(DateTime time) {
  // Draw Month
  drawMonth(2, 10, time.month(), colorGenerator);

  // Draw day
  drawNumber(21, 10, time.day(), 2, colorGenerator);
}

void drawTemperature(DateTime time) {
  
  // temp seems to be 6 degrees off.  Subtract fudge factor :-)
  int temp = time.tempF()-6;
  
  int x = 8;
  if (temp >= 100) {
    x -= 2;
  }
  int digits = drawNumber(x, 10, temp, 1, colorGenerator);
  
  drawDegree(x+digits*5, 10, colorGenerator);
  
  drawText(x+digits*5+3, 10, "F", colorGenerator);
  
  matrix.fillRect(0, 10, x, 5, 0);

  int end = x+digits*5+8;
  matrix.fillRect(end, 10, 32-end, 5, 0);
}

void setTime(DateTime time) {
   RTC.adjust(time);
}

//DateTime now;

void editTime(DateTime now) {
  Time time;
  
  time.year = now.year();
  time.month = now.month();
  time.day = now.day();
  
  time.hour = now.hour();
  time.minute = now.minute();  

  timeEditor(&time);
  
  DateTime newTime = DateTime (time.year, time.month, time.day,time.hour, time.minute, 0, 0, 0.0);

  setTime(newTime);
  
  matrix.fillScreen(0);
}

void loop() {
  DateTime now = RTC.now();

  drawTime(now, true, colorGenerator);

  int second=now.second();
  
  // Every 5 seconds, change between date and teperature display.  (5 may be changed, but it should divide evenly into 60 or you'll have to use a counter instead.)
  if ((second/5) & 1) {
    drawDate(now);
  } else {
    drawTemperature(now);
  }

  advanceAnimation();

  if (getPress() == LONG_PRESS) {
    editTime(now);
  }
}

