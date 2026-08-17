/*
  Controle de 2 motores N20 (com encoder) via ESP32 + TB6612FNG
  Controle via Bluetooth (app "BT Car Controller"):
    F -> frente
    B -> tras
    L -> vira esquerda (gira no proprio eixo)
    R -> vira direita (gira no proprio eixo)
    S -> para (parada)
  Tambem aceita comandos por texto no Monitor Serial USB (115200 baud):
    frente / tras / p

  Autor: gerado para Lucas
*/

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth classico nao habilitado. Va em Tools > Partition Scheme e escolha uma opcao com Bluetooth habilitado.
#endif

BluetoothSerial SerialBT;
const char* NOME_BLUETOOTH = "ESP32_Carrinho"; // nome que vai aparecer pra parear no celular

// ---------- Pinos do TB6612 ----------
// Motor A (esquerdo)
#define AIN1 27
#define AIN2 26
#define PWMA 25

// Motor B (direito)
#define BIN1 33
#define BIN2 32
#define PWMB 14

#define STBY 13

// ---------- Pinos dos encoders ----------
#define ENC_A_C1 34
#define ENC_A_C2 35
#define ENC_B_C1 4
#define ENC_B_C2 16

// ---------- Config PWM ----------
const int PWM_FREQ = 5000;
const int PWM_RES  = 8;      // resolucao 8 bits -> 0..255
int velocidade = 200;        // velocidade padrao (0..255), ajuste como quiser

// ---------- Parametros fisicos do robo (AJUSTE AQUI) ----------
const float RODA_DIAMETRO_CM   = 3.0;   // diametro da roda em cm
const float PULSOS_POR_VOLTA   = 210.0; // pulsos do encoder por volta completa da RODA
                                         // (se souber so a resolucao do encoder no eixo do motor,
                                         //  multiplique pela reducao da caixa de reducao do N20)
const float DIST_ENTRE_RODAS_CM = 10.0; // distancia entre as duas rodas (trilho/track width)

const float CM_POR_PULSO = (PI * RODA_DIAMETRO_CM) / PULSOS_POR_VOLTA;

// ---------- Contadores de encoder ----------
volatile long encoderA = 0;
volatile long encoderB = 0;

// ---------- Estado da odometria ----------
long ultimoEncoderA = 0;
long ultimoEncoderB = 0;
float odomX = 0.0;      // posicao X (cm)
float odomY = 0.0;      // posicao Y (cm)
float odomTheta = 0.0;  // orientacao (radianos)
float distanciaTotal = 0.0; // distancia total percorrida (cm)

void IRAM_ATTR isrEncoderA() {
  if (digitalRead(ENC_A_C2)) encoderA++; else encoderA--;
}

void IRAM_ATTR isrEncoderB() {
  if (digitalRead(ENC_B_C2)) encoderB++; else encoderB--;
}

// ---------- Funcoes de controle ----------
void setMotorA(int vel) { // vel: -255..255
  if (vel >= 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    vel = -vel;
  }
  ledcWrite(PWMA, vel);
}

void setMotorB(int vel) {
  if (vel >= 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    vel = -vel;
  }
  ledcWrite(PWMB, vel);
}

void virarEsquerda() {
  // gira no proprio eixo: roda esquerda pra tras, roda direita pra frente
  setMotorA(-velocidade);
  setMotorB(velocidade);
}

void virarDireita() {
  // gira no proprio eixo: roda esquerda pra frente, roda direita pra tras
  setMotorA(velocidade);
  setMotorB(-velocidade);
}

void atualizarOdometria() {
  long deltaA = encoderA - ultimoEncoderA;
  long deltaB = encoderB - ultimoEncoderB;
  ultimoEncoderA = encoderA;
  ultimoEncoderB = encoderB;

  float distA = deltaA * CM_POR_PULSO; // distancia percorrida pela roda A (esquerda)
  float distB = deltaB * CM_POR_PULSO; // distancia percorrida pela roda B (direita)

  float distCentro = (distA + distB) / 2.0;
  float deltaTheta = (distB - distA) / DIST_ENTRE_RODAS_CM;

  // atualiza posicao (modelo de robo diferencial)
  odomX += distCentro * cos(odomTheta + deltaTheta / 2.0);
  odomY += distCentro * sin(odomTheta + deltaTheta / 2.0);
  odomTheta += deltaTheta;

  distanciaTotal += fabs(distCentro);
}

void imprimirOdometria() {
  float thetaGraus = odomTheta * 180.0 / PI;
  Serial.printf("Odometria -> X: %.2f cm | Y: %.2f cm | Theta: %.1f graus | Dist. total: %.2f cm | EncA: %ld | EncB: %ld\n",
                odomX, odomY, thetaGraus, distanciaTotal, encoderA, encoderB);
}

void parar() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
  ledcWrite(PWMA, 0);
  ledcWrite(PWMB, 0);
}

void processarComandoBT(char cmd) {
  switch (cmd) {
    case 'F':
      setMotorA(velocidade);
      setMotorB(velocidade);
      Serial.println("[BT] -> Frente");
      break;
    case 'B':
      setMotorA(-velocidade);
      setMotorB(-velocidade);
      Serial.println("[BT] -> Tras");
      break;
    case 'L':
      virarEsquerda();
      Serial.println("[BT] -> Esquerda");
      break;
    case 'R':
      virarDireita();
      Serial.println("[BT] -> Direita");
      break;
    case 'S':
      parar();
      Serial.println("[BT] -> Parado");
      break;
    default:
      // ignora outros caracteres (alguns apps mandam comandos extras tipo G,I,H,J)
      break;
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin(NOME_BLUETOOTH);
  Serial.print("Bluetooth iniciado. Pareie no celular com o nome: ");
  Serial.println(NOME_BLUETOOTH);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH); // habilita a ponte H

  // API nova de PWM do core ESP32 (evita as funcoes antigas depreciadas)
  ledcAttach(PWMA, PWM_FREQ, PWM_RES);
  ledcAttach(PWMB, PWM_FREQ, PWM_RES);

  pinMode(ENC_A_C1, INPUT);
  pinMode(ENC_A_C2, INPUT);
  pinMode(ENC_B_C1, INPUT);
  pinMode(ENC_B_C2, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENC_A_C1), isrEncoderA, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_B_C1), isrEncoderB, RISING);

  parar();

  Serial.println("=================================");
  Serial.println("Pronto! Digite um comando e ENTER:");
  Serial.println("  frente  -> anda para frente");
  Serial.println("  tras    -> anda para tras");
  Serial.println("  p       -> para");
  Serial.println("=================================");
}

void loop() {
  // Comandos via Bluetooth (app BT Car Controller)
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    processarComandoBT(cmd);
  }

  // Comandos via Monitor Serial USB (opcional, para testes com fio)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    if (cmd == "frente") {
      setMotorA(velocidade);
      setMotorB(velocidade);
      Serial.println("-> Indo para frente");
    } else if (cmd == "tras") {
      setMotorA(-velocidade);
      setMotorB(-velocidade);
      Serial.println("-> Indo para tras");
    } else if (cmd == "p") {
      parar();
      Serial.println("-> Parado");
    } else if (cmd.length() > 0) {
      Serial.println("Comando invalido. Use: frente / tras / p");
    }
  }

  // Atualiza e imprime a odometria a cada 200ms
  static unsigned long ultimoPrint = 0;
  if (millis() - ultimoPrint > 200) {
    ultimoPrint = millis();
    atualizarOdometria();
    imprimirOdometria();
  }
}
