/*
  GENIUS COM BARRA DE LEDS - 3 CORES - ARDUINO UNO

  Baseado no mesmo hardware do jogo COLOR SHOOTER (Drops, nesse mesmo repo)
  - Fita de led endereçável: WS2812B: pino 5
  https://www.robocore.net/led/fita-de-led-rgb-ws2812-5050-1m
  CUPOM LCSISTEMAS
  - Botao verde: pino 6 + GND
  - Botao vermelho: pino 7 + GND
  - Botao azul: pino 8 + GND
  - Buzzer: pino 10
  - OLED SDA: A4
  - OLED SCL: A5

  REGRAS:
  - O Arduino mostra uma sequencia de cores na fita.
  - O jogador repete usando os 3 botoes.
  - A cada rodada correta, a sequencia aumenta em 1 cor.
  - O valor mostrado no placar e sempre PONTOS x 3.
    Exemplo: 15 rodadas = 45 pontos no placar.
  - NAO usa EEPROM. O recorde existe apenas enquanto o Arduino estiver ligado.

  NOVO RECORDE:
  1) Pisca a fita em cores por 3 segundos.
  2) Apaga a fita.
  3) Acende LEDs vermelhos de baixo para cima, 1 por 1,
     ate RECORDEx3 LEDs.
     Exemplo: recorde Genius 15 -> acende LEDs 0 ate 44 = 45 LEDs.
  4) O recorde fica CRAVADO na fita.
  5) Para iniciar outra partida, aperte qualquer um dos 3 botoes.

  Bibliotecas:
  - FastLED
  - U8g2
*/

#include <Wire.h>
#include <FastLED.h>
#include <U8g2lib.h>

// =====================================================
// OLED
// =====================================================
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(
  U8G2_R0,
  U8X8_PIN_NONE
);

// =====================================================
// HARDWARE
// =====================================================
#define DATA_PIN 5
#define NUM_LEDS 60
#define BRIGHTNESS 80

#define BTN_GREEN 6
#define BTN_RED   7
#define BTN_BLUE  8

#define BUZZER 10

CRGB leds[NUM_LEDS];

// =====================================================
// CORES DO GENIUS
// =====================================================
#define COLOR_GREEN 1
#define COLOR_RED   2
#define COLOR_BLUE  3

// =====================================================
// CONFIGURACOES DO JOGO
// =====================================================
#define MAX_SEQUENCE 100
#define SCORE_MULTIPLIER 3

byte sequenceColors[MAX_SEQUENCE];
byte sequenceLength = 0;
byte playerPosition = 0;

unsigned int score = 0;       // pontuacao real do Genius
unsigned int highScore = 0;   // recorde real do Genius

// true quando terminou a partida e esta aguardando novo jogo
bool waitingForRestart = true;

// =====================================================
// TEMPOS
// =====================================================
const unsigned long COLOR_ON_MS = 430;
const unsigned long COLOR_OFF_MS = 180;
const unsigned long PLAYER_FLASH_MS = 150;
const unsigned long ROUND_DELAY_MS = 650;
const unsigned long DEBOUNCE_MS = 25;

// =====================================================
// BOTOES / DEBOUNCE
// =====================================================
bool lastGreenReading = HIGH;
bool lastRedReading = HIGH;
bool lastBlueReading = HIGH;

bool stableGreenState = HIGH;
bool stableRedState = HIGH;
bool stableBlueState = HIGH;

unsigned long greenDebounceTime = 0;
unsigned long redDebounceTime = 0;
unsigned long blueDebounceTime = 0;

// =====================================================
// UTILITARIOS
// =====================================================
CRGB getColor(byte color) {
  if (color == COLOR_GREEN) return CRGB::Green;
  if (color == COLOR_RED)   return CRGB::Red;
  return CRGB::Blue;
}

unsigned int displayScore(unsigned int geniusScore) {
  return geniusScore * SCORE_MULTIPLIER;
}

void clearStrip() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void fillStrip(byte color) {
  fill_solid(leds, NUM_LEDS, getColor(color));
  FastLED.show();
}

