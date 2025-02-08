#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/apps/httpd.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include <time.h>

// Wi-Fi Configurações
#define WIFI_SSID "SecurityLife"
#define WIFI_PASSWORD "OLDLIfe"
#define TCP_SERVER "tcpbin.com"
#define TCP_PORT 4242
#define LED_PIN 2  // LED indicador

// Configuração do Pulse Sensor
#define PULSE_SENSOR_PIN 26  // ADC 0 (GPIO26)

// Usando UART com HC-05
#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define MAX_DADOS 100  // Máximo de amostras armazenadas

// Buffer para armazenar dados da frequência cardíaca
int frequencias[MAX_DADOS];
int total_leituras = 0;

// Função para armazenar dados
void armazenar_frequencia(int bpm) {
    if (total_leituras < MAX_DADOS) {
        frequencias[total_leituras] = bpm;
        total_leituras++;
    } else {
        for (int i = 1; i < MAX_DADOS; i++) {
            frequencias[i - 1] = frequencias[i];  
        }
        frequencias[MAX_DADOS - 1] = bpm;
    }
}

// Análise de Frequência Cardíaca
void analisar_frequencias() {
    if (total_leituras == 0) {
        printf("Nenhum dado coletado ainda.\n");
        return;
    }

    int soma = 0, min = frequencias[0], max = frequencias[0];

    for (int i = 0; i < total_leituras; i++) {
        soma += frequencias[i];
        if (frequencias[i] < min) min = frequencias[i];
        if (frequencias[i] > max) max = frequencias[i];
    }

    float media = (float)soma / total_leituras;

    printf("\nAnálise de Frequência Cardíaca:\n");
    printf("Média: %.2f bpm\n", media);
    printf("Mínima: %d bpm\n", min);
    printf("Máxima: %d bpm\n\n", max);
}

// Leitura do Pulse Sensor
int ler_pulse_sensor() {
    uint16_t leitura = adc_read();  // Lê o ADC
    int bpm = (leitura * 120) / 4096;  // Normaliza para BPM aproximado
    return bpm;
}

// LED pisca de acordo com a frequência cardíaca
void piscar_led(int bpm) {
    int intervalo = 60000 / bpm;  // Tempo entre pulsos em ms
    for (int i = 0; i < 5; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(intervalo);
        gpio_put(LED_PIN, 0);
        sleep_ms(intervalo);
    }
}

static struct tcp_pcb *tcp_client_pcb;

err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        printf("Conexao encerrada.\n");
        tcp_close(tpcb);
        return ERR_OK;
    }

    char buffer[128];
    pbuf_copy_partial(p, buffer, p->len, 0);
    buffer[p->len] = '\0';
    printf("Recebido: %s\n", buffer);

    pbuf_free(p);
    return ERR_OK;
}

err_t tcp_connect_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("Falha na conexão.\n");
        return err;
    }

    printf("Conectado ao servidor.\n");
    const char *message = "Monitoramento de paciente iniciado\n";
    tcp_write(tpcb, message, strlen(message), TCP_WRITE_FLAG_COPY);
    tcp_recv(tpcb, tcp_recv_callback);
    return ERR_OK;
}

static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr == NULL) {
        printf("Falha ao resolver DNS %s\n", name);
        return;
    }

    printf("Resolvido %s para %s\n", name, ipaddr_ntoa(ipaddr));
    tcp_connect(tcp_client_pcb, ipaddr, TCP_PORT, tcp_connect_callback);
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

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    while (true) {
        int frequencia_cardíaca = ler_pulse_sensor();
        printf("Frequência Cardíaca: %d bpm\n", frequencia_cardíaca);
        armazenar_frequencia(frequencia_cardíaca);
        piscar_led(frequencia_cardíaca);
        connect_to_server();
        sleep_ms(10000);
        if (total_leituras % 10 == 0) {
            analisar_frequencias();
        }
        sleep_ms(2000);
    }
    return 0;
}
