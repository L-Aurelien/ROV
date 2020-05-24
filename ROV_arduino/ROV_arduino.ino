/*
Pins:
 MS5540 sensor attached to pins 10 - 13:
 MOSI (DIN): pin 11
 MISO (DOUT): pin 12
 SCK: pin 13
 MCLK: pin 9 (or use external clock generator on 32kHz)
 CS is not in use, but might be pin 10
*/

// include library:
#include <SPI.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);
// generate a MCKL signal pin
const int clock = 9;

String moteur[3]={"0","0","0"};

int pin1[3]={7,2,4}; //pin de commande 1
int pin2[3]={1,3,5}; // pin de commande 2
int pinP[3]={10,5,6};// pin PWM moteur

char valeur;
char c;
int v;
int i=0;
int valeur2[3]={0,0,0};
int SER_Pin = 3;   //pin 14 on the 74HC595
int RCLK_Pin = 4;  //pin 12 on the 74HC595
int SRCLK_Pin = 7; //pin 11 on the 74HC595
 
//How many of the shift registers - change this
#define number_of_74hc595s 1 
 
//do not touch
#define numOfRegisterPins number_of_74hc595s * 8
 
boolean registers[numOfRegisterPins];

void resetsensor() //this function keeps the sketch a little shorter
{
  SPI.setDataMode(SPI_MODE0);
  SPI.transfer(0x15);
  SPI.transfer(0x55);
  SPI.transfer(0x40);
}

int result(int x)
{
  resetsensor();
  unsigned int result = 0;
  byte inbyte = 0;
  SPI.transfer(0x1D);
  SPI.transfer(x);
  SPI.setDataMode(SPI_MODE1);
  result = SPI.transfer(0x00);
  result = result <<8;
  inbyte = SPI.transfer(0x00);
  result = result | inbyte;
  return result;
}



void setup() {
  Serial.begin(9600);
  SPI.begin(); //see SPI library details on arduino.cc for details
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV32); //divide 16 MHz to communicate on 500 kHz
  pinMode(clock, OUTPUT);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  delay(100);
  pinMode(pinP[0],OUTPUT);
  pinMode(pinP[1],OUTPUT);
  pinMode(pinP[2],OUTPUT);
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
 
  //reset all register pins
  clearRegisters();
  writeRegisters();
}

//set all register pins to LOW
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
     registers[i] = LOW;
  }
} 
 
//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters(){
 
  digitalWrite(RCLK_Pin, LOW);
 
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
    digitalWrite(SRCLK_Pin, LOW);
 
    int val = registers[i];
 
    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);
 
  }
  digitalWrite(RCLK_Pin, HIGH);
 
}
 
//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value){
  registers[index] = value;
}

void loop()
{
  while (Serial.available()){
    c = Serial.read();
    if ((c>='0')&&(c<='9')){
      v=10*v+c-'0';
    }
    else if (c=='p'){
      valeur2[i]=v;
      i++;
      v=0;
    }
    else if (c=='n'){
      valeur2[i]=-v;
      i++;
      v=0;
    }
    else {
      i=0 ;
      mpu6050.update();
    }
  }
  actionMoteur(0);
  actionMoteur(1);
  actionMoteur(2);
  
  mpu6050.update();
  Serial.print(mpu6050.getAngleX());
  Serial.print(":");
  Serial.print(mpu6050.getAngleY());
  Serial.print(":");
  Serial.print(mpu6050.getAngleZ());
  Serial.print(":");
  Serial.print(Temp());
  Serial.print(":");
  Serial.println(Press());
}

