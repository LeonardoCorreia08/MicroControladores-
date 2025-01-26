// Configuração dos pinos
#define BUTTON_A 5
#define LED_PIN 13

// Variáveis globais
volatile int buttonPressCount = 0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // 50ms para debounce
bool ledBlinking = false;
unsigned long blinkStartTime = 0;
const int blinkFrequency = 100; // 100ms (10Hz)
unsigned long previousMillis = 0;  // Usado para controlar o tempo do pisca-pisca

void buttonISR() {
  // Verifica debounce
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay) {
    buttonPressCount++;
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

  // Configura botão A como entrada com interrupção
  pinMode(BUTTON_A, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_A), buttonISR, FALLING);
}

void loop() {
  // Verifica se o botão foi pressionado 5 vezes
  if (buttonPressCount >= 5 && !ledBlinking) {
    Serial1.println("Botão pressionado 5 vezes. Iniciando pisca-pisca do LED.");
    ledBlinking = true;
    blinkStartTime = millis();  // Reseta o tempo de início
    previousMillis = millis();  // Reseta o tempo de controle de pisca-pisca
    buttonPressCount = 0; // Reseta a contagem
  }

  // Pisca o LED por 10 segundos a 10 Hz
  if (ledBlinking) {
    unsigned long currentMillis = millis();

    // Alterna o estado do LED a cada 100ms
    if (currentMillis - blinkStartTime < 10000) {  // Verifica se o tempo não passou de 10s
      if (currentMillis - previousMillis >= blinkFrequency) {
        previousMillis = currentMillis; // Atualiza o tempo

        // Alterna o estado do LED
        if (digitalRead(LED_PIN) == HIGH) {
          digitalWrite(LED_PIN, LOW);
        } else {
          digitalWrite(LED_PIN, HIGH);
        }
      }
    } else {
      // Para o pisca-pisca após 10 segundos
      Serial1.println("Pisca-pisca do LED concluído.");
      ledBlinking = false;
      digitalWrite(LED_PIN, LOW); // Garante que o LED fique apagado após 10s
    }
  }
}
