#include <DHT.h>
#include <DHT_U.h>
#include <DHT11.h>

//biblioteca

#define DHTPINO A1 //Definimos qual será o pino para o sensor
#define DHTYPE DHT11 //Variavel do tipo DHT

DHT dht(DHTPINO, DHT11); //Inicializa o objeto DHT com o pino e o tipo


//Variaveis globais são da função acenderLEDPresenca
const int PIR = 2;  //apaino digital que o PIR está plugado
const int ledVermelho = 13;

//
const int MQ135 = A0;  // PINO ANALOGICO DO MQ135
const int buzzer = 12;

void acenderLEDPresenca() {
  int estadoPIR = digitalRead(PIR);  //lê o pino digital 2

  //INDENTAÇÃO (ATALHO) - SHIFT + ALT + F
  // HIGH - VALE O VALOR DE 1
  // LOW - VALE O VALOR DE 0
  if (estadoPIR == HIGH) {
    digitalWrite(ledVermelho, HIGH);
    Serial.println("LED ligado");
  } else {
    digitalWrite(ledVermelho, LOW);
    Serial.println("LED apagado");
  }
}
void verificarVazamentoDeGas() {
  int estadoMQ135 = analogRead(MQ135);

  //if - verificar a intensidade do valor do estadoMQ135 >= 600
  //sim - ativar o alarme - alarme_dois_tons();
  //nao - desativar o alarme
  if (estadoMQ135 >= 400) {
    alarme_dois_tons();
  } else {
    noTone(buzzer);
  }
}

void alarme_dois_tons() {
  int freqAlta = 2000;
  int freqBaixa = 800;
  int duracaoTom = 250;

  tone(buzzer, freqAlta, duracaoTom);
  delay(duracaoTom);
  tone(buzzer, freqBaixa, duracaoTom);
  delay(duracaoTom);
}

void verificarTemperaturaEUmuidade(){
float umidade = dht.readHumidity();
float temperatura = dht.readTemperature();//le a temperatura em celcius(padrao)

Serial.println("umidade: " + String(umidade) + "%");
Serial.println("temperatura: " + String(temperatura) + "C");
delay(7000);

}
void setup() {

  Serial.begin(9600);
  pinMode(ledVermelho, OUTPUT);
  pinMode(MQ135, INPUT);
  pinMode(buzzer, OUTPUT);

  //inicializar o sensor DHT
 dht.begin();

  Serial.println("calibrando os sensores");
  delay(10000);
  Serial.println("Sensores calibrados ;) Pode testar ");
}

void loop() {

  //verificarVazamentoDeGas();
  verificarTemperaturaEUmuidade();
}