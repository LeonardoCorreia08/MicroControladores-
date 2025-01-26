from machine import Pin, PWM
import time

# Configuração dos pinos
led_vermelho = Pin(13, Pin.OUT)  # Configura o pino 13 como saída para o LED vermelho
led_azul = Pin(12, Pin.OUT)     # Configura o pino 12 como saída para o LED azul
led_verde = Pin(11, Pin.OUT)    # Configura o pino 11 como saída para o LED verde

botao1 = Pin(5, Pin.IN, Pin.PULL_UP)  # Configura o pino 5 como entrada para o botão 1 com resistor pull-up
botao2 = Pin(6, Pin.IN, Pin.PULL_UP)  # Configura o pino 6 como entrada para o botão 2 com resistor pull-up

buzzer = PWM(Pin(10))  # Configura o pino 10 como saída PWM para o buzzer

# Função para tocar som no buzzer
def tocar_buzzer(frequencia, duracao):
    """
    Faz o buzzer emitir um som com a frequência e duração especificadas.
    
    Args:
        frequencia (int): Frequência do som em Hz.
        duracao (float): Duração do som em segundos.
    """
    buzzer.freq(frequencia)  # Define a frequência do buzzer
    buzzer.duty_u16(1000)    # Define o volume do som (valor ajustável)
    time.sleep(duracao)      # Toca o som por 'duracao' segundos
    buzzer.duty_u16(0)       # Desliga o som do buzzer

# Loop principal
while True:
    # Verifica o estado do botão 1
    if not botao1.value():  # Se o botão 1 for pressionado (valor = 0)
        led_vermelho.on()   # Liga o LED vermelho
        tocar_buzzer(1000, 0.5)  # Emite som no buzzer com frequência de 1000 Hz por 0.5 segundos
    else:
        led_vermelho.off()  # Desliga o LED vermelho se o botão não estiver pressionado

    # Verifica o estado do botão 2
    if not botao2.value():  # Se o botão 2 for pressionado (valor = 0)
        led_azul.on()       # Liga o LED azul
        tocar_buzzer(1500, 0.5)  # Emite som no buzzer com frequência de 1500 Hz por 0.5 segundos
    else:
        led_azul.off()      # Desliga o LED azul se o botão não estiver pressionado

    time.sleep(0.1)  # Pequeno atraso para evitar leituras excessivas dos botões
