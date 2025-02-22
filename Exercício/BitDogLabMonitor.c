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
#define LED_PIN 2  // Define o GPIO 2 para o LED

// Configuração do Pulse Sensor
#define PULSE_SENSOR_PIN 26  // ADC 0 (GPIO26)
#define MAX_DADOS 100  // Quantidade máxima de dados armazenados

// Função para armazenar os dados da frequência cardíaca
void armazenar_frequencia(int bpm) {
    if (total_leituras < MAX_DADOS) {
        frequencias[total_leituras] = bpm;
        total_leituras++;
    } else {
        printf("Buffer cheio! Dados antigos serão sobrescritos.\n");
        for (int i = 1; i < MAX_DADOS; i++) {
            frequencias[i - 1] = frequencias[i];  // Desloca os dados
        }
        frequencias[MAX_DADOS - 1] = bpm;
    }
}

// Função para calcular e exibir a análise dos dados armazenados
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

// Função para controlar a quantidade de piscadas com base na frequência cardíaca
void piscar_led(int bpm) {
    // A quantidade de piscadas será inversamente proporcional à frequência cardíaca
    // Quanto maior o BPM, mais rápido será o tempo entre os pisca-piscas
    int intervalo = 1000 / bpm;  // Calculo do intervalo entre as piscadas (em ms)

    // O LED vai piscar um número de vezes com o intervalo baseado no BPM
    for (int i = 0; i < 5; i++) {  // Define que ele pisca 5 vezes
        gpio_put(LED_PIN, 3);  // Liga o LED
        sleep_ms(intervalo);    // Espera o intervalo calculado
        gpio_put(LED_PIN, 3);  // Desliga o LED
        sleep_ms(intervalo);    // Espera o intervalo calculado
    }
}

static struct tcp_pcb *tcp_client_pcb;

err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        printf("Conexao recebida com servidor.\n");
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
        printf("conexao falhou com servidor.\n");
        return err;
    }

    printf("Connected to server.\n");
}
const char *message = "idosinho sendo monitorado\n";
tcp_write(tpcb, message, strlen(message), TCP_WRITE_FLAG_COPY);

tcp_recv(tpcb, tcp_recv_callback);
return ERR_OK;
}

static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr == NULL) {
        printf("Falha ao resolver o DNS %s\n", name);
        return;
    }

    printf("Resolvido %s to %s\n", name, ipaddr_ntoa(ipaddr));
    tcp_connect(tcp_client_pcb, ipaddr, TCP_PORT, tcp_connect_callback);
}

void wifi_setup() {
    if (cyw43_arch_init()) {
        printf("Falha para iniciar o modulo wifi.\n");
        return;
    }

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("SSID nao encontrada.\n");
        cyw43_arch_poll();
        cyw43_arch_deinit();
        sleep_ms(1000);
        wifi_setup();
    }
    printf("WiFi Conectado com suceso.\n");
}

void print_network_info() {
    struct netif *netif = netif_list;
    printf("Network : %c%c\n", netif->name[0], netif->name[1]);
    printf("IP : %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
    printf("Netmask: %s\n", ip4addr_ntoa(netif_ip4_netmask(netif)));
    printf("Gateway: %s\n", ip4addr_ntoa(netif_ip4_gw(netif)));
}

void connect_to_server() {
    tcp_client_pcb = tcp_new();
    if (!tcp_client_pcb) {
        printf("Falha em criar TCP PCB.\n");
        return;
    }

    printf("Resolvendo %s...\n", TCP_SERVER);
    ip_addr_t server_ip;
    err_t err = dns_gethostbyname(TCP_SERVER, &server_ip, dns_callback, NULL);
    if (err == ERR_OK) {
        printf("DNS resolvido : %s\n", ipaddr_ntoa(&server_ip));
        tcp_connect(tcp_client_pcb, &server_ip, TCP_PORT, tcp_connect_callback);
    } else if (err != ERR_INPROGRESS) {
        printf("Falha na resolucao do DNS: %d\n", err);
        tcp_close(tcp_client_pcb);
    }
}

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
    }
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
// Publicar dados no broker MQTT
    mqtt_publish(&mqtt_client, MQTT_TOPIC, payload, strlen(payload), MQTT_QOS_1, 0);
}
// Função de configuração MQTT
void mqtt_config() {
    mqtt_init(&mqtt_client, MQTT_BROKER, MQTT_PORT);
    mqtt_connect(&mqtt_client, "monitoramento/saude", "user", "password");  // Defina credenciais 
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
        int glicose = 110;          // Valor da glicose
        float pressao = 135.0;      // Valor da pressão arterial
        int frequencia_cardíaca = ler_pulse_sensor();

        printf("Frequência Cardíaca: %d bpm\n", frequencia_cardíaca);
        
        armazenar_dado(1, frequencia_cardíaca);  // Armazena a frequência cardíaca
        armazenar_dado(2, glicose);              // Armazena glicose
        armazenar_pressao(pressao);              // Armazena pressão arterial
        piscar_led(frequencia_cardíaca);
        connect_to_server();

        // Armazena a leitura para análise
        armazenar_frequencia(frequencia_cardíaca);

        // Chama a função para piscar o LED de acordo com a frequência
        piscar_led(frequencia_cardíaca);

        // Conectar ao servidor e aguardar reconexão
        connect_to_server();
        sleep_ms(10000); // Aguarda 10 segundos antes de tentar reconectar
        
        // Verifica se os valores exigem um alerta
        enviar_alerta(glicose, pressao, frequencia_cardíaca);
        analisar_dados();
        exportar_para_csv(frequencia_cardíaca, glicose, pressao);

        // A cada 10 leituras, exibe a análise dos dados
        if (total_leituras % 10 == 0) {
            analisar_frequencias();
        }

        sleep_ms(5000);  // Espera 5 segundos antes de coletar novamente
    }

    return 0;
}
