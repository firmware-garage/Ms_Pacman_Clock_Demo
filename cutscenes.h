#pragma once
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

extern MatrixPanel_I2S_DMA *dma_display;

// Palette
extern uint16_t C_BLACK, C_YELLOW, C_RED, C_PINK, C_CYAN, C_ORANGE, C_BLUE, C_WHITE, C_GREEN, C_PELLET, C_GRAY, C_BROWN;

// Drawing primitives (implemented in main.cpp)
void drawBigChar(int x, int y, char c, uint16_t color);
void drawSmallText(int x, int y, const char* str, uint16_t color);
void drawGhost(int x, int y, uint16_t bodyColor, bool shades = false);
void drawMsPacMan(int x, int y, bool mouthOpen, bool flipX = false, bool hasBow = true);
void drawJunior(int x, int y, bool mouthOpen, bool hasPropeller = true);
void drawMinivan(int x, int y, uint16_t color);
void drawCar(int x, int y, uint16_t color);
void drawBattery(int x, int y);
void drawStork(int x, int y, bool hasBundle);
void drawCherry(int x, int y);
void drawPretzel(int x, int y);
void drawHeart(int x, int y);

struct SceneInfo {
  const char* name;
  uint32_t durationMs;
};

const int TOTAL_SCENES = 27;

const SceneInfo SCENES[TOTAL_SCENES] = {
  {"0. Main Clock + Chase", 12000},
  {"1. 7:45 AM Boot Sequence", 10000},
  {"2. 4:00 PM Level 256 Kill Screen", 12000},
  {"3. Meet: Two Ms. Pac-Mans", 6000},
  {"4. Meet: Cherry Meets Pretzel", 6000},
  {"5. Ghost Wedding", 7000},
  {"6. Jr: Drone Delivery", 7000},
  {"7. Jr: Triplets Out Cold", 7000},
  {"8. Jr: Stork Backordered", 7000},
  {"9. Jr: Bow & Propeller Cap", 6000},
  {"10. Pac-Van Chases Ghosts", 7000},
  {"11. Student Driver Cones", 7000},
  {"12. Grounded: Pellet Taken", 7000},
  {"13. Empty Nest: Bye Jr", 7000},
  {"14. No Start: No Click", 7000},
  {"15. Monday: Coffee Ghosts", 7000},
  {"16. Friday: Shades On", 6000},
  {"17. Check Engine Light", 7000},
  {"18. Where Is The 10mm?", 7000},
  {"19. Oil Change Overdue", 7000},
  {"20. Are We There Yet?", 7000},
  {"21. Curfew Flashlight Check", 8000},
  {"22. 12:00 PM Lunch Truce", 7000},
  {"23. Afternoon Coffee Refill", 7000},
  {"24. Ghost Pit Crew Stop", 7000},
  {"25. Power Pellet Sugar Rush", 7000},
  {"26. Weekend No Alarm", 7000}
};

inline void drawCornerClock() {
  drawSmallText(43, 1, "10:30", C_YELLOW);
}