int Temp()
{
  TCCR1B = (TCCR1B & 0xF8) | 1 ; //generates the MCKL signal
  analogWrite (clock, 128) ;
  int result1 = result(80);
  int result2 = result(96);
  
  long c5 = ((result1 & 0x0001) << 10) | ((result2 >> 6) & 0x03FF);
  long c6 = result2 & 0x003F;
  resetsensor(); //resets the sensor

  unsigned int tempMSB = 0; //first byte of value
  unsigned int tempLSB = 0; //last byte of value
  unsigned int D2 = 0;
  SPI.transfer(0x0F); //send first byte of command to get temperature value
  SPI.transfer(0x20); //send second byte of command to get temperature value
  delay(35); //wait for conversion end
  SPI.setDataMode(SPI_MODE1); //change mode in order to listen
  tempMSB = SPI.transfer(0x00); //send dummy byte to read first byte of value
  tempMSB = tempMSB << 8; //shift first byte
  tempLSB = SPI.transfer(0x00); //send dummy byte to read second byte of value
  D2 = tempMSB | tempLSB; //combine first and second byte of value

  //calculation of the real values by means of the calibration factors and the maths
  //in the datasheet. const MUST be long
  const long UT1 = (c5 << 3) + 20224;
  const long dT = D2 - UT1;
  
  const long TEMP = 200 + ((dT * (c6 + 50)) >> 10);  
  float TEMPREAL = TEMP/10;
  return TEMPREAL;
}

float Press()
{
  TCCR1B = (TCCR1B & 0xF8) | 1 ; //generates the MCKL signal
  analogWrite (clock, 128) ;
  int result1 = result(80);
  int result2 = result(96);
  int result3 = result(144);
  int result4 = result(160);
  
  long c1 = (result1 >> 1) & 0x7FFF;
  long c2 = ((result3 & 0x003F) << 6) | (result4 & 0x003F);
  long c3 = (result4 >> 6) & 0x03FF;
  long c4 = (result3 >> 6) & 0x03FF;
  long c5 = ((result1 & 0x0001) << 10) | ((result2 >> 6) & 0x03FF);
  resetsensor(); //resets the sensor

  unsigned int presMSB = 0; //first byte of value
  unsigned int presLSB = 0; //last byte of value
  unsigned int D1 = 0;
  SPI.transfer(0x0F); //send first byte of command to get pressure value
  SPI.transfer(0x40); //send second byte of command to get pressure value
  delay(35); //wait for conversion end
  SPI.setDataMode(SPI_MODE1); //change mode in order to listen
  presMSB = SPI.transfer(0x00); //send dummy byte to read first byte of value
  presMSB = presMSB << 8; //shift first byte
  presLSB = SPI.transfer(0x00); //send dummy byte to read second byte of value
  D1 = presMSB | presLSB; //combine first and second byte of value

  resetsensor(); //resets the sensor 
  
  unsigned int tempMSB = 0; //first byte of value
  unsigned int tempLSB = 0; //last byte of value
  unsigned int D2 = 0;
  SPI.transfer(0x0F); //send first byte of command to get temperature value
  SPI.transfer(0x20); //send second byte of command to get temperature value
  delay(35); //wait for conversion end
  SPI.setDataMode(SPI_MODE1); //change mode in order to listen
  tempMSB = SPI.transfer(0x00); //send dummy byte to read first byte of value
  tempMSB = tempMSB << 8; //shift first byte
  tempLSB = SPI.transfer(0x00); //send dummy byte to read second byte of value
  D2 = tempMSB | tempLSB; //combine first and second byte of value

  //calculation of the real values by means of the calibration factors and the maths
  //in the datasheet. const MUST be long
  const long UT1 = (c5 << 3) + 20224;
  const long dT = D2 - UT1;
  
  const long OFF  = (c2 * 4) + (((c4 - 512) * dT) >> 12);
  const long SENS = c1 + ((c3 * dT) >> 10) + 24576;
  const long X = (SENS * (D1 - 7168) >> 14) - OFF;
  long PCOMP = ((X * 10) >> 5) + 2500;
  return float(PCOMP)/10;
}

void actionMoteur(int moteur){
  int etat1,etat2,puissance=valeur2[moteur]; //variable de la fonction
  
  //test sens du moteur 1,-1 (sens contraire) ou tout autre valeur (stoppe le moteur)
  if (puissance>0){
    etat1=HIGH;
    etat2=LOW;
  }
  else if (puissance<0){
    etat1=LOW;
    etat2=HIGH;
  }
  else {
    
    etat1=LOW;
    etat2=LOW;
  }
  analogWrite(pinP[moteur],abs(puissance));
  setRegisterPin(pin1[moteur], etat1);
  setRegisterPin(pin2[moteur], etat2);
  writeRegisters();
}
