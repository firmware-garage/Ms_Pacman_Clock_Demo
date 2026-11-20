#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "cutscenes.h"

// ==========================================
// 1. HARDWARE PINOUT (ESP32-S3 N16R8 -> 64x32 HUB75)
// ==========================================
#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANEL_CHAIN 1

#define R1_PIN 4
#define G1_PIN 5
#define B1_PIN 6
#define R2_PIN 7
#define G2_PIN 15
#define B2_PIN 16
#define A_PIN  17
#define B_PIN  18
#define C_PIN  8
#define D_PIN  9
#define E_PIN  -1  // Not used on 64x32 (1/16 scan)
#define LAT_PIN 11
#define OE_PIN  12
#define CLK_PIN 13

// Uncomment if your 64x32 panel uses FM6126A driver chips and stays blank
// #define USE_FM6126A

MatrixPanel_I2S_DMA *dma_display = nullptr;
WebServer server(80);

// Color Palette
uint16_t C_BLACK, C_YELLOW, C_RED, C_PINK, C_CYAN, C_ORANGE, C_BLUE, C_WHITE, C_GREEN, C_PELLET, C_GRAY, C_BROWN;

// Playback Controller State
int activeScene = 0;
float speedMultiplier = 1.0f;
bool autoCycle = true;
int brightnessVal = 120;
uint32_t virtualTimeMs = 0;
uint32_t lastFrameMs = 0;

// ==========================================
// 2. FONTS & SPRITES (NATIVE 64x32)
// ==========================================
const uint8_t FONT_5X8[11][8] = {
  {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E,0x00}, // 0
  {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E,0x00}, // 1
  {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F,0x00}, // 2
  {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E,0x00}, // 3
  {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02,0x00}, // 4
  {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E,0x00}, // 5
  {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E,0x00}, // 6
  {0x1F,0x01,0x02,0x04,0x08,0x08,0x08,0x00}, // 7
  {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E,0x00}, // 8
  {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C,0x00}, // 9
  {0x00,0x00,0x04,0x00,0x00,0x04,0x00,0x00}  // :
};

void drawBigChar(int x, int y, char c, uint16_t color) {
  int idx = (c >= '0' && c <= '9') ? (c - '0') : (c == ':' ? 10 : -1);
  if (idx < 0) return;
  for (int r = 0; r < 8; r++)
    for (int col = 0; col < 5; col++)
      if (FONT_5X8[idx][r] & (1 << (4 - col)))
        dma_display->fillRect(x + col * 2, y + r * 2, 2, 2, color);
}

const uint8_t FONT_3X5[40][5] = {
  {0x2,0x5,0x7,0x5,0x5}, {0x6,0x5,0x6,0x5,0x6}, {0x3,0x4,0x4,0x4,0x3}, {0x6,0x5,0x5,0x5,0x6},
  {0x7,0x4,0x6,0x4,0x7}, {0x7,0x4,0x6,0x4,0x4}, {0x3,0x4,0x5,0x5,0x3}, {0x5,0x5,0x7,0x5,0x5},
  {0x7,0x2,0x2,0x2,0x7}, {0x1,0x1,0x1,0x5,0x2}, {0x5,0x5,0x6,0x5,0x5}, {0x4,0x4,0x4,0x4,0x7},
  {0x5,0x7,0x5,0x5,0x5}, {0x5,0x7,0x7,0x5,0x5}, {0x2,0x5,0x5,0x5,0x2}, {0x6,0x5,0x6,0x4,0x4},
  {0x2,0x5,0x5,0x7,0x3}, {0x6,0x5,0x6,0x5,0x5}, {0x3,0x4,0x2,0x1,0x6}, {0x7,0x2,0x2,0x2,0x2},
  {0x5,0x5,0x5,0x5,0x7}, {0x5,0x5,0x5,0x5,0x2}, {0x5,0x5,0x5,0x7,0x5}, {0x5,0x5,0x2,0x5,0x5},
  {0x5,0x5,0x2,0x2,0x2}, {0x7,0x1,0x2,0x4,0x7},
  {0x7,0x5,0x5,0x5,0x7}, {0x2,0x6,0x2,0x2,0x7}, {0x7,0x1,0x7,0x4,0x7}, {0x7,0x1,0x3,0x1,0x7},
  {0x5,0x5,0x7,0x1,0x1}, {0x7,0x4,0x7,0x1,0x7}, {0x7,0x4,0x7,0x5,0x7}, {0x7,0x1,0x1,0x2,0x2},
  {0x7,0x5,0x7,0x5,0x7}, {0x7,0x5,0x7,0x1,0x7},
  {0x0,0x2,0x0,0x2,0x0}, {0x2,0x2,0x2,0x0,0x2}, {0x0,0x0,0x7,0x0,0x0}, {0x6,0x1,0x2,0x0,0x2}
};

