#include "pico/stdlib.h"
#include <stdio.h>

#define UART0_TX_PIN 0
#define UART0_RX_PIN 1

int main() {
    // Inicializa a UART0 com baudrate de 115200
    uart_init(uart0, 115200);

    // Configura os pinos para UART0
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);

    // Inicializa a comunicação serial padrão
    stdio_init_all();

    char var;  // Variável para armazenar o dado recebido

    while (true) {
        // Solicita o caractere do usuário via console
        printf("Digite um caractere: ");
        var = getchar();  // Lê apenas o caractere digitado
        getchar();        // Descarta o '\n' gerado pelo Enter

        printf("Você digitou: %c\n", var);

        // Envia o caractere para a UART0
        uart_putc(uart0, var);

        // Aguarda até que o caractere seja recebido de volta na UART0
        while (!uart_is_readable(uart0)) {
            tight_loop_contents();  // Aguarda
        }

        // Lê o caractere recebido pela UART0
        char received = uart_getc(uart0);

        // Exibe o caractere recebido e seu valor hexadecimal
        printf("Caractere recebido e enviado de volta: %c (0x%X)\n", received, received);

        // Pequena pausa para evitar fluxo excessivo
        sleep_ms(100);
    }

    return 0;
}

