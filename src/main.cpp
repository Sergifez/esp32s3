#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "esp_camera.h"
#include <WiFi.h>

// ---------------- LED ----------------
#define LED_PIN    48
#define LED_COUNT  1
#define BRIGHTNESS 50

// --------------------------- CAMARA  (CAMERA_MODEL_ESP32S3_EYE) --------------------------
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  14 // 14
#define SIOD_GPIO_NUM  4  // 1
#define SIOC_GPIO_NUM  5  // 2

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM  7
#define PCLK_GPIO_NUM  13


Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

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

// ---------------- WIFI ----------------
const char *ssid = "sagemcomFAB0";
const char *password = "GTM5CDM4ADNMWZ";

// ---------------- CAMARA (SIN PSRAM) ----------------
camera_config_t config = {
    .pin_pwdn       = PWDN_GPIO_NUM,
    .pin_reset      = RESET_GPIO_NUM,
    .pin_xclk       = XCLK_GPIO_NUM,
    .pin_sscb_sda   = SIOD_GPIO_NUM,
    .pin_sscb_scl   = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,

    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href  = HREF_GPIO_NUM,
    .pin_pclk  = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,
    .ledc_timer   = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565,
    .frame_size   = FRAMESIZE_QQVGA,
    .jpeg_quality = 30,
    .fb_count     = 1
};

// ---------------- SERVIDOR ----------------
WiFiServer server(80);

void sendCapture(WiFiClient &client) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return;

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/bmp");
  client.println("Connection: close");
  client.println();

  // Convertir RGB565 → BMP simple
  uint16_t w = fb->width;
  uint16_t h = fb->height;

  uint32_t fileSize = 54 + w * h * 2;

  uint8_t bmpHeader[54] = {
    'B','M',
    (uint8_t)(fileSize), (uint8_t)(fileSize>>8), (uint8_t)(fileSize>>16), (uint8_t)(fileSize>>24),
    0,0, 0,0, 54,0,0,0, 40,0,0,0,
    (uint8_t)(w), (uint8_t)(w>>8), 0,0,
    (uint8_t)(h), (uint8_t)(h>>8), 0,0,
    1,0, 16,0, 0,0,0,0,
    0,0,0,0, 0x13,0x0B,0,0, 0x13,0x0B,0,0,
    0,0,0,0, 0,0,0,0
  };

  client.write(bmpHeader, 54);
  client.write(fb->buf, fb->len);

  esp_camera_fb_return(fb);
}

void sendStream(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println();

  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) continue;

    client.printf("--frame\r\nContent-Type: image/bmp\r\nContent-Length: %u\r\n\r\n", fb->len + 54);

    // BMP header
    uint16_t w = fb->width;
    uint16_t h = fb->height;
    uint32_t fileSize = 54 + w * h * 2;

    uint8_t bmpHeader[54] = {
      'B','M',
      (uint8_t)(fileSize), (uint8_t)(fileSize>>8), (uint8_t)(fileSize>>16), (uint8_t)(fileSize>>24),
      0,0, 0,0, 54,0,0,0, 40,0,0,0,
      (uint8_t)(w), (uint8_t)(w>>8), 0,0,
      (uint8_t)(h), (uint8_t)(h>>8), 0,0,
      1,0, 16,0, 0,0,0,0,
      0,0,0,0, 0x13,0x0B,0,0, 0x13,0x0B,0,0,
      0,0,0,0, 0,0,0,0
    };

    client.write(bmpHeader, 54);
    client.write(fb->buf, fb->len);
    client.println();

    esp_camera_fb_return(fb);
    delay(80); // SIN PSRAM → refresco lento
  }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("SERIAL OK");
  
  if (psramFound()) {
    Serial.println("PSRAM detectada");
    Serial.printf("Tamaño PSRAM: %d bytes\n", ESP.getPsramSize());
  } else {
    Serial.println("No hay PSRAM");
  }

  led.begin();
  led.setBrightness(BRIGHTNESS);
  led.show();

  WiFi.begin(ssid, password);
  Serial.println("WIFI CONNECTING");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  pinMode(PWDN_GPIO_NUM, OUTPUT);
  digitalWrite(PWDN_GPIO_NUM, LOW);
  delay(10);

  esp_err_t err = esp_camera_init(&config);
  Serial.printf("CAM INIT: 0x%x\n", err);

  server.begin();
}



// ---------------- LOOP ----------------
void loop() {

  // LED animación
  fadeColor(255, 0, 0,   0, 255, 0,   100, 20);
  fadeColor(0, 255, 0,   0, 0, 255,   100, 20);
  fadeColor(0, 0, 255,   255, 0, 0,   100, 20);

  WiFiClient client = server.available();
  if (!client) return;

  String req = client.readStringUntil('\r');

  if (req.indexOf("/capture") != -1) {
    sendCapture(client);
  } else if (req.indexOf("/stream") != -1) {
    sendStream(client);
  } else {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<h1>ESP32-S3 Camera (NO PSRAM)</h1>");
    client.println("<a href='/capture'>Capture</a><br>");
    client.println("<a href='/stream'>Stream</a><br>");
  }

  client.stop();
}
