#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "hsv.h"

#define NUM_PIXEL       60
#define PIXEL_PIN       D4
#define BUTTON_PIN_1    D6
#define BUTTON_PIN_2    D8

// Set to false if you don't want to use the button for mode switching
static const bool useButton = true;

// Define a timeout for auto-switching modes if no button press is detected
// Set to 0 to disable auto-switching
static const int autoSwitchTimeoutMillis = 5000; // 5 seconds

Adafruit_NeoPixel *strip;

static bool buttonPressed = false;

void IRAM_ATTR button_isr() {
  buttonPressed = true;
}

void setup() {
  Serial.begin(115200);

  strip = new Adafruit_NeoPixel(NUM_PIXEL, PIXEL_PIN, NEO_GRB | NEO_KHZ800);
  strip->begin();

  for (unsigned int i = 0; i < NUM_PIXEL; i++) {
    strip->setPixelColor(i, 255, 0, 0);
  }

  strip->show();

  if (useButton) {
    pinMode(BUTTON_PIN_1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN_1), button_isr, FALLING);

    pinMode(BUTTON_PIN_2, OUTPUT);
    digitalWrite(BUTTON_PIN_2, LOW);
  }
}

enum {
    ModeRainbow = 0,
    ModeChasing,
    ModeMultiChading,
    ModeSparkling,
    ModeMax
};

static int currentMode = ModeRainbow;

void loop() {
  if (buttonPressed) {
    buttonPressed = false;

    currentMode++;

    if (currentMode == ModeMax) {
      currentMode = ModeRainbow;
    }
  }

  switch (currentMode) {
    case ModeRainbow:
      static int counter = 0;

      counter++;

      for (int i = 0; i < NUM_PIXEL; i++) {
        int hue = (counter+i) % 360;
        int r, g, b;

        HSVtoRGB(hue, 100, 100, &r, &g, &b);
        strip->setPixelColor(i, strip->Color(r, g, b));
      }

      strip->show();
      delay(10);

      break;

    case ModeChasing:
      static int chaseIndex = 0;
      static int chaseDirection = 1;

      for (int i = 0; i < NUM_PIXEL; i++) {
        if (i == chaseIndex) {
          strip->setPixelColor(i, strip->Color(255, 255, 255)); // white
        } else {
          strip->setPixelColor(i, strip->Color(0, 0, 0)); // off
        }
      }

      strip->show();
      chaseIndex += chaseDirection;

      if (chaseIndex >= NUM_PIXEL || chaseIndex < 0) {
        chaseDirection *= -1; // reverse direction
      }

      delay(20);
      break;

    case ModeMultiChading:
      static int multiChaseIndex = 0;

      for (int i = 0; i < NUM_PIXEL; i++) {
        int hue = (multiChaseIndex+i) % 360;
        int r, g, b;

        HSVtoRGB(hue, 100, 100, &r, &g, &b);

        if ((i+multiChaseIndex) % 4 == 0) {
          strip->setPixelColor(i, strip->Color(r, g, b));
        } else {
          strip->setPixelColor(i, strip->Color(0, 0, 0)); // off
        }
      }

      strip->show();
      multiChaseIndex++;

      delay(100);
      break;

    case ModeSparkling:
      for (int i = 0; i < NUM_PIXEL; i++) {
        if (random(0, 10) < 2) { // 20% chance to light up
          int hue = random(0, 360);
          int r, g, b;

          HSVtoRGB(hue, 100, 100, &r, &g, &b);
          strip->setPixelColor(i, strip->Color(r, g, b));
        } else {
          strip->setPixelColor(i, strip->Color(0, 0, 0)); // off
        }
      }

      strip->show();
      delay(50);
      break;

    default:
      for (int i = 0; i < NUM_PIXEL; i++) {
        strip->setPixelColor(i, strip->Color(0, 0, 0));
      }
      break;
  }

  if (autoSwitchTimeoutMillis > 0) {
    static unsigned long lastMillis = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastMillis >= autoSwitchTimeoutMillis) {
      lastMillis = currentMillis;
      buttonPressed = true; // Trigger mode switch automatically
    }
  }
}
