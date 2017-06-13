#include "button.h"
/**
 * Recognize button presses and debounce
 * Returns 0 if nothing happend, SHORT_PRESS on a "short" press, and LONG_PRESS on a "long" press.
 * A long press is currently longer than 1000ms.
 *
 * When a long press is recognised, it will be returned immediately, but no more events will be generated until the button is released.
 *
 * This would probably be better done with an interrupt handler.
 */
 
enum PressState {WAITING, PRESSED, WAITING_FOR_RELEASE};
unsigned long pressStart = 0;
PressState state = WAITING;

int getPress() {
  int result = 0;

  unsigned long elapsed = millis() - pressStart;

  int button = !digitalRead(BUTTON_PIN);

  switch (state) {
    case WAITING:
      if (button) {
        state = PRESSED;
        pressStart = millis();
      }
      break;
    
    case PRESSED:
      if (elapsed > LONG_PRESS_TIME) {
        result = LONG_PRESS;
        state = WAITING_FOR_RELEASE;
      } else if (!button && elapsed > DEBOUNCE_TIME) {
        state = WAITING_FOR_RELEASE;
        result = elapsed > 1000 ? LONG_PRESS: SHORT_PRESS;
      }
      break;

    case WAITING_FOR_RELEASE:
      if (!button) {
        state = WAITING;
      }
      break;
  }

 return result;
}

