const int touchPin = 2;       // Entrada do labirinto
const int sirenRelayPin = 7;  // Relé da sirene

const int maxErrors = 3;
const unsigned long quickBeepDuration = 200;
const unsigned long sirenDuration = 5000;

int errorCount = 0;
bool lastTouchState = HIGH;

unsigned long sirenStartTime = 0;

enum GameState {
  PLAYING,
  QUICK_BEEP,
  GAME_OVER
};

GameState gameState = PLAYING;

void setup() {
  pinMode(touchPin, INPUT_PULLUP);
  pinMode(sirenRelayPin, OUTPUT);
  digitalWrite(sirenRelayPin, LOW);
}

void loop() {
  unsigned long currentTime = millis();
  bool touchDetected = digitalRead(touchPin) == LOW;

  switch (gameState) {
    case PLAYING:
      if (touchDetected && lastTouchState == HIGH) {
        errorCount++;
        if (errorCount >= maxErrors) {
          digitalWrite(sirenRelayPin, HIGH);
          sirenStartTime = currentTime;
          gameState = GAME_OVER;
        } else {
          digitalWrite(sirenRelayPin, HIGH);
          sirenStartTime = currentTime;
          gameState = QUICK_BEEP;
        }
      }
      break;

    case QUICK_BEEP:
      if (currentTime - sirenStartTime >= quickBeepDuration) {
        digitalWrite(sirenRelayPin, LOW);
        gameState = PLAYING;
      }
      break;

    case GAME_OVER:
      if (currentTime - sirenStartTime >= sirenDuration) {
        digitalWrite(sirenRelayPin, LOW);
        errorCount = 0;
        gameState = PLAYING;
      }
      break;
  }

  lastTouchState = touchDetected;
}
