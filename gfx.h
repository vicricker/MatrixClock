typedef struct {
  int r, g, b;
} RGB;

typedef int (*ColorGenerator)(uint8_t x, uint8_t y);

void drawPixel(uint8_t x, uint8_t y, ColorGenerator cg);