void drawSmallText(int x, int y, const char* str, uint16_t color) {
  int cx = x;
  for (int i = 0; str[i] != '\0'; i++) {
    char c = str[i];
    int idx = -1;
    if (c >= 'A' && c <= 'Z') idx = c - 'A';
    else if (c >= 'a' && c <= 'z') idx = c - 'a';
    else if (c >= '0' && c <= '9') idx = 26 + (c - '0');
    else if (c == ':') idx = 36;
    else if (c == '!') idx = 37;
    else if (c == '-') idx = 38;
    else if (c == '?') idx = 39;

    if (idx >= 0) {
      for (int r = 0; r < 5; r++)
        for (int col = 0; col < 3; col++)
          if (FONT_3X5[idx][r] & (1 << (2 - col)))
            dma_display->drawPixel(cx + col, y + r, color);
    }
    cx += 4;
  }
}

const uint8_t SPRITE_GHOST[7] = {0x1C, 0x3E, 0x7F, 0x5D, 0x7F, 0x7F, 0x55};
const uint8_t SPRITE_PAC_OPEN[7] = {0x1C, 0x3E, 0x78, 0x70, 0x78, 0x3E, 0x1C};
const uint8_t SPRITE_PAC_CLOSED[7] = {0x1C, 0x3E, 0x7F, 0x7F, 0x7F, 0x3E, 0x1C};

void drawGhost(int x, int y, uint16_t bodyColor, bool shades) {
  for (int r = 0; r < 7; r++)
    for (int c = 0; c < 7; c++)
      if (SPRITE_GHOST[r] & (1 << (6 - c)))
        dma_display->drawPixel(x + c, y + r, bodyColor);
  if (shades) {
    dma_display->drawFastHLine(x + 1, y + 2, 5, C_BLACK);
    dma_display->fillRect(x + 1, y + 2, 2, 2, C_BLACK);
    dma_display->fillRect(x + 4, y + 2, 2, 2, C_BLACK);
  } else {
    dma_display->drawPixel(x + 2, y + 2, C_WHITE);
    dma_display->drawPixel(x + 5, y + 2, C_WHITE);
    dma_display->drawPixel(x + 2, y + 3, C_BLUE);
    dma_display->drawPixel(x + 5, y + 3, C_BLUE);
  }
}

void drawMsPacMan(int x, int y, bool mouthOpen, bool flipX, bool hasBow) {
  const uint8_t* bmp = mouthOpen ? SPRITE_PAC_OPEN : SPRITE_PAC_CLOSED;
  for (int r = 0; r < 7; r++) {
    for (int c = 0; c < 7; c++) {
      int srcCol = flipX ? c : (6 - c);
      if (bmp[r] & (1 << srcCol))
        dma_display->drawPixel(x + c, y + r, C_YELLOW);
    }
  }
  if (hasBow) {
    int bx = flipX ? (x + 3) : (x + 1);
    dma_display->drawPixel(bx, y - 1, C_RED);
    dma_display->drawPixel(bx + 1, y, C_RED);
    dma_display->drawPixel(bx + 2, y - 1, C_RED);
  }
}

