int white = matrix.Color444(0xff, 0xff, 0xff);
int green = matrix.Color444(0x0, 0xff, 0x0);


RGB rainbow[] = {
  {255,0,0}, 
  {255,128,0},
  {255,255,0},
  {128,255,0},
  {0,255,0}, 
  {0,255,255},
  {0,128,255},
  {0,0,255},
  {0,0,255},
  {128,0,255},
  {255,0,128}
};

#define RAINBOW_SIZE (sizeof(rainbow)/sizeof(RGB))

// Lame pre-calculated sin values
const int8_t sinTable[] PROGMEM = {
  0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1
};

int whiteGenerator(uint8_t x, uint8_t y) {
  return white;
}

int greenGenerator(uint8_t x, uint8_t y) {
  return green;
}

int titleColorGenerator(uint8_t x, uint8_t y) {
  double gradientPercent = x/32.0;
  RGB color1= {0xff,0,0};
  RGB color2= {0xff,0xff,0};
  int r=fade(color1.r, color2.r, gradientPercent);  
  int g=fade(color1.g, color2.g, gradientPercent);  
  int b=fade(color1.b, color2.b, gradientPercent);  
 
  return matrix.Color444(r>>4, g>>4, b>>4);
  
  return green;
}

void fade(RGB *dest, RGB *c1, RGB *c2, double percent) {
  dest->r=fade(c1->r, c2->r, percent);  
  dest->g=fade(c1->g, c2->g, percent);  
  dest->b=fade(c1->b, c2->b, percent);  
}

// Index into the virtual rainbow
unsigned int rainbowIndex;

RGB topColor= {0x80,0x80,0x80};
RGB bottomColor={0xff,0xff,0xff};

RGB *topInitial=&rainbow[0];
RGB *bottomInitial=&rainbow[1];

RGB *topTarget=&rainbow[0];
RGB *bottomTarget=&rainbow[1];

int c1;
int c2;
int i = 0;

double pct =0;
void advanceAnimation() {
  rainbowIndex++;
  
  if (rainbowIndex%2 ==0) {
    pct+=.01;
    if (pct>1) {
      pct = 0;
    
//      topInitial = topTarget;
      topInitial = bottomInitial;
      bottomInitial = bottomTarget;
    
 //     topTarget=&rainbow[random(RAINBOW_SIZE)];
      bottomTarget=&rainbow[random(RAINBOW_SIZE)];
    }

//    fade(&topColor, topInitial, topTarget, pct);
    fade(&topColor, topInitial, bottomInitial, pct);
    fade(&bottomColor, bottomInitial, bottomTarget, pct);
  }
}

// Steps between physical rainbow colors
int fadeSteps = 3;

int8_t lameSin(int x) {
//  return pgm_read_byte(sinTable + x%sizeof(sinTable));
  return pgm_read_byte(sinTable + (x&15)); // Assumes 16 entries
}

int limit(int x, int max) {
  return x>=max ? 0:x;
}

int currentColorIndex;
int rainbowLines(uint8_t x, uint8_t y) {
  double gradientPercent = (double)y/16;

  int r=fade(topColor.r, bottomColor.r, gradientPercent);  
  int g=fade(topColor.g, bottomColor.g, gradientPercent);  
  int b=fade(topColor.b, bottomColor.b, gradientPercent);
 
  return matrix.Color444(r>>4, g>>4, b>>4);
}

/**
 * Generate a rainbow linear gradient
 */
int rainbowGenerator(uint8_t x, uint8_t y) {
  unsigned int colorIndex = rainbowIndex+x+lameSin(y+rainbowIndex);
  double gradientPercent;
  int r,g,b;
  
  int physicalColorIndex = colorIndex/fadeSteps % (sizeof(rainbow)/sizeof(RGB));
  int nextColorIndex = physicalColorIndex +1;
  if (nextColorIndex >= (sizeof(rainbow)/sizeof(RGB))) {
    nextColorIndex = 0;
  }
  
  gradientPercent =  (double)(colorIndex % fadeSteps) / (double)fadeSteps;
  
  RGB* color1=rainbow+physicalColorIndex;
  RGB* color2=rainbow+nextColorIndex;
  
  r=fade(color1->r, color2->r, gradientPercent);  
  g=fade(color1->g, color2->g, gradientPercent);  
  b=fade(color1->b, color2->b, gradientPercent);
 
  return matrix.Color444(r>>4, g>>4, b>>4);
}




int fullRainbow(uint8_t x, uint8_t y) {
  unsigned int colorIndex = rainbowIndex;
  double gradientPercent;
  int r,g,b;
  
  int physicalColorIndex = colorIndex/fadeSteps % (sizeof(rainbow)/sizeof(RGB));
  int nextColorIndex = physicalColorIndex +1;
  if (nextColorIndex >= (sizeof(rainbow)/sizeof(RGB))) {
    nextColorIndex = 0;
  }
  
  gradientPercent = (double)(colorIndex % fadeSteps) / (double)fadeSteps;
  
  RGB* color1=rainbow+physicalColorIndex;
  RGB* color2=rainbow+nextColorIndex;
  
  r=fade(color1->r, color2->r, gradientPercent);  
  g=fade(color1->g, color2->g, gradientPercent);  
  b=fade(color1->b, color2->b, gradientPercent);  
 
  return matrix.Color444(r>>4, g>>4, b>>4);
}

