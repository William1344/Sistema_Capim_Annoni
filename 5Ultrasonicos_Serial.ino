/*
     Projeto:

  ---- Componentes: ----

  5 Sensores de Ultrassom HC-04
  1 Giroscópio

  ---- Funcionamento: ----

*/


// ==============================================================================
// ------------ Bibliotecas -----------------------

#include <Arduino.h>
#include <Wire.h>

// ==============================================================================
// ------------ Portas -----------------------

//HC-04 01
#define hc1_trig_pin 4
#define hc1_echo_pin 5

//HC-04 02
#define hc2_trig_pin 6
#define hc2_echo_pin 7

//HC-04 03
#define hc3_trig_pin 8
#define hc3_echo_pin 9

//HC-04 04
#define hc4_trig_pin 10
#define hc4_echo_pin 11

//HC-04 05
#define hc5_trig_pin 12
#define hc5_echo_pin 13

// ==============================================================================
// ------------ Variáveis e Constantes -----------------------


//Endereco I2C do MPU6050
const int MPU = 0x68;
//SCL     ->     A5
//SDA     ->     A4

//Variaveis para armazenar valores do acelerometro e giroscópio
int AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

//Numero de iterações da média móvel
#define num 20

//Numero de sensores HC
#define sensores_HC 5

//Ajustes
int delay_Loop = 60; //Delay para o loop do arduino em ms
//Distância para Acionar o motor
double distanceToActivate = 18; //Aciona quando está a esta distância do sensor

//Média móvel
int values [num][sensores_HC];

//Média móvel 2
long accf[sensores_HC];
int cont_num[sensores_HC];

//DistânciaMedida
double distance[sensores_HC];
double distance_filtered[sensores_HC];

int read_ADC[sensores_HC];
long set_time [sensores_HC];
long rememTime[sensores_HC];

//Estado
int flag = 0;

// ==============================================================================
// ------------ Protótipo das Funções  -----------------------
long moving_average(int sig, int sensor);
long moving_average2(int sig, int sensor);
long sensorHC (int trigpin , int echopin);
float mediaMovel(float xis) ;
void giroscopio( );
void print_giroscopio( );
void print_giroscopioSerialStudio( );
void print_HC( );
void print_HC_SerialStudio( );

// ==============================================================================
// ------------ Setup -----------------------
void setup()
{

  //HC
  pinMode( hc1_trig_pin, OUTPUT );   //HC-04 trig
  pinMode( hc1_echo_pin, INPUT  );   //HC-04 echo
  pinMode( hc2_trig_pin, OUTPUT );   //HC-04 trig
  pinMode( hc2_echo_pin, INPUT  );   //HC-04 echo
  pinMode( hc3_trig_pin, OUTPUT );   //HC-04 trig
  pinMode( hc3_echo_pin, INPUT  );   //HC-04 echo
  pinMode( hc4_trig_pin, OUTPUT );   //HC-04 trig
  pinMode( hc4_echo_pin, INPUT  );   //HC-04 echo
  pinMode( hc5_trig_pin, OUTPUT );   //HC-04 trig
  pinMode( hc5_echo_pin, INPUT  );   //HC-04 echo

  delay(500);

  //Serial
  Serial.begin(9600);
  Serial.println("Sistema incializado");

  //I2C
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  //Inicializa o acelerômetro
  Wire.write(0);
  Wire.endTransmission(true);


  //media2
  for (int i = 0; i < sensores_HC; i++ ) {
    accf[i] = 0;
    cont_num[i] = 0;
  }

}

// ==============================================================================
// ------------ Loop -----------------------
void loop()
{
  // HC acionando alternadamente, saltando ao menos sensor 1 - 3 - 5 - 2 - 4
  distance_filtered[0] = moving_average(sensorHC(hc1_trig_pin , hc1_echo_pin ), 0); //HC-04 01
  distance_filtered[2] = moving_average(sensorHC(hc3_trig_pin , hc3_echo_pin ), 2); //HC-04 03
  distance_filtered[4] = moving_average(sensorHC(hc5_trig_pin , hc5_echo_pin ), 4); //HC-04 05
  distance_filtered[1] = moving_average(sensorHC(hc2_trig_pin , hc2_echo_pin ), 1); //HC-04 02
  distance_filtered[3] = moving_average(sensorHC(hc4_trig_pin , hc4_echo_pin ), 3); //HC-04 04

  giroscopio( );

  print_HC( );
  print_giroscopio( );

  delay (delay_Loop); //Atraso de loop

}

