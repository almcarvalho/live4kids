#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// OLED Arduino UNO
// SDA → A4
// SCL → A5

#define GREEN_LED   2
#define RED_LED     3
#define YELLOW_LED  4
#define BLUE_LED    5

#define GREEN_BTN   6
#define RED_BTN     7
#define YELLOW_BTN  8
#define BLUE_BTN    9

#define BUZZER      10

const int tones[] = { 261, 294, 329, 392 };
const int leds[] = { GREEN_LED, RED_LED, YELLOW_LED, BLUE_LED };
const int buttons[] = { GREEN_BTN, RED_BTN, YELLOW_BTN, BLUE_BTN };

#define MAX_SEQ_LEN 100
#define EEPROM_ADDR_HIGHSCORE 0

int sequence[MAX_SEQ_LEN];
int level = 0;
int highScore = 0;

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(leds[i], OUTPUT);
    pinMode(buttons[i], INPUT_PULLUP);
  }

  pinMode(BUZZER, OUTPUT);
  Serial.begin(9600);

  randomSeed(analogRead(A0));

  EEPROM.get(EEPROM_ADDR_HIGHSCORE, highScore);

  if (highScore < 0 || highScore > MAX_SEQ_LEN) {
    highScore = 0;
    EEPROM.put(EEPROM_ADDR_HIGHSCORE, highScore);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Erro ao iniciar o display OLED"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Jogo Genius"));
  display.setCursor(0, 10);
  display.println(F("By @BR.LCSISTEMAS"));
  display.setCursor(0, 25);
  display.print(F("Recorde: "));
  display.println(highScore);
  display.display();

  delay(2000);

  updateOLED();
  playStartSound();
}

void loop() {
  addStepToSequence();
  updateOLED();
  playSequence();

  if (!getPlayerInput()) {
    gameOver();
    delay(2000);
    level = 0;
    updateOLED();
  } else {
    delay(1000);
  }
}

void addStepToSequence() {
  int novaCor;

  do {
    novaCor = random(0, 4);
  } while (level > 0 && novaCor == sequence[level - 1]);

  sequence[level] = novaCor;
  level++;

  if (level > highScore) {
    highScore = level;
    EEPROM.put(EEPROM_ADDR_HIGHSCORE, highScore);
  }

  if (level >= MAX_SEQ_LEN) {
    level = 0;
  }
}

void playSequence() {
  for (int i = 0; i < level; i++) {
    int index = sequence[i];

    digitalWrite(leds[index], HIGH);
    tone(BUZZER, tones[index], 300);
    delay(300);

    digitalWrite(leds[index], LOW);
    noTone(BUZZER);
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
        delay(30);

        while (digitalRead(buttons[i]) == LOW);

        delay(30);
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
  noTone(BUZZER);
  delay(100);
}

void playStartSound() {
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, tones[i], 150);
    digitalWrite(leds[i], HIGH);
    delay(150);

    digitalWrite(leds[i], LOW);
    noTone(BUZZER);
    delay(100);
  }
}

void gameOver() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      digitalWrite(leds[j], HIGH);
    }

    tone(BUZZER, 150, 400);
    delay(400);

    for (int j = 0; j < 4; j++) {
      digitalWrite(leds[j], LOW);
    }

    noTone(BUZZER);
    delay(200);
  }
}

void updateOLED() {
  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Nivel:");
  display.println(level);

  display.setTextSize(2);
  display.setCursor(0, 30);
  display.print("Rec:");
  display.println(highScore);

  display.display();
}
