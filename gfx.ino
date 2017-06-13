/**
 * Some drawing functions
 */

void drawPixel(uint8_t x, uint8_t y, ColorGenerator cg) {
  matrix.drawPixel(x, y, cg(x, y));
}

void drawDigitBitmap(int8_t x, int8_t y, const uint8_t *bitmap, ColorGenerator cg) {
  uint16_t i, j, color;

  unsigned int b;
  unsigned int mask;

  for(j=0; j<5; j++) {// height
    b = pgm_read_byte(bitmap + j);
    
    i=0;
    for(mask = 0x80; mask&0xf0; mask >>=1 ) {
      if (b & mask) {
        color = cg(x+i, y+j);
      } else {
        color = 0;
      }
      
      matrix.drawPixel(x+(i++), y+j, color);
    }
  }
}

void drawAlphaBitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, ColorGenerator cg) {
  uint16_t i, j, color;

  unsigned int b;
  unsigned int mask;

  for(j=0; j<5; j++) {// height
    b = pgm_read_byte(bitmap + j);
    
    i=0;
    for(mask = 0x10; mask&0x1f; mask >>=1 ) {
      if (b & mask) {
        color = cg(x+i, y+j);
      } else {
        color = 0;
      }

      matrix.drawPixel(x+(i++), y+j, color);
    }
  }
}

void drawDigit(uint8_t x, uint8_t y, int digit, ColorGenerator cg) {
  drawDigitBitmap(x, y, digits[digit], cg);
  
  matrix.drawLine(x+4, y, x+4, y+4, 0);
}

void drawText(uint8_t x, uint8_t y, char *message, ColorGenerator cg) {
  char c;
  
  while (c = *message++) {
    drawAlphaBitmap(x, y, letters[c-'A'], cg);
    matrix.drawLine(x+5, y, x+5, y+4, 0);
    x+=6;
  }
}

int drawNumber(uint8_t x, uint8_t y, int number, int minDigits, ColorGenerator cg) {
  int mult = 10000;
  
  int remaining = number;
  int significantZero = false;
  int digitsLeft = 5;
  int ndigits=0;
  int digit;
  
  do {
    digit = remaining / mult;
    remaining -= digit *mult;
    
    if (digitsLeft-- <= minDigits || digit != 0) {
      significantZero=true;
    }
    
    if (digit !=0 || significantZero) {
      drawDigit(x, y, digit, cg);
      ndigits++;
      x+=5;
    }
    
    mult = mult / 10;
  } while (mult);
  
  return ndigits;
}

void fillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, ColorGenerator cg) {
  int w, h;
  
  for (w = 0; w < width; w++) {
    for (h = 0; h < height; h++) {
      drawPixel(x+w, y+h, cg);
    }
  }
}

void centerText(uint8_t y, char *text, ColorGenerator cg) {
  int offset = (32-strlen(text)*6)/2;
  
  drawText(offset, y, text, cg);
}

// Fade between c1 and c2 by percent
int fade(int c1, int c2, double percent) {
  int d = c2 - c1;
  
  return c1 + d * percent;
}

