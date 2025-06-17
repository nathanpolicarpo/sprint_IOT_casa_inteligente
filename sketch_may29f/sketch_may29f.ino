const int PIR = 2;
const int ledBranco = 13;

//

const int MQ135 = A0;  //Pino Analogico do mq-135.

void acenderLEDAoDetectarPrensenca() {
  int estadoPIR = digitalRead(PIR);

  if (estadoPIR == HIGH) {
    digitalWrite(ledBranco, HIGH);
    Serial.println("LED ligado");
  } else {
    digitalWrite(ledBranco, LOW);
    Serial.println("LED desligado");
  }
}

void verificarVazamentosDeGas(){
  int estadoMQ135 =analogRead(MQ135);

  if

  Serial.println(estadoMQ135);
}


void setup() {  // put your setup code here, to run once:


  //Fala ao programa que o pino 13 sera uma saida/output
  pinMode(ledBranco, OUTPUT);
  pinMode(MQ135, INPUT);

  Serial.println("Calibrando os sensores");
  delay(1000);
  Serial.println("Sensores calibrados os sensores");
}

void loop() {
  // put your main code here, to run repeatedly:
  acenderLEDAoDetectarPrensenca();
  verificarVazamentosDeGas();
}
