#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/apps/mqtt.h"
#include "hardware/uart.h"
#include <time.h>

// Wi-Fi Configurações
#define WIFI_SSID "SecurityLife"
#define WIFI_PASSWORD "OLDLIfe"
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define TOPIC "monitoramento/saude"

// Usando UART com HC-05
#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0  // Pino TX do UART (GPIO 0)
#define UART_RX_PIN 1  // Pino RX do UART (GPIO 1)

#define LED_PIN 2
#define MAX_DADOS 100
#define DIAS_ARMAZENAMENTO 3
#define INTERVALO_LEITURA 10  // Em segundos

typedef struct {
    char pressao_arterial[10];  // Exemplo: "120/80"
    int frequencia_cardiaca;     // Exemplo: 72 bpm
    int glicose;                 // Exemplo: 110 mg/dL
    time_t timestamp;            // Momento da leitura
} DadosSaude;

// Função de Callback para Mensagens MQTT
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Mensagem recebida no tópico '%s'\n", topic);
}
DadosSaude historico[MAX_DADOS];
int total_leituras = 0;

// Limites de alerta
#define LIMITE_PRESSAO_SISTOLICA 140
#define LIMITE_PRESSAO_DIASTOLICA 90
#define LIMITE_FREQ_CARDIACA 100
#define LIMITE_GLICOSE 140

// Função para armazenar os dados por 3 dias
void armazenar_dados(DadosSaude dados) {
    if (total_leituras < MAX_DADOS) {
        historico[total_leituras] = dados;
        total_leituras++;
    } else {
        for (int i = 1; i < MAX_DADOS; i++) {
            historico[i - 1] = historico[i];
        }
        historico[MAX_DADOS - 1] = dados;
    }
}

// Simula a geração de dados
int gerar_frequencia_cardiaca() { return rand() % 41 + 60; }
int gerar_glicose() { return rand() % 80 + 70; }
void gerar_pressao_arterial(char *buffer) { snprintf(buffer, 10, "%d/%d", rand() % 40 + 90, rand() % 30 + 60); }

// Enviar dados para MQTT
void enviar_mqtt(mqtt_client_t *client, DadosSaude dados) {
    char payload[100];
    snprintf(payload, sizeof(payload), "{\"pressao_arterial\":\"%s\",\"frequencia_cardiaca\":%d,\"glicose\":%d}", 
             dados.pressao_arterial, dados.frequencia_cardiaca, dados.glicose);
    mqtt_publish(client, TOPIC, payload, strlen(payload), 1, 0, NULL, NULL);
}

void analisar_dados(DadosSaude dados) {
    int sistolica, diastolica;
    sscanf(dados.pressao_arterial, "%d/%d", &sistolica, &diastolica);

    if (sistolica > LIMITE_PRESSAO_SISTOLICA || diastolica > LIMITE_PRESSAO_DIASTOLICA ||
        dados.frequencia_cardiaca > LIMITE_FREQ_CARDIACA || dados.glicose > LIMITE_GLICOSE) {
        printf("ALERTA: Níveis críticos detectados!\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(1000);
        gpio_put(LED_PIN, 0);
    }
}

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    srand(time(NULL));

    printf("Monitoramento Iniciado...\n");

    // Conectar WiFi
    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000);

    // Configuração MQTT
    mqtt_client_t *client = mqtt_client_new();
    ip_addr_t broker_ip;
    dns_gethostbyname(MQTT_BROKER, &broker_ip, NULL, NULL);
    mqtt_connect(client, &broker_ip, MQTT_PORT, NULL, 0, NULL, NULL, NULL, 0, 0);

    while (true) {
        DadosSaude dados;
        gerar_pressao_arterial(dados.pressao_arterial);
        dados.frequencia_cardiaca = gerar_frequencia_cardiaca();
        dados.glicose = gerar_glicose();
        dados.timestamp = time(NULL);
        
        printf("Pressão: %s, FC: %d, Glicose: %d\n", dados.pressao_arterial, dados.frequencia_cardiaca, dados.glicose);
        armazenar_dados(dados);
        analisar_dados(dados);
        enviar_mqtt(client, dados);
        
        sleep_ms(INTERVALO_LEITURA * 1000);
    }
    return 0;
}
