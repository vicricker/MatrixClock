/**
 * Editor to set the time and date
 *
 * Use a short press to change the selected value, and a long press (See button.h) to commit that value and advance to the next
 */

#include "editor.h"

static const uint8_t daysInMonth[] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

/**
 * Factory to get the ColorGenerator for the item being edited.
 * This implementation just alternates between white and green every ~1024ms
 */
ColorGenerator getEditColorGenerator() {
  return millis() & 0x100 ? whiteGenerator : greenGenerator; // Swap every ~256ms
}

/**
 * Is it leap year?
 * This is used to allow the 29th of February during leap year
 */
bool isLeapYear(uint16_t year) {
  return ((!year%4) && (year % 100)) || (!year % 400);
}

/**
 * Draw the editor page title (year, date, time)
 */
void drawEditorTitle(char *title) {
  // Clear screen
  matrix.fillScreen(0);

  // Draw title
  centerText(1, title, titleColorGenerator);
}

/**
 * Edit the given digit at the x,y coordinates, and wrap to 0 if the maxValue is exceeded
 */
int digitEditor(uint8_t x, uint8_t y, int digit, int maxValue) {
  return digitEditor(x, y, digit, 0, maxValue);
}

/**
 * Edit the given digit at the x,y coordinates, and wrap to minValue if the maxValue is exceeded
 */
int digitEditor(uint8_t x, uint8_t y, uint8_t digit, uint8_t minValue, uint8_t maxValue) {
  ColorGenerator cg;

  // It's possible for the incoming digit to be invalid if the previous digit was changed
  // Fix it
  if (digit > maxValue) {
    digit = maxValue;
  }
  
  if (digit < minValue) {
    digit = minValue;
  }
  
  int press;
  do {
    cg = getEditColorGenerator();
  
    drawDigit(x, y, digit, cg);

    press = getPress();

    if (press == SHORT_PRESS) {
      digit++;
    
      if (digit > maxValue) {
        digit = minValue;
      }
    }
  } while(press != LONG_PRESS);

  drawDigit(x, y, digit, whiteGenerator);
  
  return digit;
}

/**
 * Editor entry point
 */
void timeEditor(struct Time *time) {
  editYear(time);

  editDate(time);

  editTime(time);
}

/**
 * Year editor
 * For convenience sake, it only allows the last two digits to be modified.
 */
void editYear(struct Time *time) {
  drawEditorTitle("YEAR");

  drawNumber(6, 10, time->year, 4, whiteGenerator);  
  
  uint16_t year2 = time->year - 2000; // Kind of hacky, but I'll be long gone before this is a problem :-)
  uint8_t decade = year2/10;
  uint8_t rest = year2%10;
  
  decade = digitEditor(16, 10, decade, 9);
  rest = digitEditor(21, 10, rest, 9);
  
  time->year = 2000 + decade * 10 + rest;
}

/**
 * Allows the user to select the month.
 * returns its ordinal value (1-12)
 */
int editMonth(struct Time *time) {
  ColorGenerator cg;
  uint8_t month = time->month;
  
  int press;
  do {
    cg = getEditColorGenerator();
  
    drawMonth(2, 10, month, cg);

    press = getPress();

    if (press == SHORT_PRESS) {
      month++;
    
      if (month > 12) {
        month = 1;
      }
    }
  } while(press != LONG_PRESS);

  drawMonth(2, 10, month, whiteGenerator);
  
  return month;
}

/**
 * Edit the date (month and day)
 */
void editDate(struct Time *time) {
  drawEditorTitle("DATE");

  uint8_t day = time->day;
  uint8_t tens = day/10;
  uint8_t ones = day%10;
  
  drawNumber(21, 10, day, 2, whiteGenerator);  

  time->month = editMonth(time);
  
  
  uint8_t maxDays = pgm_read_byte(daysInMonth+time->month-1);
  
  // Handle 29 days in February during leap year
  if (time->month == 2 && isLeapYear(time->year)) {
    maxDays++;
  }
  
  uint8_t maxTens = maxDays/10;
 
  tens = digitEditor(21, 10, tens, maxTens);
 
  // If at max of tens, limit last digit
  if (tens == maxTens) {
    maxDays = maxDays%10;
  } else {
    maxDays = 9;
  }
  
  ones = digitEditor(26, 10, ones, !tens ? 1:0, maxDays);
 
  time->day = tens*10+ones;
}

/**
 * Edit the time in 24 hour format to avoid having to select AM or PM
 */
void editTime(struct Time *time) {
  drawEditorTitle("TIME");

  uint8_t hour = time->hour;
  uint8_t minute = time->minute;

  drawNumber(4, 10, hour, 2, whiteGenerator);

  drawColon(14, 11, 1, whiteGenerator);

  drawNumber(16, 10, minute, 2, whiteGenerator);


  uint8_t hourTens = hour/10;
  hourTens = digitEditor(4, 10, hourTens, 2);
  
  uint8_t hourOnes = hour%10;
  hourOnes = digitEditor(9, 10, hourOnes, hourTens == 2 ? 3: 9);
  
  time->hour = hourTens * 10 + hourOnes;
  
  uint8_t minuteTens = minute/10;
  minuteTens = digitEditor(16, 10, minuteTens, 5);
  
  uint8_t minuteOnes = minute%10;
  minuteOnes = digitEditor(21, 10, minuteOnes, 9);
  
  time->minute = minuteTens * 10 + minuteOnes;
}