inline void renderScene(int id, uint32_t t, uint32_t dur) {
  dma_display->clearScreen();
  float p = (float)(t % dur) / (float)dur; // 0.0 -> 1.0 progress
  bool chomp = ((t / 160) % 2) == 0;

  switch (id) {
    case 0: { // Main Clock + Maze Chase
      drawBigChar(7, 1, '1', C_YELLOW);
      drawBigChar(18, 1, '0', C_YELLOW);
      drawBigChar(29, 1, ':', C_YELLOW);
      drawBigChar(37, 1, '4', C_YELLOW);
      drawBigChar(48, 1, '7', C_YELLOW);
      drawSmallText(12, 18, "WED NOV 11", C_CYAN);
      for (int x = 1; x < 63; x += 2) dma_display->drawPixel(x, 23, C_BLUE);
      int pacX = (int)(p * 64);
      int ghX = (pacX - 18 + 64) % 64;
      for (int px = 4; px < 60; px += 4) if (px > pacX + 7) dma_display->drawPixel(px, 27, C_PELLET);
      if (pacX < 56 && ((t / 200) % 2 == 0)) dma_display->fillRect(59, 26, 2, 2, C_WHITE);
      drawGhost(ghX, 24, C_RED);
      drawMsPacMan(pacX, 24, chomp);
      break;
    }

    case 1: { // 7:45 AM Boot Sequence
      if (p < 0.08f) {
        uint16_t cols[] = {C_RED, C_YELLOW, C_CYAN, C_PINK, C_BLUE, C_WHITE};
        for (int y = 0; y < 32; y++)
          for (int x = 0; x < 64; x++)
            if (random(0, 3) == 0) dma_display->drawPixel(x, y, cols[random(0, 6)]);
      } else if (p < 0.22f) {
        drawSmallText(16, 8, "RAM OK", C_WHITE);
        if (p > 0.14f) drawSmallText(16, 17, "ROM OK", C_WHITE);
      } else if (p < 0.34f) {
        for (int x = 2; x <= 62; x += 10) dma_display->drawFastVLine(x, 2, 28, C_WHITE);
        for (int y = 2; y <= 30; y += 7) dma_display->drawFastHLine(2, y, 60, C_WHITE);
      } else if (p < 0.60f) {
        float sp = (p - 0.34f) / 0.26f;
        if (sp > 0.0f)  { drawGhost(8, 1, C_RED);    drawSmallText(20, 2, "BLINKY", C_RED); }
        if (sp > 0.25f) { drawGhost(8, 9, C_PINK);   drawSmallText(20, 10, "PINKY", C_PINK); }
        if (sp > 0.50f) { drawGhost(8, 17, C_CYAN);  drawSmallText(20, 18, "INKY", C_CYAN); }
        if (sp > 0.75f) { drawGhost(8, 25, C_ORANGE);drawSmallText(20, 26, "SUE", C_ORANGE); }
      } else if (p < 0.76f) {
        drawSmallText(16, 5, "STARRING", C_WHITE);
        drawSmallText(12, 13, "MS PAC-MAN", C_YELLOW);
        drawMsPacMan(28, 22, chomp);
      } else {
        dma_display->drawRect(1, 1, 62, 30, C_BLUE);
        drawSmallText(20, 10, "READY!", C_YELLOW);
        drawMsPacMan(28, 18, true);
      }
      break;
    }

    case 2: { // 4:00 PM Level 256 Kill Screen
      if (p < 0.72f) {
        drawBigChar(7, 1, '4', C_YELLOW);
        drawBigChar(18, 1, ':', C_YELLOW);
        drawSmallText(12, 18, "FRI NOV", C_CYAN);
        for (int x = 1; x < 32; x += 2) dma_display->drawPixel(x, 23, C_BLUE);
        drawSmallText(2, 25, "256", C_WHITE);
        drawMsPacMan(18, 24, chomp);
        randomSeed(256);
        uint16_t gc[] = {C_YELLOW, C_CYAN, C_RED, C_PINK, C_WHITE, C_BLUE};
        int maxCol = min(63, 33 + (int)(p * 60));
        for (int gx = 32; gx < maxCol; gx += 2)
          for (int gy = 1; gy < 31; gy += 2)
            if (random(0, 10) > 3) dma_display->drawPixel(gx, gy, gc[random(0, 6)]);
        if (p > 0.4f) {
          drawGhost(42 + ((t / 250) % 6), 8, C_CYAN);
          drawGhost(38, 22, C_RED);
        }
      } else {
        for (int px = 4; px < 28; px += 4) dma_display->drawPixel(px, 4, C_PELLET);
        drawSmallText(12, 14, "GAME OVER", C_RED);
      }
      break;
    }

    case 3: { // 1. They Meet: Two Ms. Pac-Mans
      drawCornerClock();
      int lx = 10 + (int)(min(p * 1.4f, 1.0f) * 14);
      int rx = 46 - (int)(min(p * 1.4f, 1.0f) * 14);
      drawMsPacMan(lx, 20, chomp, false, true);
      drawMsPacMan(rx, 20, chomp, true, p < 0.7f);
      if (p > 0.55f) drawHeart(29, 9);
      break;
    }

    case 4: { // 2. They Meet: Cherry meets Pretzel
      drawCornerClock();
      int lx = 10 + (int)(min(p * 1.3f, 1.0f) * 12);
      int rx = 46 - (int)(min(p * 1.3f, 1.0f) * 12);
      drawMsPacMan(lx, 20, chomp, false, true);
      drawMsPacMan(rx, 20, chomp, true, false);
      drawCherry(23, 21);
      drawPretzel(35, 21);
      if (p > 0.5f) drawHeart(29, 7);
      break;
    }

    case 5: { // 3. Ghost Wedding
      drawCornerClock();
      drawGhost(6, 14, C_PINK);  // Pinky tossing dots
      drawGhost(24, 12, C_CYAN); // Inky officiating
      drawGhost(14, 22, C_RED);
      drawGhost(40, 22, C_ORANGE);
      drawHeart(29, 23);
      for (int i = 0; i < 6; i++) {
        int dx = 12 + ((t / 80 + i * 7) % 36);
        int dy = 6 + ((i * 5 + t / 120) % 10);
        dma_display->drawPixel(dx, dy, (i % 2) ? C_PINK : C_WHITE);
      }
      break;
    }

    case 6: { // 4. Jr: Drone Delivery
      drawCornerClock();
      int dy = 3 + (int)(sin(p * PI) * 8);
      // Drone rotors + frame
      dma_display->drawFastHLine(25, dy, 5, C_WHITE);
      dma_display->drawFastHLine(33, dy, 5, C_WHITE);
      dma_display->drawFastHLine(27, dy + 1, 9, C_GRAY);
      dma_display->drawFastVLine(31, dy + 1, 3, C_GRAY);
      drawJunior(28, dy + 4, chomp, false);
      drawGhost(28, 17, C_RED);
      drawSmallText(2, 26, "OUT FOR DELIVERY", C_ORANGE);
      break;
    }

    case 7: { // 5. Jr: Triplets Out Cold
      drawCornerClock();
      drawStork(36, 6, true);
      // Parents dizzy/tilted on left
      drawMsPacMan(6, 21, false, false, true);
      drawMsPacMan(15, 22, false, false, false);
      // Triplets
      drawJunior(26, 23, chomp, true);
      drawJunior(34, 23, chomp, true);
      drawJunior(42, 23, chomp, true);
      break;
    }

    case 8: { // 6. Jr: Stork Backordered
      drawCornerClock();
      int sx = 46 - (int)(p * 24);
      drawStork(sx, 8, false);
      drawSmallText(8, 19, "BACKORDERED", C_ORANGE);
      drawMsPacMan(6, 24, false, false, true);
      drawMsPacMan(15, 24, false, false, false);
      break;
    }

    case 9: { // 7. Jr: Bow and Propeller Cap
      drawCornerClock();
      int jx = 18 + (int)(sin(p * PI * 2) * 10);
      drawJunior(jx, 12, chomp, true);
      dma_display->drawPixel(jx + 1, 11, C_RED);
      dma_display->drawPixel(jx + 3, 11, C_RED);
      drawGhost(8, 22, C_PINK);
      drawGhost(17, 22, C_CYAN);
      drawGhost(38, 22, C_PINK);
      drawGhost(47, 22, C_CYAN);
      break;
    }

    case 10: { // 8. Pac-Van Chases Ghosts
      drawCornerClock();
      int vx = (int)(p * 52) - 8;
      drawMinivan(vx, 17, C_RED);
      drawGhost(vx + 24, 21, C_BLUE);
      drawGhost(vx + 33, 20, C_BLUE);
      drawGhost(vx + 42, 22, C_BLUE);
      break;
    }

    case 11: { // 9. Student Driver Cones
      drawCornerClock();
      drawSmallText(2, 8, "STUDENT", C_WHITE);
      drawSmallText(2, 14, "DRIVER", C_WHITE);
      int cx = 22 + (int)(p * 26);
      drawCar(cx, 22, C_GREEN);
      // Traffic cones
      dma_display->drawPixel(18, 27, C_ORANGE); dma_display->drawFastHLine(17, 28, 3, C_ORANGE);
      dma_display->drawPixel(46, 27, C_ORANGE); dma_display->drawFastHLine(45, 28, 3, C_ORANGE);
      break;
    }

    case 12: { // 10. Grounded: Pellet Taken
      drawCornerClock();
      drawSmallText(2, 2, "GROUNDED", C_RED);
      drawMsPacMan(10, 20, false, false, true);
      drawJunior(26, 22, false, true);
      if (((t / 200) % 2) == 0) dma_display->fillRect(19, 19, 2, 2, C_WHITE); // Mom holding pellet
      drawGhost(38, 17, C_BLUE);
      drawGhost(48, 22, C_BLUE);
      break;
    }

    case 13: { // 11. Empty Nest: Bye Jr
      drawCornerClock();
      drawSmallText(16, 10, "BYE JR", C_WHITE);
      drawMsPacMan(8, 21, chomp, false, true);
      drawMsPacMan(17, 21, chomp, false, false);
      int cx = 42 + (int)(p * 24);
      drawCar(cx, 20, C_GREEN);
      break;
    }

    case 14: { // 12. No Start: No Click
      drawCornerClock();
      drawSmallText(2, 2, "NO CLICK", C_WHITE);
      drawGhost(8, 20, C_CYAN);
      drawMsPacMan(24, 20, false, false, false); // Bow moved to battery + post!
      drawBattery(40, 17);
      // Red bow sitting on battery positive terminal
      dma_display->drawPixel(42, 14, C_RED);
      dma_display->drawPixel(43, 15, C_RED);
      dma_display->drawPixel(44, 14, C_RED);
      break;
    }

    case 15: { // 13. Monday: Slow Ghosts with Coffee
      drawCornerClock();
      drawSmallText(2, 2, "MONDAY", C_WHITE);
      int drift = (int)(p * 6);
      uint16_t cols[] = {C_RED, C_PINK, C_CYAN, C_ORANGE};
      for (int i = 0; i < 4; i++) {
        int gx = 4 + i * 14 + drift;
        drawGhost(gx, 21, cols[i]);
        // Coffee mug
        dma_display->fillRect(gx + 8, 24, 3, 3, C_WHITE);
        dma_display->drawPixel(gx + 9, 23, C_BROWN);
      }
      break;
    }

    case 16: { // 14. Friday: Shades On
      drawCornerClock();
      drawSmallText(2, 2, "FRIDAY", C_WHITE);
      int bob = ((t / 250) % 2);
      drawGhost(6,  18 + bob, C_RED, true);
      drawGhost(20, 21 - bob, C_PINK, true);
      drawGhost(34, 19 + bob, C_CYAN, true);
      drawGhost(48, 21 - bob, C_ORANGE, true);
      break;
    }

    case 17: { // 15. Check Engine Light
      drawCornerClock();
      drawSmallText(2, 2, "CEL ON", C_ORANGE);
      drawMinivan(4, 17, C_GREEN);
      if (((t / 300) % 2) == 0) {
        dma_display->fillRect(19, 13, 5, 3, C_ORANGE); // Engine icon
        dma_display->drawFastHLine(20, 12, 3, C_ORANGE);
      }
      drawGhost(34, 20, C_RED);
      dma_display->drawFastHLine(23, 23, 11, C_CYAN); // OBD cable
      break;
    }

    case 18: { // 16. Where Is The 10mm?
      drawCornerClock();
      drawSmallText(2, 2, "WHERE 10MM", C_WHITE);
      uint16_t cols[] = {C_RED, C_PINK, C_CYAN, C_ORANGE};
      for (int i = 0; i < 4; i++) {
        drawGhost(6 + i * 13, 21, cols[i]);
        if (((t / 400 + i) % 2) == 0) dma_display->drawPixel(9 + i * 13, 18, C_WHITE);
      }
      int sx = 48 + (int)(p * 18);
      dma_display->fillRect(sx, 27, 3, 2, C_GRAY); // 10mm socket rolling away
      break;
    }

    case 19: { // 17. Oil Change Overdue
      drawCornerClock();
      bool flash = ((t / 350) % 2) == 0;
      drawSmallText(2, 2, flash ? "5000 MI" : "3000 MI", flash ? C_RED : C_YELLOW);
      drawMsPacMan(10, 20, chomp, false, true);
      dma_display->fillRect(19, 23, 3, 3, C_YELLOW); // Oil can
      int dropY = 26 + ((t / 200) % 5);
      dma_display->drawPixel(23, dropY, C_BROWN);
      drawGhost(42, 20, C_RED);
      break;
    }

    case 20: { // 18. Are We There Yet?
      drawCornerClock();
      drawSmallText(2, 2, "THERE YET?", C_CYAN);
      int vx = 4 + (int)(p * 34);
      drawMinivan(vx, 18, C_RED);
      int step = (t / 400) % 3;
      if (step >= 0) dma_display->drawPixel(vx + 5, 15, C_WHITE);
      if (step >= 1) dma_display->drawPixel(vx + 9, 14, C_YELLOW);
      if (step >= 2) dma_display->drawPixel(vx + 13, 13, C_PINK);
      break;
    }

    case 21: { // 19. Curfew Flashlight Check
      drawCornerClock();
      drawSmallText(2, 2, "CURFEW", C_RED);
      bool lookRight = ((t / 900) % 2) == 0;
      drawMsPacMan(6, 20, false, !lookRight, true);
      if (lookRight) {
        // Flashlight beam cone
        dma_display->drawLine(14, 23, 34, 19, C_YELLOW);
        dma_display->drawLine(14, 24, 34, 27, C_YELLOW);
      }
      int jx = 54 - (int)(p * 26);
      drawJunior(jx, 21, !lookRight && chomp, true);
      break;
    }

    case 22: { // 20. 12:00 PM Lunch Truce
      drawCornerClock();
      drawSmallText(2, 2, "LUNCH", C_GREEN);
      drawMsPacMan(16, 20, chomp, false, true);
      drawCherry(27, 21);
      drawPretzel(33, 21);
      drawGhost(42, 20, C_RED);
      break;
    }

    case 23: { // 21. Afternoon Coffee Refill
      drawCornerClock();
      drawSmallText(2, 2, "REFILL", C_ORANGE);
      // Coffee maker on right
      dma_display->fillRect(48, 17, 7, 10, C_GRAY);
      dma_display->fillRect(50, 21, 3, 4, C_BROWN);
      int ix = (p < 0.6f) ? (8 + (int)((p / 0.6f) * 32)) : (40 - (int)(((p - 0.6f) / 0.4f) * 48));
      drawGhost(ix, 20, C_CYAN);
      break;
    }

    case 24: { // 22. Ghost Pit Crew Stop
      drawCornerClock();
      drawSmallText(2, 2, "PIT STOP", C_CYAN);
      bool lifted = (p > 0.25f && p < 0.75f);
      drawCar(24, lifted ? 19 : 21, C_GREEN);
      drawGhost(15, 20, C_CYAN);
      drawGhost(42, 20, C_ORANGE);
      if (lifted) dma_display->drawFastVLine(31, 26, 4, C_RED); // Jack
      break;
    }

    case 25: { // 23. Power Pellet Sugar Rush
      drawCornerClock();
      drawSmallText(2, 2, "SUGAR RUSH", C_PINK);
      drawMsPacMan(4, 21, false, false, true);
      int jx = 16 + abs((int)((p * 120)) % 80 - 40);
      int jy = 10 + abs((int)((p * 90)) % 32 - 16);
      drawJunior(jx, jy, chomp, true);
      if (((t / 100) % 2) == 0) dma_display->drawCircle(jx + 2, jy + 2, 5, C_CYAN);
      break;
    }

    case 26: { // 24. Weekend No Alarm
      drawCornerClock();
      drawSmallText(2, 2, "NO ALARM", C_YELLOW);
      drawMsPacMan(8, 22, false, false, true);
      drawGhost(20, 22, C_RED);
      drawGhost(31, 22, C_PINK);
      drawGhost(42, 22, C_CYAN);
      drawGhost(53, 22, C_ORANGE);
      int zy = 16 - ((t / 350) % 7);
      dma_display->drawPixel(14, zy, C_CYAN);
      dma_display->drawPixel(36, zy - 1, C_CYAN);
      break;
    }
  }
}
