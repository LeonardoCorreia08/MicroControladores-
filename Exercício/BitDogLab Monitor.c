#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <lwip/apps/mqtt.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

// Wi-Fi Configurações
const char* ssid = " SecurityLife";
const char* password = "OLDLIfe";

// Configurações MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "monitoramento/saude";

// MQTT Client
mqtt_client_t* mqtt_client;

// Configurações de Sensores (exemplo fictício)
const uint pulsePin = 26; // Pino GPIO do sensor de pulso

// Protótipo de funções
void setupWiFi();
void mqtt_publish(const char* topic, const char* message);
void mqtt_connect();

// Função de Callback para Mensagens MQTT
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Mensagem recebida no tópico '%s'\n", topic);
}

// Inicialização do Wi-Fi
void setupWiFi() {
    if (cyw43_arch_init()) {
        printf("Falha ao inicializar Wi-Fi!\n");
        return;
    }

    printf("Conectando ao Wi-Fi...\n");
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_async(ssid, password, CYW43_AUTH_WPA2_AES_PSK);

    while (!cyw43_wifi_link_up()) {
        sleep_ms(1000);
        printf("Tentando conectar...\n");
    }
    printf("Wi-Fi conectado com sucesso!\n");
}

// Conexão MQTT
void mqtt_connect() {
    mqtt_client = mqtt_client_new();
    if (!mqtt_client) {
        printf("Falha ao criar cliente MQTT!\n");
        return;
    }

    ip_addr_t broker_ip;
    if (netconn_gethostbyname(mqtt_server, &broker_ip) != ERR_OK) {
        printf("Erro ao resolver o nome do broker MQTT!\n");
        return;
    }

    mqtt_connect_client_info_t mqtt_info = {
        .client_id = "BitDogLabClient",
        .client_user = NULL,
        .client_pass = NULL,
        .keep_alive = 60,
        .will_topic = NULL,
        .will_msg = NULL,
        .will_qos = 0,
        .will_retain = 0
    };

    mqtt_client_connect(mqtt_client, &broker_ip, mqtt_port, NULL, &mqtt_info);
    printf("Conectado ao servidor MQTT!\n");
}

// Publicação MQTT
void mqtt_publish(const char* topic, const char* message) {
    if (mqtt_client) {
        mqtt_publish(mqtt_client, topic, message, strlen(message), 1, 0, NULL);
        printf("Mensagem publicada no tópico '%s': %s\n", topic, message);
    }
}

// Função Principal
int main() {
    stdio_init_all(); // Inicialização da comunicação serial
    setupWiFi();      // Inicialização do Wi-Fi
    mqtt_connect();   // Conexão ao broker MQTT

    // Configuração do pino do sensor
    gpio_init(pulsePin);
    gpio_set_dir(pulsePin, GPIO_IN);

    while (true) {
        // Simulação de leitura do sensor
        int pulseValue = gpio_get(pulsePin);

        // Formatação dos dados em JSON
        char json_data[128];
        snprintf(json_data, sizeof(json_data), "{\"frequencia_cardiaca\": %d}", pulseValue);

        // Publicação dos dados no MQTT
        mqtt_publish(mqtt_topic, json_data);

        sleep_ms(5000); // Intervalo entre medições
    }

    return 0;
}