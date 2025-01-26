import time
import machine
from machine import I2C
from time import sleep

# Endereço I2C do DS3231
_ADDR = const(104)

# Tentativa de inicializar o módulo RTC
try:
    rtc = machine.RTC()
except:
    print("Cuidado: O módulo não é suportado.")
    rtc = None

# Configuração do barramento I2C
i2c = I2C(0, sda=machine.Pin(0), scl=machine.Pin(1), freq=400000)

# Classe DS3231 para comunicação com o RTC
class DS3231:
    def __init__(self, i2c):
        self.ds3231 = i2c
        if _ADDR not in self.ds3231.scan():
            raise RuntimeError(f"DS3231 não encontrado no barramento I2C no endereço {_ADDR}")

    def get_time(self, data=bytearray(7)):
        def bcd2dec(bcd):
            return ((bcd & 0x70) >> 4) * 10 + (bcd & 0x0F)

        self.ds3231.readfrom_mem_into(_ADDR, 0, data)
        ss, mm, hh, wday, DD, MM, YY = [bcd2dec(x) for x in data]
        YY += 2000
        result = YY, MM, DD, hh, mm, ss, wday - 1, 0
        return result

    def set_time(self, tt=None):
        def gbyte(dec, mod=0):
            tens, units = divmod(dec, 10)
            n = (tens << 4) + units
            n |= 0x80 if mod & 0x0F else mod & 0xC0
            return n.to_bytes(1, "little")

        YY, MM, mday, hh, mm, ss, wday, yday = time.localtime() if tt is None else tt
        self.ds3231.writeto_mem(_ADDR, 0, gbyte(ss))
        self.ds3231.writeto_mem(_ADDR, 1, gbyte(mm))
        self.ds3231.writeto_mem(_ADDR, 2, gbyte(hh))
        self.ds3231.writeto_mem(_ADDR, 3, gbyte(wday + 1))
        self.ds3231.writeto_mem(_ADDR, 4, gbyte(mday))
        self.ds3231.writeto_mem(_ADDR, 5, gbyte(MM, 0x80))
        self.ds3231.writeto_mem(_ADDR, 6, gbyte(YY - 2000))

    def temperature(self):
        def twos_complement(input_value: int, num_bits: int) -> int:
            mask = 2 ** (num_bits - 1)
            return -(input_value & mask) + (input_value & ~mask)

        t = self.ds3231.readfrom_mem(_ADDR, 0x11, 2)
        i = t[0] << 8 | t[1]
        return twos_complement(i >> 6, 10) * 0.25

    def __str__(self, buf=bytearray(0x13)):
        self.ds3231.readfrom_mem_into(_ADDR, 0, buf)
        s = ""
        for n, v in enumerate(buf):
            s = f"{s}0x{n:02x} 0x{v:02x} {v >> 4:04b} {v & 0xF :04b}\n"
            if not (n + 1) % 4:
                s = f"{s}\n"
        return s


# Criação de instância do DS3231
ds = DS3231(i2c)

# Configura o DS3231 com a data e hora atual
ds.set_time()

# Configura o RTC com uma data e hora específica
rtc.datetime((2024, 9, 24, 1, 13, 27, 0, 0))  # (ano, mês, dia, dia da semana, hora, minuto, segundo, subsegundos)

while True:
    # Lê a hora atual do RTC
    current_time = rtc.datetime()

    # Extrai os componentes de data e hora
    year = current_time[0]
    month = current_time[1]
    day = current_time[2]
    weekday = current_time[3]
    hour = current_time[4]
    minute = current_time[5]
    second = current_time[6]

    # Formata a data e a hora
    date_str = "{:02d}/{:02d}/{:04d}".format(day, month, year)
    time_str = "{:02d}:{:02d}:{:02d}".format(hour, minute, second)

    # Exibe a data e hora no console serial
    print("Date: {}, Time: {}".format(date_str, time_str))

    # Aguarda 5 segundos antes de ler novamente
    sleep(5)
