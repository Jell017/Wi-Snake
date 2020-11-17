#include <string.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <Ticker.h>
// Solo se usa Wire.h si se usa I2Cdev I2CDEV_ARDUINO_WIRE en I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MPU6050 mpu;

Ticker mpuInt; //Interrupciones del mpu

#define INTERRUPT_PIN 2
//#define led 13 //Led para mostrar actividad
bool blinkState = false;

bool dmpReady = false;
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
//VectorInt16 aa;         // [x, y, z]            accel sensor measurements
//VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
//VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
//float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector


volatile bool mpuInterrupt = false;
void dmpDataReady() {
  mpuInterrupt = true;
}


void setup() {
  Wire.begin(0,2);
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  Wire.beginTransmission(0x68); // 0x68 is the i2c address of MPU6050
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.begin(115200);
  mpu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);


  // carga y configura el mpu
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  //mpu.setXGyroOffset(220);
  //mpu.setYGyroOffset(76);
  //mpu.setZGyroOffset(-85);

  // Si funciona devuelve 0
  if (devStatus == 0) {
    // Calibración y config
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();
    mpu.setRate(7);
    mpu.setDLPFMode(6);
    mpu.setDMPEnabled(true);

    ///*ICACHE_RAM_ATTR*/ attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    //mpuIntStatus = mpu.getIntStatus(); USAR TICKER

    mpuInt.attach_ms (100, FuncionMPU);

    dmpReady = true;

    // consigue el package para comparar después
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    //Serial.print(F("DMP Initialization failed (code "));
    //Serial.print(devStatus);
    //Serial.println(F(")"));
  }

}

void loop() {
  delay(100);
}

void FuncionMPU () {
  if (!dmpReady) return;

  
  // wait for MPU interrupt or extra packet(s) available
  /*while (!mpuInterrupt && fifoCount < packetSize) {
    if (mpuInterrupt && fifoCount < packetSize) {
      // try to get out of the infinite loop
      fifoCount = mpu.getFIFOCount();
    }
  }

  // reset interrupt flag and get INT_STATUS byte
  //mpuInterrupt = false;
  //mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();
  if (fifoCount < packetSize) {
    //Lets go back and wait for another interrupt. We shouldn't be here, we got an interrupt from another event
    // This is blocking so don't do it   while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
  }
  // check for overflow (this should never happen unless our code is too inefficient)
  else if ((mpuIntStatus & (0x01 << MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || fifoCount >= 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    //  fifoCount = mpu.getFIFOCount();  // will be zero after reset no need to ask
    Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & (0x01 << MPU6050_INTERRUPT_DMP_INT_BIT)) {

    // read a packet from FIFO
    while (fifoCount >= packetSize) { // Lets catch up to NOW, someone is using the dreaded delay()!
      mpu.getFIFOBytes(fifoBuffer, packetSize);
      // track FIFO count here in case there is > 1 packet available
      // (this lets us immediately read more without waiting for an interrupt)
      fifoCount -= packetSize;
    }*/

   Wire.beginTransmission(0x68);
   Wire.write(0x43); //La dirección del Gyro
   Wire.endTransmission(false);
   Wire.requestFrom(0x68,4,true); //Sólo le pasamos 4 bits

    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    //Solo necesitamos el roll
    //Serial.print("ypr\t");
    //Serial.print(ypr[0] * 180 / M_PI);
    //Serial.print("\t");
    Serial.println(ypr[1] * 180 / M_PI);
    Serial.print("\t");
    Serial.println(ypr[2] * 180 / M_PI);
    
    // blink LED to indicate activity
    //blinkState = !blinkState;
    //digitalWrite(led, blinkState);
 }
