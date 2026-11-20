# Ms. Pac-Man 64x32 HUB75 Cutscene & Clock Demo (ESP32-S3)

A custom 64x32 RGB LED Matrix proof-of-concept and cutscene showcase inspired by Ms. Pac-Man, arcade kill screens, and shop/family humor. Built for **ESP32-S3 N16R8** using **PlatformIO** and **VS Code**.

## Features
- **Access Point Web Controller:** Boots directly into Wi-Fi AP mode (`MsPacMan-Demo`, password `mspacman`) hosting a mobile-friendly web UI at `http://192.168.4.1` to test all 24 cutscenes, adjust playback speed (`0.25x` to `3.0x`), toggle auto-cycle mode, and adjust panel brightness.
- **Native 64x32 Pixel Art Engine:** Includes custom 5x8 (10x16 rendered) arcade digits, 3x5 compact alphanumeric font, 7x7 character sprites, vehicles, and props.
- **All 24 Cutscenes + Boot & Kill Screen Sequences:** Includes the 7:45 AM Arcade Power-On Boot Sequence, 4:00 PM Level 256 Kill Screen Finale, 14 original gag cutscenes, and 10 expanded shop/family scenes.

## Hardware & GPIO Wiring (ESP32-S3 N16R8 -> 64x32 HUB75)
> Note: A 64x32 panel is **1/16 scan**, so address pin **E** is not required (`-1`).

| HUB75 Pin | ESP32-S3 GPIO | Notes |
| :--- | :--- | :--- |
| **R1** | GPIO 4 | Upper half Red |
| **G1** | GPIO 5 | Upper half Green |
| **B1** | GPIO 6 | Upper half Blue |
| **R2** | GPIO 7 | Lower half Red |
| **G2** | GPIO 15 | Lower half Green |
| **B2** | GPIO 16 | Lower half Blue |
| **A** | GPIO 17 | Row select A |
| **B** | GPIO 18 | Row select B |
| **C** | GPIO 8 | Row select C |
| **D** | GPIO 9 | Row select D |
| **E** | `-1` (NC) | Unused on 64x32 1/16 scan |
| **LAT** | GPIO 11 | Data latch |
| **OE** | GPIO 12 | Output enable |
| **CLK** | GPIO 13 | Shift clock |
| **GND** | GND | Common ground with 5V PSU |
| **LDR** | GPIO 1 | Optional photoresistor voltage divider |

## Quick Start (VS Code + PlatformIO)
1. Create a new PlatformIO project for `esp32-s3-devkitc-1` (or place these files into your project root).
2. Place `platformio.ini`, `README.md`, `LICENSE`, and `.gitignore` in the root folder, and place `main.cpp` and `cutscenes.h` inside `src/`.
3. Click **PlatformIO: Build** and **PlatformIO: Upload**.
4. Connect your phone or PC to Wi-Fi network **`MsPacMan-Demo`** (password: **`mspacman`**) and open **`http://192.168.4.1`**.