//============================================
// --- Funções ---

long moving_average (int sig, int sensorNum)
{
  int i; // controle do vetor
  long acc = 0; // acumulador

  for (i = num; i > 0 ; i--) values[i][sensorNum] = values [i - 1][sensorNum];

  values[0][sensorNum] = sig;

  for (i = 0; i < num; i++) acc += values[i][sensorNum];

  return acc / num;
}


//terminar
long moving_average2 (int sig, int sensorNum)
{

  accf[sensorNum] = accf[sensorNum] - values[cont_num[sensores_HC]][sensorNum] ; // Retira da soma
  accf[sensorNum] = accf[sensorNum] + sig; // soma o novo valor
  values[cont_num[sensores_HC]][sensorNum] = sig;  //Salva no vetor o novo valor
  
  cont_num[sensores_HC]++; //avança contador de posição do vetor
  if (cont_num[sensores_HC]  = num ) cont_num[sensores_HC] = 0; // Zera se estourou as posições

  return   accf[sensores_HC] / num; //retorna a nova media
}






// Sensor HC lê distancia
long sensorHC (int trigpin , int echopin)
{

  digitalWrite( trigpin, HIGH );
  delayMicroseconds( 100 );
  digitalWrite( trigpin, LOW );
  int interval = pulseIn( echopin, HIGH );

  return interval * 0.017; //cm
}

float mediaMovel(float xis) {
  static float media = 0.0;
  static int indice = 1;

  if (indice == 0 || indice == 33) {
    indice = 1;
    media = 0.0;
  }
  media = media + (xis - media) / indice++;

  return media;
}


void giroscopio( ) {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  //Solicita os dados do sensor
  Wire.requestFrom(MPU, 14, true);

  //Armazena o valor dos sensores nas variaveis correspondentes
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

void print_giroscopio( ) {
  //Mostra os valores na serial
  Serial.print("AcX = ");
  Serial.print(AcX);
  Serial.print(" | Y = ");
  Serial.print(AcY);
  Serial.print(" | Z = ");
  Serial.print(AcZ);
  Serial.print(" | Gir. X = ");
  Serial.print(GyX);
  Serial.print(" | Y = ");
  Serial.print(GyY);
  Serial.print(" | Z = ");
  Serial.print(GyZ);
  Serial.print(" | Temp = ");
  Serial.println(Tmp / 340.00 + 36.53);
}


void print_giroscopioSerialStudio( ) {
  Serial.print("/*");        // Frame start sequence  [/*]
  Serial.print(AcX);//AcX
  Serial.print(",");         // Separator character   [,]
  Serial.print(AcY);//| Y
  Serial.print(",");         // Separator character   [,]
  Serial.print(AcZ);//| Z
  Serial.print(",");         // Separator character   [,]
  Serial.print(GyX);//| Gir. X
  Serial.print(",");         // Separator character   [,]
  Serial.print(GyY);//| Gir.Y
  Serial.print(",");         // Separator character   [,]
  Serial.print(GyZ);//| Gir.Z
  Serial.print(",");         // Separator character   [,]
  Serial.print(Tmp / 340.00 + 36.53);//| Temp
  Serial.print("*/");        // Frame finish sequence [*/]
}


void print_HC( ) {
  //Mostra os valores na serial
  Serial.print("HC_1 = ");
  Serial.print(distance_filtered[0]);
  Serial.print(" | HC_2 = ");
  Serial.print(distance_filtered[1]);
  Serial.print(" | HC_3 = ");
  Serial.print(distance_filtered[2]);
  Serial.print(" | HC_4 = ");
  Serial.print(distance_filtered[3]);
  Serial.print(" | HC_5 = ");
  Serial.println(distance_filtered[4]);
}

void print_HC_SerialStudio( ) {
  Serial.print("/*");        // Frame start sequence  [/*]
  Serial.print(distance_filtered[0]);// HC_1
  Serial.print(",");
  Serial.print(distance_filtered[1]);// HC_2
  Serial.print(",");
  Serial.print(distance_filtered[2]);// HC_3
  Serial.print(",");
  Serial.print(distance_filtered[3]);// HC_4
  Serial.print(",");
  Serial.print(distance_filtered[4]);// HC_5
  Serial.print("*/");        // Frame finish sequence [*/]
}
