#include <stdio.h>             // Biblioteca padrão para entrada e saída, utilizada para printf.
#include "pico/stdlib.h"       // Biblioteca padrão para funções básicas do Pico, como GPIO e temporização.
#include "hardware/adc.h"      // Biblioteca para controle do ADC (Conversor Analógico-Digital).


// Definições
#define ADC_TEMPERATURE_CHANNEL 4   // Canal ADC que corresponde ao sensor de temperatura interno

// Função para converter o valor lido do ADC para temperatura em Fahrenheit
float adc_to_temperature(uint16_t adc_value) {
    // Constantes fornecidas no datasheet do RP2040
    const float conversion_factor = 3.3f / (1 << 12);  // Conversão de 12 bits (0-4095) para 0-3.3V
    float voltage = adc_value * conversion_factor;     // Converte o valor ADC para tensão
    float temperature_celsius = 27.0f - (voltage - 0.706f) / 0.001721f;  // Equação fornecida para conversão

    // Converte de Celsius para Fahrenheit
    float temperature_fahrenheit = (temperature_celsius * 9.0f / 5.0f) + 32.0f;
    return temperature_fahrenheit;
}

int main() {
    // Inicializa a comunicação serial para permitir o uso de printf
    stdio_init_all();

    // Inicializa o módulo ADC do Raspberry Pi Pico
    adc_init();

    // Seleciona o canal 4 do ADC (sensor de temperatura interno)
    adc_set_temp_sensor_enabled(true);  // Habilita o sensor de temperatura interno
    adc_select_input(ADC_TEMPERATURE_CHANNEL);  // Seleciona o canal do sensor de temperatura

    // Loop infinito para leitura contínua do valor de temperatura
    while (true) {
        // Lê o valor do ADC no canal selecionado (sensor de temperatura)
        uint16_t adc_value = adc_read();

        // Converte o valor do ADC para temperatura em Fahrenheit
        float temperature = adc_to_temperature(adc_value);

        // Imprime a temperatura na comunicação serial
        printf("Temperatura: %.2f °F\n", temperature);

        // Atraso de 1000 milissegundos (1 segundo) entre as leituras
        sleep_ms(1000);
    }

    return 0;
}