// =====================================================
// SONS
// =====================================================
void playColorTone(byte color, unsigned int durationMs) {
  if (color == COLOR_GREEN) tone(BUZZER, 880, durationMs);
  if (color == COLOR_RED)   tone(BUZZER, 660, durationMs);
  if (color == COLOR_BLUE)  tone(BUZZER, 1047, durationMs);
}

void playRoundSuccessSound() {
  tone(BUZZER, 1200, 90);
  delay(110);
  tone(BUZZER, 1600, 120);
  delay(140);
  noTone(BUZZER);
}

void playGameOverSound() {
  tone(BUZZER, 260, 180);
  delay(210);
  tone(BUZZER, 160, 350);
  delay(380);
  noTone(BUZZER);
}

void playRecordSound() {
  tone(BUZZER, 900, 80);
  delay(90);
  tone(BUZZER, 1200, 80);
  delay(90);
  tone(BUZZER, 1500, 80);
  delay(90);
  tone(BUZZER, 1900, 180);
  delay(210);
  noTone(BUZZER);
}

// =====================================================
// OLED
// =====================================================
void drawGameDisplay() {
  char buffer[12];

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "PONTOS");
    u8g2.drawStr(76, 10, "REC");

    u8g2.setFont(u8g2_font_logisoso20_tn);

    itoa(displayScore(score), buffer, 10);
    u8g2.drawStr(0, 33, buffer);

    itoa(displayScore(highScore), buffer, 10);
    u8g2.drawStr(76, 33, buffer);

    u8g2.drawHLine(0, 39, 128);

    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 54, "Repita a sequencia");
  } while (u8g2.nextPage());
}

void drawWaitingDisplay() {
  char buffer[12];

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_9x15B_tf);
    u8g2.drawStr(12, 17, "GENIUS 3 CORES");

    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 35, "Recorde:");

    itoa(displayScore(highScore), buffer, 10);
    u8g2.drawStr(52, 35, buffer);

    u8g2.drawStr(0, 51, "Aperte qualquer cor");
    u8g2.drawStr(0, 63, "para iniciar");
  } while (u8g2.nextPage());
}

void drawGameOverDisplay(bool newRecord) {
  char buffer[12];

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_9x15B_tf);

    if (newRecord) {
      u8g2.drawStr(8, 17, "NOVO RECORDE!");
    } else {
      u8g2.drawStr(18, 17, "GAME OVER");
    }

    u8g2.setFont(u8g2_font_6x12_tf);

    u8g2.drawStr(0, 38, "Pontos:");
    itoa(displayScore(score), buffer, 10);
    u8g2.drawStr(52, 38, buffer);

    u8g2.drawStr(0, 54, "Recorde:");
    itoa(displayScore(highScore), buffer, 10);
    u8g2.drawStr(52, 54, buffer);
  } while (u8g2.nextPage());
}

// =====================================================
// ANIMACOES
// =====================================================
void flashColor(byte color, unsigned long onTime) {
  fillStrip(color);
  playColorTone(color, onTime - 20);
  delay(onTime);
  clearStrip();
}

void newRecordCelebration() {
  unsigned long startTime = millis();
  byte previousColor = 0;

  // 3 segundos piscando as 3 cores.
  while (millis() - startTime < 3000UL) {
    byte color;

    do {
      color = random(1, 4);
    } while (color == previousColor);

    previousColor = color;

    fillStrip(color);
    playColorTone(color, 90);
    delay(110);

    clearStrip();
    delay(70);
  }

  noTone(BUZZER);
  clearStrip();
  delay(250);
}

void showHighScoreBarAnimated() {
  clearStrip();

  unsigned int ledTarget = displayScore(highScore);

  if (ledTarget > NUM_LEDS) {
    ledTarget = NUM_LEDS;
  }

  // LED 0 = parte de baixo da barra.
  for (unsigned int i = 0; i < ledTarget; i++) {
    leds[i] = CRGB::Red;
    FastLED.show();

    tone(BUZZER, 420 + (i * 8), 20);
    delay(45);
  }

  noTone(BUZZER);
}

void showHighScoreBarFixed() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  unsigned int ledTarget = displayScore(highScore);

  if (ledTarget > NUM_LEDS) {
    ledTarget = NUM_LEDS;
  }

  for (unsigned int i = 0; i < ledTarget; i++) {
    leds[i] = CRGB::Red;
  }

  FastLED.show();
}

