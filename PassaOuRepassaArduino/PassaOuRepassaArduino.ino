// PASSA OU REPASSA
// FAÇA VOCÊ MESMO
// CRIADO POR LUCAS CARVALHO.

const int ledAzul = 3;        
const int ledVermelho = 2;     
const int botaoAzul = 5;
const int botaoVermelho = 4;
const int rele = 7;  // Pino do relé (SIRENE)

// Define as variáveis de controle
bool jogoAtivo = true;
unsigned long tempoInicio = 0;

void setup() {
  pinMode(rele, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(botaoAzul, INPUT_PULLUP);
  pinMode(botaoVermelho, INPUT_PULLUP);

  // Inicializa o estado inicial dos LEDs
  digitalWrite(ledAzul, HIGH);
  digitalWrite(ledVermelho, HIGH);
  digitalWrite(rele, LOW);
}

void loop() {
  if (jogoAtivo) {
    // Verifica se o botão azul foi pressionado
    if (digitalRead(botaoAzul) == LOW) {
      jogoAtivo = false;
      digitalWrite(ledAzul, HIGH);  // LED azul aceso
      digitalWrite(ledVermelho, LOW);  // LED vermelho apagado
      sirene();
    }

    // Verifica se o botão vermelho foi pressionado
    if (digitalRead(botaoVermelho) == LOW) {
      jogoAtivo = false;
      digitalWrite(ledVermelho, HIGH);  // LED vermelho aceso
      digitalWrite(ledAzul, LOW);  // LED azul apagado
      sirene();
    }

    // Faz os LEDs piscarem
    unsigned long tempoAtual = millis();
    if (tempoAtual - tempoInicio >= 500) {
      tempoInicio = tempoAtual;
      digitalWrite(ledAzul, !digitalRead(ledAzul));
      digitalWrite(ledVermelho, !digitalRead(ledVermelho));
    }
  } else {
    jogoAtivo = true;
  }
}

void sirene() {
  digitalWrite(rele, HIGH);
  delay(5000);
  digitalWrite(rele, LOW);
}
