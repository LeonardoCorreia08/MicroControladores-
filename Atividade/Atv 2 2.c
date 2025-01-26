// Configuração dos pinos
#define BUTTON_A 5
#define BUTTON_B 4
#define LED_PIN 13

// Variáveis globais
volatile int buttonPressCount = 0;
volatile bool buttonBPressed = false; // Flag para indicar se o Botão B foi pressionado
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // 50ms para debounce
bool ledBlinking = false;
unsigned long blinkStartTime = 0;
unsigned long previousMillis = 0;  // Usado para controlar o tempo do pisca-pisca
int blinkFrequency = 100;  // Frequência inicial de 10Hz (100ms)
int newBlinkFrequency = 1000; // 1Hz (1000ms)

// Função para a interrupção do Botão A
void buttonISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay) {
    buttonPressCount++;
    lastDebounceTime = currentTime;
  }
}

// Função para a interrupção do Botão B
void buttonBISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay) {
    buttonBPressed = !buttonBPressed;  // Alterna o estado de pressionado
    lastDebounceTime = currentTime;
  }
}

void setup() {
  // Inicializa comunicação serial
  Serial1.begin(115200);
  Serial1.println("Hello, Raspberry Pi Pico!");

  // Configura LED como saída
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Configura Botões A e B como entradas com interrupção
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_A), buttonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_B), buttonBISR, FALLING);
}

void loop() {
  // Verifica se o botão A foi pressionado 5 vezes
  if (buttonPressCount >= 5 && !ledBlinking) {
    Serial1.println("Botão A pressionado 5 vezes. Iniciando pisca-pisca do LED.");
    ledBlinking = true;
    blinkStartTime = millis();  // Reseta o tempo de início
    previousMillis = millis();  // Reseta o tempo de controle de pisca-pisca
    buttonPressCount = 0; // Reseta a contagem
  }

  // Se o Botão B foi pressionado, altera a frequência do LED
  if (buttonBPressed) {
    if (blinkFrequency != newBlinkFrequency) {
      blinkFrequency = newBlinkFrequency;
      Serial1.println("Frequência alterada para 1Hz (1 segundo).");
      buttonBPressed = false;  // Reseta a flag do botão B
    }
  }

  // Pisca o LED com a frequência selecionada (10Hz ou 1Hz)
  if (ledBlinking) {
    unsigned long currentMillis = millis();

    // Alterna o estado do LED baseado na frequência
    if (currentMillis - previousMillis >= blinkFrequency) {
      previousMillis = currentMillis;  // Atualiza o tempo

      // Alterna o estado do LED
      if (digitalRead(LED_PIN) == HIGH) {
        digitalWrite(LED_PIN, LOW);
      } else {
        digitalWrite(LED_PIN, HIGH);
      }
    }

    // Para o pisca-pisca após 10 segundos
    if (currentMillis - blinkStartTime >= 10000) {
      Serial1.println("Pisca-pisca do LED concluído.");
      ledBlinking = false;
      digitalWrite(LED_PIN, LOW); // Garante que o LED fique apagado após 10s
    }
  }
}