void drawJunior(int x, int y, bool mouthOpen, bool hasPropeller) {
  const uint8_t* bmp = mouthOpen ? SPRITE_PAC_OPEN : SPRITE_PAC_CLOSED;
  for (int r = 1; r < 6; r++)
    for (int c = 1; c < 6; c++)
      if (bmp[r] & (1 << (6 - c)))
        dma_display->drawPixel(x + c - 1, y + r - 1, C_YELLOW);
  if (hasPropeller) {
    dma_display->drawPixel(x + 2, y - 1, C_BLUE);
    dma_display->drawFastHLine(x + 1, y - 2, 3, C_CYAN);
  }
}

void drawMinivan(int x, int y, uint16_t color) {
  dma_display->fillRect(x + 2, y, 13, 3, color);
  dma_display->fillRect(x, y + 3, 16, 4, color);
  dma_display->fillRect(x + 4, y + 1, 3, 2, C_CYAN);
  dma_display->fillRect(x + 9, y + 1, 4, 2, C_CYAN);
  dma_display->fillRect(x + 2, y + 7, 3, 2, C_GRAY);
  dma_display->fillRect(x + 11, y + 7, 3, 2, C_GRAY);
}

void drawCar(int x, int y, uint16_t color) {
  dma_display->fillRect(x + 3, y, 6, 2, color);
  dma_display->fillRect(x, y + 2, 12, 3, color);
  dma_display->fillRect(x + 2, y + 5, 2, 2, C_GRAY);
  dma_display->fillRect(x + 8, y + 5, 2, 2, C_GRAY);
}

void drawBattery(int x, int y) {
  dma_display->fillRect(x + 1, y, 2, 2, C_RED);
  dma_display->fillRect(x + 7, y, 2, 2, C_BLUE);
  dma_display->fillRect(x, y + 2, 10, 8, C_GRAY);
  dma_display->drawFastHLine(x + 1, y + 4, 8, C_WHITE);
}

void drawStork(int x, int y, bool hasBundle) {
  dma_display->drawFastHLine(x, y + 2, 4, C_ORANGE);
  dma_display->fillRect(x + 4, y + 1, 3, 3, C_WHITE);
  dma_display->fillRect(x + 6, y + 3, 5, 3, C_WHITE);
  dma_display->drawFastHLine(x + 6, y + 1, 4, C_CYAN);
  if (hasBundle) dma_display->fillCircle(x + 2, y + 6, 2, C_WHITE);
}

void drawCherry(int x, int y) {
  dma_display->drawLine(x + 1, y + 3, x + 3, y, C_GREEN);
  dma_display->drawLine(x + 4, y + 3, x + 3, y, C_GREEN);
  dma_display->fillRect(x, y + 3, 2, 2, C_RED);
  dma_display->fillRect(x + 3, y + 3, 2, 2, C_RED);
}

void drawPretzel(int x, int y) {
  dma_display->drawRect(x, y + 1, 3, 3, C_ORANGE);
  dma_display->drawRect(x + 2, y + 1, 3, 3, C_ORANGE);
}

void drawHeart(int x, int y) {
  dma_display->drawPixel(x + 1, y, C_PINK);
  dma_display->drawPixel(x + 3, y, C_PINK);
  dma_display->fillRect(x, y + 1, 5, 2, C_PINK);
  dma_display->fillRect(x + 1, y + 3, 3, 1, C_PINK);
  dma_display->drawPixel(x + 2, y + 4, C_PINK);
}

