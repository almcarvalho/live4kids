//criado por Lucas Carvalho @br.lcsistemas
const int buttonPin = 2;
const int ledPin = 3;
const int sirenRelayPin = 7;

unsigned long countdownDuration = 120000; // 2 minutos em milissegundos
unsigned long fastBlinkThreshold = 15000; // últimos 15 segundos
unsigned long sirenDuration = 5000;       // sirene toca por 5 segundos

unsigned long startTime;
unsigned long previousBlinkTime = 0;
unsigned long sirenStartTime = 0;

bool ledState = false;
bool buttonPressed = false;
bool sirenActive = false;
bool finished = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(sirenRelayPin, OUTPUT);
  digitalWrite(sirenRelayPin, LOW); // Sirene desligada

  startTime = millis();
}

void loop() {
  unsigned long currentTime = millis();

  if (finished) return;

  // Verifica se o botão foi pressionado
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    buttonPressed = true;
    finished = true;
    digitalWrite(ledPin, LOW);
    return;
  }

  unsigned long elapsed = currentTime - startTime;

  // Controle de piscar o LED
  unsigned long blinkInterval = (elapsed >= countdownDuration - fastBlinkThreshold) ? 200 : 1000;
  if (currentTime - previousBlinkTime >= blinkInterval) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    previousBlinkTime = currentTime;
  }

  // Verifica se o tempo acabou e o botão não foi pressionado
  if (elapsed >= countdownDuration && !buttonPressed && !sirenActive) {
    sirenActive = true;
    sirenStartTime = currentTime;
    digitalWrite(sirenRelayPin, HIGH); // Ativa sirene
    digitalWrite(ledPin, LOW);         // Desliga LED
  }

  // Desliga sirene após 5 segundos
  if (sirenActive && currentTime - sirenStartTime >= sirenDuration) {
    digitalWrite(sirenRelayPin, LOW); // Desliga sirene
    finished = true;
  }
}