// =====================================================
// GENIUS
// =====================================================
void showSequence() {
  delay(450);

  for (byte i = 0; i < sequenceLength; i++) {
    byte color = sequenceColors[i];

    fillStrip(color);
    playColorTone(color, COLOR_ON_MS - 20);
    delay(COLOR_ON_MS);

    clearStrip();
    delay(COLOR_OFF_MS);
  }

  noTone(BUZZER);
}

void addRandomColor() {
  if (sequenceLength >= MAX_SEQUENCE) {
    return;
  }

  sequenceColors[sequenceLength] = random(1, 4);
  sequenceLength++;
}

void startNewGame() {
  score = 0;
  sequenceLength = 0;
  playerPosition = 0;
  waitingForRestart = false;

  clearStrip();
  drawGameDisplay();

  delay(400);

  addRandomColor();
  showSequence();
}

void finishGame() {
  waitingForRestart = true;

  bool newRecord = false;

  if (score > highScore) {
    highScore = score;
    newRecord = true;
  }

  playGameOverSound();
  drawGameOverDisplay(newRecord);

  if (newRecord) {
    delay(400);
    newRecordCelebration();
    playRecordSound();
    showHighScoreBarAnimated();
  } else {
    showHighScoreBarFixed();
  }

  delay(500);
  drawWaitingDisplay();

  // Mantem o recorde cravado na fita ate apertar uma cor.
  showHighScoreBarFixed();
}

void playerPressed(byte color) {
  if (waitingForRestart) {
    startNewGame();
    return;
  }

  // Feedback visual da cor apertada.
  fillStrip(color);
  playColorTone(color, PLAYER_FLASH_MS - 10);
  delay(PLAYER_FLASH_MS);
  clearStrip();
  delay(50);

  // Errou a sequencia.
  if (color != sequenceColors[playerPosition]) {
    finishGame();
    return;
  }

  playerPosition++;

  // Completou toda a sequencia atual.
  if (playerPosition >= sequenceLength) {
    score++;
    playerPosition = 0;

    drawGameDisplay();
    playRoundSuccessSound();

    if (sequenceLength >= MAX_SEQUENCE) {
      finishGame();
      return;
    }

    delay(ROUND_DELAY_MS);

    addRandomColor();
    showSequence();
  }
}

// =====================================================
// BOTOES
// =====================================================
void updateButton(
  byte pin,
  bool &lastReading,
  bool &stableState,
  unsigned long &debounceTime,
  byte color
) {
  bool reading = digitalRead(pin);

  if (reading != lastReading) {
    debounceTime = millis();
    lastReading = reading;
  }

  if (millis() - debounceTime >= DEBOUNCE_MS) {
    if (reading != stableState) {
      stableState = reading;

      // Evento somente no momento em que o botao e pressionado.
      if (stableState == LOW) {
        playerPressed(color);
      }
    }
  }
}

void readButtons() {
  updateButton(
    BTN_GREEN,
    lastGreenReading,
    stableGreenState,
    greenDebounceTime,
    COLOR_GREEN
  );

  updateButton(
    BTN_RED,
    lastRedReading,
    stableRedState,
    redDebounceTime,
    COLOR_RED
  );

  updateButton(
    BTN_BLUE,
    lastBlueReading,
    stableBlueState,
    blueDebounceTime,
    COLOR_BLUE
  );
}

// =====================================================
// SETUP
// =====================================================
void setup() {
  pinMode(BTN_GREEN, INPUT_PULLUP);
  pinMode(BTN_RED, INPUT_PULLUP);
  pinMode(BTN_BLUE, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);

  randomSeed(analogRead(A0));

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(
    leds,
    NUM_LEDS
  );

  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  Wire.begin();

  u8g2.begin();
  u8g2.setContrast(255);

  // Sem EEPROM: highScore sempre comeca em zero ao ligar/reiniciar.
  highScore = 0;

  drawWaitingDisplay();
  showHighScoreBarFixed();
}

// =====================================================
// LOOP
// =====================================================
void loop() {
  readButtons();
}
