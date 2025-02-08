# 🚀 Monitoramento de Saúde com Raspberry Pi Pico W e MQTT

Este projeto implementa um sistema de monitoramento de saúde utilizando um **Raspberry Pi Pico W**. Ele conecta-se a uma rede **Wi-Fi** e publica os dados de um sensor de frequência cardíaca em um broker **MQTT**.

Parcial do projeto para teste 
https://wokwi.com/projects/422297815670019073
====================================

Wi-Fi:
Conecta a BitDogLab ao Wi-Fi para acessar o MQTT Broker.

MQTT:
Utiliza o protocolo para enviar os dados dos sensores à nuvem.

PulseSensor:
Realiza leitura analógica direta da frequência cardíaca.

Bluetooth BLE:
Detecta e conecta ao FreeStyle Libre 2 (glicose) e ao WBP202 (pressão arterial) para obter os dados.

JSON:
Formata os dados coletados em um payload JSON antes de enviá-los ao broker.


Descrição do Código

---

## 📌 **Recursos do Projeto**
✅ **Conexão Wi-Fi** com o Raspberry Pi Pico W  

✅ **Leitura de sensor de frequência cardíaca** via GPIO

✅ **Leitura do bluethoot Freestyle (Glicose)** 

✅ **Leitura de bluethoot WBP202 (Pressao arterial)**

✅ **Publicação dos dados no MQTT** em um broker remoto  

✅ **Formato JSON** para transmissão de dados  

✅ **Simulação no Wokwi** para testes

---

## 🛠 **Requisitos**

- **Placa:** Raspberry Pi Pico W (BitDogLab) 
- **Linguagem:** C  
- **Broker MQTT:** [HiveMQ (gratuito)](https://www.hivemq.com/)  
- **Simulador:** [Wokwi](https://wokwi.com/) *(para testes)*  

---


## 🔹 **Passo a Passo**
### 📌 **1. Configuração Wi-Fi**
O código conecta-se ao Wi-Fi e verifica se há conexão ativa.

### 📌 **2. Conexão com o Broker MQTT**
Usa o broker **HiveMQ** para envio de dados JSON no tópico MQTT.

### 📌 **3. Leitura do Sensor**
Lê o valor do sensor de frequência cardíaca conectado ao **GPIO 26**.

### 📌 **4. Publicação dos Dados**
Os dados do sensor são formatados em **JSON** e publicados a cada **5 segundos** no broker MQTT.

---

## ⚙️ **Como Rodar o Projeto no Wokwi**
1️⃣ Acesse o [Wokwi](https://wokwi.com/)  
2️⃣ Crie um novo projeto e selecione o **Raspberry Pi Pico W**  
3️⃣ Copie e cole o código acima no editor  
4️⃣ Configure os **pinos e periféricos** conforme necessário  
5️⃣ **Execute a simulação** e observe a saída no console  

---

## ⚠️ **Possíveis Problemas e Soluções**
### ❌ **Wi-Fi não conecta**
✔️ Verifique SSID e senha  
✔️ Certifique-se de que sua rede opera em **2.4GHz**  

### ❌ **Erro na conexão MQTT**
✔️ Teste o broker manualmente com o **MQTT Explorer**  
✔️ Verifique se o **broker.hivemq.com** está acessível  

---
### ***Exportando para JSON:***
O formato JSON foi implementado pois  tbem usado para  exportação e pode ser usado se você precisar enviar os dados para um servidor ou ferramenta que suporte JSON.

### ***Exportacao para BD***
Com isso, sempre que você quiser exportar os dados para o banco de dados MySQL, pode chamar a função exportar_para_mysql(), passando os dados a serem registrados.

Conectar o Banco de Dados ao BI:
No BI (Power BI, Tableau, etc.), você pode configurar uma conexão com o banco de dados e realizar consultas para exibir os dados de maneira visual.


## 📢 **Contribuição**
Sinta-se à vontade para contribuir com melhorias! Faça um **fork** do repositório e envie um **pull request** 🚀




