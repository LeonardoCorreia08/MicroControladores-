#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Definir os pinos dos LEDs RGB
#define RED_PIN 0  // Pino para LED vermelho
#define GREEN_PIN 1  // Pino para LED verde
#define BLUE_PIN 2  // Pino para LED azul

int main() {
    // Configuração do pino para PWM
    gpio_set_function(RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(BLUE_PIN, GPIO_FUNC_PWM);

    // Configuração de PWM para LED vermelho (1 kHz)
    uint red_slice = pwm_gpio_to_slice_num(RED_PIN);
    pwm_set_clkdiv(red_slice, 125);  // Frequência de 1 kHz (125 MHz / 125)
    pwm_set_wrap(red_slice, 1023);  // 1024 ciclos (0 a 1023)
    pwm_set_enabled(red_slice, true);

    // Configuração de PWM para LED azul (10 kHz)
    uint blue_slice = pwm_gpio_to_slice_num(BLUE_PIN);
    pwm_set_clkdiv(blue_slice, 12.5);  // Frequência de 10 kHz (125 MHz / 12.5)
    pwm_set_wrap(blue_slice, 1023);  // 1024 ciclos (0 a 1023)
    pwm_set_enabled(blue_slice, true);

    // Variáveis para controlar o duty cycle do LED vermelho
    int red_duty = 51;  // 5% de duty cycle inicial
    int increment = 51; // Incremento de 5% (1024 * 5 / 100 = 51)
    bool red_direction = true;  // Direção do incremento

    while (1) {
        // Atualiza o duty cycle do LED vermelho a cada 2 segundos
        pwm_set_chan_level(red_slice, PWM_CHAN_A, red_duty);
        
        // Atualiza o duty cycle do LED verde com base no valor do duty cycle do vermelho
        pwm_set_chan_level(blue_slice, PWM_CHAN_A, 1023);  // Mantém o azul ligado para contraste

        if (red_direction) {
            red_duty += increment;
            if (red_duty >= 1024) {
                red_duty = 1024;
                red_direction = false;  // Inverte a direção
            }
        } else {
            red_duty -= increment;
            if (red_duty <= 51) {
                red_duty = 51;
                red_direction = true;  // Inverte a direção
            }
        }

        sleep_ms(2000);  // Aguarda 2 segundos antes de atualizar
    }

    return 0;
}
