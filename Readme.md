# ESP32-S3 RGB LED (WS2812)

Proyecto para ESP32-S3 N16R8 usando PlatformIO.
Control de un LED RGB direccionable (WS2812) con transiciones suaves de color.

## Hardware
- ESP32-S3 WROOM-1 N16R8
- LED RGB WS2812 integrado
- GPIO usado: **48**

## Software
- PlatformIO
- Framework Arduino
- Librería Adafruit NeoPixel

## Funcionalidad
- Transiciones suaves entre:
  - Rojo
  - Verde
  - Azul
- Brillo configurable

## Uso
1. Abrir el proyecto en VS Code + PlatformIO
2. Conectar la placa por USB
3. Compilar y subir

## Notas
- Confirmado funcionamiento del LED en GPIO 48