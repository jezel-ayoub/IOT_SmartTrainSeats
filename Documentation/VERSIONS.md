# Toolchain and library versions

Recorded so the build can be reproduced exactly.

| Component | Version |
|---|---|
| Arduino IDE | 2.3.10 |
| esp32 board package (Espressif Systems) | 3.3.12 |
| Adafruit NeoPixel | 1.15.5 |

## Board settings (Arduino IDE -> Tools)

| Setting | Value |
|---|---|
| Board | ESP32 Dev Module / DOIT ESP32 DEVKIT V1 |
| Upload Speed | 115200 |
| Port | COM3 (Windows) |
| Serial Monitor baud | 115200 |

## Notes

Upload speed is deliberately set to 115200 rather than the 921600 default.
The faster rate produced intermittent "Serial data stream stopped" and flash
communication failures on this board and cable.
