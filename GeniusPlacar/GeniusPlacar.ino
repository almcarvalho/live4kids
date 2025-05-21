#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pinos
#define GREEN_LED   2
#define RED_LED     3
#define YELLOW_LED  4
#define BLUE_LED    5

#define GREEN_BTN   6
#define RED_BTN     7
#define YELLOW_BTN  8
#define BLUE_BTN    9

#define BUZZER      10

//DISPLAY OLED I2C
//SCL NO ARDUINO UNO -> A5
//SDA NO ARDUINO UNO -> A4

const int tones[] = { 261, 294, 329, 392 }; // C4, D4, E4, G4
const int leds[] = { GREEN_LED, RED_LED, YELLOW_LED, BLUE_LED };
const int buttons[] = { GREEN_BTN, RED_BTN, YELLOW_BTN, BLUE_BTN };

#define MAX_SEQ_LEN 100
int sequence[MAX_SEQ_LEN];
int level = 0;
int highScore = 0;

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(leds[i], OUTPUT);
    pinMode(buttons[i], INPUT_PULLUP);
  }
  pinMode(BUZZER, OUTPUT);
  randomSeed(analogRead(0));
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Display OLED nao encontrado!"));
    while (true);
  }

  // Mensagem de autoria
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Desenvolvido por:");
  display.setCursor(10, 35);
  display.println("@br.LcSistemas");
  display.display();
  delay(3000);

  // Título do jogo
  display.clearDisplay();
  display.setCursor(20, 20);
  display.println("   Genius Game");
  display.display();
  delay(1000);

  updateOLED();
  playStartSound();
}

void loop() {
  addStepToSequence();
  playSequence();
  updateOLED();

  if (!getPlayerInput()) {
    gameOver();
    delay(2000);
    level = 0;
    updateOLED();
  } else {
    if (level > highScore) {
      highScore = level;
      celebrateNewRecord();
    }
    delay(1000);
  }
}

void addStepToSequence() {
  sequence[level] = random(0, 4);
  level++;
}

void playSequence() {
  for (int i = 0; i < level; i++) {
    int index = sequence[i];
    digitalWrite(leds[index], HIGH);
    tone(BUZZER, tones[index], 300);
    delay(300);
    digitalWrite(leds[index], LOW);
    delay(200);
  }
}

bool getPlayerInput() {
  for (int i = 0; i < level; i++) {
    int pressed = waitForButton();
    if (pressed != sequence[i]) {
      return false;
    }
    flashLed(pressed);
  }
  return true;
}

int waitForButton() {
  while (true) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttons[i]) == LOW) {
        while (digitalRead(buttons[i]) == LOW); // espera soltar
        return i;
      }
    }
  }
}

void flashLed(int index) {
  digitalWrite(leds[index], HIGH);
  tone(BUZZER, tones[index], 300);
  delay(300);
  digitalWrite(leds[index], LOW);
  delay(100);
}

void playStartSound() {
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, tones[i], 150);
    digitalWrite(leds[i], HIGH);
    delay(150);
    digitalWrite(leds[i], LOW);
    delay(100);
  }
}

void gameOver() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) digitalWrite(leds[j], HIGH);
    tone(BUZZER, 150, 400);
    delay(400);
    for (int j = 0; j < 4; j++) digitalWrite(leds[j], LOW);
    delay(200);
  }
}

void celebrateNewRecord() {
  unsigned long start = millis();
  while (millis() - start < 5000) {
    tone(BUZZER, 440);
    for (int i = 0; i < 4; i++) digitalWrite(leds[i], HIGH);
    delay(250);
    for (int i = 0; i < 4; i++) digitalWrite(leds[i], LOW);
    delay(250);
  }
  noTone(BUZZER);
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(2);  // Fonte maior
  display.setCursor(0, 0);
  display.print("Nivel: ");
  display.println(level);

  display.setCursor(0, 32);
  display.print("Recorde: ");
  display.println(highScore);

  display.display();
}
