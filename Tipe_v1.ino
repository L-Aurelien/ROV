/*
 MS5540C Miniature Barometer Module
 This program will read your MS5440C or compatible pressure sensor every 5 seconds and show you the calibration words, the calibration factors,
 the raw values and the compensated values of temperature and pressure.
 Once you read out the calibration factors you can define them in the header of any sketch you write for the sensor.
 
Pins:
 MS5540 sensor attached to pins 10 - 13:
 MOSI (DIN): pin 11
 MISO (DOUT): pin 12
 SCK: pin 13
 MCLK: pin 9 (or use external clock generator on 32kHz)
 CS is not in use, but might be pin 10
 
 created 29 February 2012
 by MiGeRA
*/

// include library:
#include <SPI.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);
// generate a MCKL signal pin
const int clock = 9;


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
}

void loop()
{
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
