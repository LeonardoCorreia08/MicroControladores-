#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/apps/httpd.h"
#include "hardware/adc.h"
#include <time.h>

// Wi-Fi Configurações
#define WIFI_SSID "SecurityLife"
#define WIFI_PASSWORD "OLDLIfe"
#define TCP_SERVER "tcpbin.com"
#define TCP_PORT 4242
#define LED_PIN 2  // LED indicador

// Configuração do Pulse Sensor
#define PULSE_SENSOR_PIN 26  // ADC 0 (GPIO26)

// Configuração do Bluetooth
#define BT_DEVICE_NAME "MonitorHealth"  // Nome do dispositivo Bluetooth

#define MAX_DADOS 100  // Máximo de amostras armazenadas

// Buffers para armazenar dados
int frequencias[MAX_DADOS];
int glicoses[MAX_DADOS];
float pressao[MAX_DADOS];
int total_frequencias = 0;
int total_glicose = 0;
int total_pressao = 0;

// Função para armazenar dados
void armazenar_dado(int tipo, int dado) {
    if (tipo == 1 && total_frequencias < MAX_DADOS) {
        frequencias[total_frequencias] = dado;
        total_frequencias++;
    } else if (tipo == 2 && total_glicose < MAX_DADOS) {
        glicoses[total_glicose] = dado;
        total_glicose++;
    }
}

// Função para armazenar pressão
void armazenar_pressao(float dado) {
    if (total_pressao < MAX_DADOS) {
        pressao[total_pressao] = dado;
        total_pressao++;
    }
}

// Função para análise de dados (frequência, glicose, pressão)
void analisar_dados() {
    if (total_frequencias > 0) {
        int soma = 0, min = frequencias[0], max = frequencias[0];
        for (int i = 0; i < total_frequencias; i++) {
            soma += frequencias[i];
            if (frequencias[i] < min) min = frequencias[i];
            if (frequencias[i] > max) max = frequencias[i];
        }
        float media = (float)soma / total_frequencias;
        printf("\nFrequência Cardíaca - Média: %.2f bpm, Mínima: %d bpm, Máxima: %d bpm\n", media, min, max);
    }

    if (total_glicose > 0) {
        int soma = 0, min = glicoses[0], max = glicoses[0];
        for (int i = 0; i < total_glicose; i++) {
            soma += glicoses[i];
            if (glicoses[i] < min) min = glicoses[i];
            if (glicoses[i] > max) max = glicoses[i];
        }
        float media = (float)soma / total_glicose;
        printf("\nGlicose - Média: %.2f mg/dL, Mínima: %d mg/dL, Máxima: %d mg/dL\n", media, min, max);
    }

    if (total_pressao > 0) {
        float soma = 0.0f, min = pressao[0], max = pressao[0];
        for (int i = 0; i < total_pressao; i++) {
            soma += pressao[i];
            if (pressao[i] < min) min = pressao[i];
            if (pressao[i] > max) max = pressao[i];
        }
        float media = soma / total_pressao;
        printf("\nPressão Arterial - Média: %.2f mmHg, Mínima: %.2f mmHg, Máxima: %.2f mmHg\n", media, min, max);
    }
}

// Função para formatar e enviar alerta via MQTT
void enviar_alerta(int glicose, float pressao, int freq_cardiaca) {
    char payload[256];
    snprintf(payload, sizeof(payload),
             "{\"pressao\": \"%.1f\", \"freq_cardiaca\": %d, \"glicose\": %d}",
             pressao, freq_cardiaca, glicose);
    
    const char *topico = "monitoramento/saude";
    
    printf("Enviando alerta para tópico: %s\n", topico);
    printf("Payload: %s\n", payload);
    

}

// Função de callback para recepção de dados via Bluetooth
void bt_recv_callback(uint8_t *data, size_t len) {
    // Processar dados recebidos do FreeStyle Libre 2 e WBP202
    if (len == sizeof(int)) {
        int glicose = *((int *)data);  // dado para glicose
        printf("Glicose recebida: %d mg/dL\n", glicose);
        armazenar_dado(2, glicose);  // Armazena glicose
    } else if (len == sizeof(float)) {
        float pressao = *((float *)data);  // dado para pressão arterial
        printf("Pressão arterial recebida: %.2f mmHg\n", pressao);
        armazenar_pressao(pressao);  // Armazena pressão
    }
}

// Função de conexão Bluetooth
void bt_connect_callback(void) {
    printf("Bluetooth conectado!\n");
    }

// Exportar para CSV
void exportar_para_csv(int frequencia, int glicose, float pressao) {
    FILE *file = fopen("/path/to/dados_monitoramento.csv", "a");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo CSV\n");
        return;
    }

    fprintf(file, "%d,%d,%.2f\n", frequencia, glicose, pressao);
    fclose(file);
}

// Configuração do Bluetooth
void bt_setup() {
    if (cyw43_arch_init()) {
        printf("Falha ao inicializar Bluetooth.\n");
        return;
    }

    cyw43_arch_enable_bt_mode();
    if (cyw43_arch_bt_advertise_name(BT_DEVICE_NAME)) {
        printf("Falha ao anunciar dispositivo Bluetooth.\n");
        return;
    }

    // Adicionar callback para quando o dispositivo for conectado
    cyw43_arch_bt_set_connection_callback(bt_connect_callback);

    printf("Bluetooth configurado e pronto para conexão.\n");
}

void wifi_setup() {
    if (cyw43_arch_init()) {
        printf("Falha ao iniciar Wi-Fi.\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Wi-Fi não conectado.\n");
        cyw43_arch_poll();
        cyw43_arch_deinit();
        sleep_ms(1000);
        wifi_setup();
    }
    printf("Wi-Fi Conectado!\n");
}

void connect_to_server() {
    tcp_client_pcb = tcp_new();
    if (!tcp_client_pcb) {
        printf("Falha ao criar TCP PCB.\n");
        return;
    }
    ip_addr_t server_ip;
    err_t err = dns_gethostbyname(TCP_SERVER, &server_ip, dns_callback, NULL);
    if (err == ERR_OK) {
        tcp_connect(tcp_client_pcb, &server_ip, TCP_PORT, tcp_connect_callback);
    }
}

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    adc_init();
    adc_gpio_init(PULSE_SENSOR_PIN);
    adc_select_input(0);  // Define GPIO26 como entrada ADC

    srand(time(NULL));

    wifi_setup();
    bt_setup();  // Inicializa Bluetooth

    while (true) {
        int glicose = 110;  // Valor da glicose
        float pressao = 135.0;  // Valor da pressão arterial
        int frequencia_cardíaca = ler_pulse_sensor();
        printf("Frequência Cardíaca: %d bpm\n", frequencia_cardíaca);
        armazenar_dado(1, frequencia_cardíaca);  // Armazena a frequência cardíaca
        armazenar_dado(2, glicose);  // Armazena glicose
        armazenar_pressao(pressao);  // Armazena pressão arterial
        piscar_led(frequencia_cardíaca);
        connect_to_server();
        
        // Verifica se os valores exigem um alerta
        enviar_alerta(glicose, pressao, frequencia_cardíaca);
        analisar_dados();
        exportar_para_csv(frequencia_cardíaca, glicose, pressao);
        
        sleep_ms(1000);  // Espera 1 segundo antes de coletar novamente
    }

    return 0;
}
