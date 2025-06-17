verificarTemperaturaEUmuidade();
}#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP32Servo.h>

// --- WiFi & MQTT ---
const char* ssid = "???";//sua rede wifi
const char* password = "???";//senha da sua rede wifi
const char* mqtt_server = "???";//endereço do broker público
const int mqtt_port = 1883;//porta do broker público, geralmente 1883

//Tópicos
const char* topic_led = "escolainteligente/lab19/luzsala";
const char* topic_temp = "escolainteligente/lab19/temperatura";
const char* topic_umid = "escolainteligente/lab19/umidade";
const char* topic_porta = "escolainteligente/lab19/porta";

// --- Pinos ---
const int luzSala = 18;
const int rele = 15;
const int servoMotor = 2;
const int PIR = 5;
const int ledAmarelo = 4;
const int buzzer = 12;
const int mq135 = 34;
#define DHTPIN 33
#define DHTTYPE DHT11

// --- Objetos ---
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);
Servo motor;

// --- Variáveis ---
int contadorGas = 0;
unsigned long ultimaLeitura = 0;

// --- Funções WiFi e MQTT ---
void conectarWiFi() {//verifica conexão wifi para somente depois iniciar o sistema
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void reconectarMQTT() {//verifica e reconecta a conexão com o broker mqtt
  while (!client.connected()) {
    Serial.print("Reconectando MQTT...");
    if (client.connect("ESP32ClientTest")) {
      Serial.println("Conectado!");
      client.subscribe(topic_led);//conecta ao topico do led assim que estabelecer ligação com o broker
      client.subscribe(topic_porta);//conecta ao topico da porta assim que estabelecer ligação com o broker
    } else {
      Serial.print("Falha: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

/**
  Função para tratamento das mensagens de callback/retorno do broker de cada tópico subscrito (led, porta, etc.)

  char* é uma cadeia de caracteres em C como um vetor onde cada caractter/letra está em uma posição, 
  diferente de uma String em C++ que pode ser lida completamente
*/
void tratarMensagem(char* topic, byte* payload, unsigned int length) {//
  String mensagem = "";
  for (int i = 0; i < length; i++) {//concatena todas os char* para se ter o texto completo em String
    mensagem += (char)payload[i];
  }

  Serial.printf("Mensagem recebida [%s]: %s\n", topic, mensagem.c_str());
  
  //led - luz da sala
  if (strcmp(topic, topic_led) == 0) {//tópico atual é o do led?
    if (mensagem == "ligar") {
      digitalWrite(luzSala, HIGH);
    } else if (mensagem == "desligar") {
      digitalWrite(luzSala, LOW);
    }
  }
  
  /*
    Verifica se o tópico recebido é o topico da porta
  é uma função da linguagem C que compara duas strings (topic e topic_porta)
  */
  //porta
  if (strcmp(topic, topic_porta) == 0) {//tópico atual é o da porta?
    if (mensagem == "abrir") {
      destrancarPorta();
      delay(500);
      abrirPortaAutomatico();
    } else if (mensagem == "fechar") {
      fecharPortaAutomatico();
      delay(500);
      trancarPorta();
    }
  }
}

// --- Sensores e atuadores ---

void lerSensorEDisponibilizar() {
  float temperatura = dht.readTemperature();//lê a temperatura do sensor
  float umidade = dht.readHumidity();// lê a umidade do sensor

  if (isnan(temperatura) || isnan(umidade)) {//avisa no console se deu erro
    Serial.println("Erro ao ler DHT!");
    return;
  }

  Serial.printf("Temp: %.1f °C | Umid: %.1f %%\n", temperatura, umidade);//mostra temperatura e umidade no console

  char tempStr[10], umidStr[10];
  dtostrf(temperatura, 4, 1, tempStr); //converte o valor da temperatura do sensor que para string (ele vem float do sensor)
  dtostrf(umidade, 4, 1, umidStr); //converte o valor da umidade do sensor que para string (ele vem float do sensor)
  client.publish(topic_temp, tempStr);//publica a temperatura no tópico, lá no Broker Server
  client.publish(topic_umid, umidStr);//publica a umidade no tópico, lá no Broker Server
}

void acenderLEDAoDetectarPresenca() {
  //acende o led através da leitura do sensor pir que retorna 0 ou 1 que é traduzido para LOW ou HIGH
  digitalWrite(ledAmarelo, digitalRead(PIR));
}

void verificarVazamentoDeGas() {
  int leituraMQ = analogRead(mq135);//lê o valor do sensor de gás
  Serial.print("Gás: ");
  Serial.println(leituraMQ);

  if (leituraMQ >= 800) {//caso o valor seja acima de 800
    if (contadorGas == 0) {//se o contador de gás ainda não foi utilizado
      Serial.println("Gás detectado!");
      delay(3000);
      contadorGas = 1;
    }
    alarme_dois_tons();//ativa o alarme do buzzer
  } else {
    if (contadorGas == 1) {//se o contador de gás já foi utilizado então zera e desativa o buzzer
      contadorGas = 0;
      noTone(buzzer);
    }
  }
}

void alarme_dois_tons() {
  tone(buzzer, 2000, 250);
  delay(250);
  tone(buzzer, 800, 250);
  delay(250);
}

void destrancarPorta() {//trava elétrica 12v
  digitalWrite(rele, HIGH);
  Serial.println("Porta destrancada");
}

void trancarPorta() {//trava elétrica 12v
  digitalWrite(rele, LOW);
  Serial.println("Porta trancada");
}

void abrirPortaAutomatico() {
  motor.write(0);
  Serial.println("Porta aberta");
}

void fecharPortaAutomatico() {
  motor.write(90);
  Serial.println("Porta fechada");
}

// --- Setup ---
void setup() {//configuração inicial dos sensores
  Serial.begin(115200);//inicia a serial do esp32 - ATENÇÃO: tem que colocar a serial do Arduino IDE na mesma velocidade

  //configura os pinos dos sensores
  pinMode(luzSala, OUTPUT);//Inicia o led como saída
  pinMode(PIR, INPUT);//Inicia o sensor PIR como entrada
  pinMode(ledAmarelo, OUTPUT);//inicia o led amarelo como saída que será utilizado em conjunto com o sensor PIR 
  pinMode(mq135, INPUT);//inicia o sensor de gás como entrada
  pinMode(buzzer, OUTPUT);//inicia o sensor buzzer como saída para controle do alarme
  pinMode(rele, OUTPUT);//inicia o sensor relê como saída para controle da trava elétrica
  digitalWrite(rele, LOW);//já fecha a porta através do sensor relê

  motor.attach(servoMotor);//inicia o servo motor (interamente é como se ele desse o comando pinMode() pela biblioteca do Servo)
  motor.write(180);//porta fechada está em 180 grau do motor !!!

  dht.begin();//inicia o sensor dht
  conectarWiFi();//conecta no wifi
  client.setServer(mqtt_server, mqtt_port);//conecta no broker server
  client.setCallback(tratarMensagem);//trata as mensagens recebidas do broker

  Serial.println("Sistema iniciado!");
}

// --- Loop ---
void loop() {
  if (!client.connected()) reconectarMQTT();//se não tem conexão com o broker, tenta reconectar
  client.loop(); //mantém a conexão com o broker serve sempre aberta

  //acenderLEDAoDetectarPresenca();
  //verificarVazamentoDeGas();

  if (millis() - ultimaLeitura > 5000) {
    ultimaLeitura = millis();   //contador de milisegundos
    lerSensorEDisponibilizar(); //Dados do sensor de umidade e temperatura