// ==========================================
// 3. WEB GUI (AP MODE @ 192.168.4.1)
// ==========================================
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>Ms. Pac-Man 64x32 Demo</title><style>";
  html += "body{background:#111;color:#ff0;font-family:monospace;padding:16px;max-width:480px;margin:auto}";
  html += "h2{color:#0ff;margin-top:0}select,input,button{width:100%;padding:12px;margin:8px 0;font-size:16px;background:#222;color:#fff;border:1px solid #44f;border-radius:6px}";
  html += "button{background:#22f;font-weight:bold;cursor:pointer}.row{display:flex;gap:8px}";
  html += "</style></head><body><h2>MS. PAC-MAN 64x32 DEMO</h2>";
  html += "<label>Select Cutscene:</label><select id='sc' onchange='upd()'>";
  for (int i = 0; i < TOTAL_SCENES; i++) {
    html += "<option value='" + String(i) + "'" + (i == activeScene ? " selected" : "") + ">" + String(SCENES[i].name) + "</option>";
  }
  html += "</select><div class='row'><button onclick='step(-1)'>&lt; PREV</button><button onclick='step(1)'>NEXT &gt;</button></div>";
  html += "<label>Playback Speed: <span id='spv'>" + String(speedMultiplier, 2) + "x</span></label>";
  html += "<input type='range' id='sp' min='0.25' max='3.0' step='0.25' value='" + String(speedMultiplier, 2) + "' oninput='upd()'>";
  html += "<label>Brightness: <span id='brv'>" + String(brightnessVal) + "</span></label>";
  html += "<input type='range' id='br' min='15' max='230' step='5' value='" + String(brightnessVal) + "' oninput='upd()'>";
  html += "<label><input type='checkbox' id='ac' style='width:auto' " + String(autoCycle ? "checked" : "") + " onchange='upd()'> Auto-Cycle All Scenes</label>";
  html += "<script>function upd(){let s=document.getElementById('sc').value,sp=document.getElementById('sp').value,br=document.getElementById('br').value,ac=document.getElementById('ac').checked?1:0;";
  html += "document.getElementById('spv').innerText=sp+'x';document.getElementById('brv').innerText=br;";
  html += "fetch(`/set?s=${s}&sp=${sp}&br=${br}&ac=${ac}`);}";
  html += "function step(d){let el=document.getElementById('sc');el.selectedIndex=(el.selectedIndex+d+el.length)%el.length;document.getElementById('ac').checked=false;upd();}</script></body></html>";
  server.send(200, "text/html", html);
}

void handleSet() {
  if (server.hasArg("s")) {
    int ns = server.arg("s").toInt();
    if (ns != activeScene) { activeScene = constrain(ns, 0, TOTAL_SCENES - 1); virtualTimeMs = 0; }
  }
  if (server.hasArg("sp")) speedMultiplier = constrain(server.arg("sp").toFloat(), 0.25f, 3.0f);
  if (server.hasArg("br")) {
    brightnessVal = constrain(server.arg("br").toInt(), 15, 230);
    dma_display->setBrightness8(brightnessVal);
  }
  if (server.hasArg("ac")) autoCycle = (server.arg("ac").toInt() == 1);
  server.send(200, "text/plain", "OK");
}

// ==========================================
// 4. SETUP & LOOP
// ==========================================
void setup() {
  Serial.begin(115200);

  HUB75_I2S_CFG::i2s_pins _pins = {
    R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN,
    A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN
  };
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  mxconfig.clkphase = false;
#ifdef USE_FM6126A
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;
#endif

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(brightnessVal);

  C_BLACK  = dma_display->color565(0, 0, 0);
  C_YELLOW = dma_display->color565(255, 220, 0);
  C_RED    = dma_display->color565(255, 25, 25);
  C_PINK   = dma_display->color565(255, 160, 210);
  C_CYAN   = dma_display->color565(0, 240, 255);
  C_ORANGE = dma_display->color565(255, 165, 0);
  C_BLUE   = dma_display->color565(33, 33, 255);
  C_WHITE  = dma_display->color565(255, 255, 255);
  C_GREEN  = dma_display->color565(0, 220, 60);
  C_PELLET = dma_display->color565(255, 190, 160);
  C_GRAY   = dma_display->color565(150, 150, 160);
  C_BROWN  = dma_display->color565(160, 90, 30);

  // Start Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP("MsPacMan-Demo", "mspacman");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();

  lastFrameMs = millis();
}

void loop() {
  server.handleClient();

  uint32_t now = millis();
  uint32_t dt = now - lastFrameMs;
  lastFrameMs = now;

  virtualTimeMs += (uint32_t)(dt * speedMultiplier);

  uint32_t dur = SCENES[activeScene].durationMs;
  if (autoCycle && virtualTimeMs >= dur) {
    virtualTimeMs = 0;
    activeScene = (activeScene + 1) % TOTAL_SCENES;
  }

  renderScene(activeScene, virtualTimeMs, dur);
  delay(25);
}
