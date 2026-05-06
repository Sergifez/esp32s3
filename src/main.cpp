#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN    48
#define LED_COUNT  1
#define BRIGHTNESS 50   // 0–255 (no pongas 255, consume más)

Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Función para hacer un fundido suave entre dos colores
void fadeColor(uint8_t r1, uint8_t g1, uint8_t b1,
               uint8_t r2, uint8_t g2, uint8_t b2,
               uint16_t steps, uint16_t delayMs) {

  for (uint16_t i = 0; i <= steps; i++) {
    float t = (float)i / steps;

    uint8_t r = r1 + (r2 - r1) * t;
    uint8_t g = g1 + (g2 - g1) * t;
    uint8_t b = b1 + (b2 - b1) * t;

    led.setPixelColor(0, led.Color(r, g, b));
    led.show();
    delay(delayMs);
  }
}

void setup() {
  led.begin();
  led.setBrightness(BRIGHTNESS);
  led.show(); // LED apagado al inicio
}

void loop() {
  fadeColor(255, 0, 0,   0, 255, 0,   100, 20); // Rojo → Verde
  fadeColor(0, 255, 0,   0, 0, 255,   100, 20); // Verde → Azul
  fadeColor(0, 0, 255,   255, 0, 0,   100, 20); // Azul → Rojo
}