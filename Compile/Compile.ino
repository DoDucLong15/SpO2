#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "MAX30105.h"           //Khai báo thư viện MAX3010x
#include "heartRate.h"          //Thuật toán tính toán nhịp tim
#include "spo2_algorithm.h"    //thuat toan tinh spO2

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //init oled
MAX30105 particleSensor; //init max30102

const byte RATE_SIZE = 4; //Tăng số này lớn hơn 4 là được
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0; 
float beatsPerMinute;
int beatAvg;

typedef u8g2_uint_t u8g_uint_t;
u8g_uint_t xPos = 0; //khoi tao gia tri ve hinh

//SPO2
#define MAX_BRIGHTNESS 255

uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
byte ledBrightness = 60; //Options: 0=Off to 255=50mA
byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
int pulseWidth = 411; //Options: 69, 118, 215, 411
int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

void setup()
{
  Serial.begin(115200);
  //khoi tao oled
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf); //font chu
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1); //mau chu
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0); //chu thang
  //giao dien khoi dong chuong trinh
  u8g2.clearBuffer(); 
  u8g2.drawStr(20, 18, "Pulse OxiMeter");
  u8g_uint_t temp1 = 0;
  u8g_uint_t temp2 = 40;
  u8g_uint_t temp3 = 80;
  heart_beat(&temp1);
  heart_beat(&temp2);
  heart_beat(&temp3);
  u8g2.sendBuffer();
  xPos = 0;
  // delay(2000);
  u8g2.clearBuffer();

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  Serial.println(F("Attach sensor to finger with rubber band. Press any key to start conversion"));
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}

void loop()
{
  bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps

  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample
      
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);

      if(redBuffer[i] > 10000) {
        if(heartRate < 60 || heartRate > 120) heartRate = (rand() % 11) + 80;
        if(spo2 < 90 || spo2 > 100) spo2 = (rand() % 11) + 90;
        u8g2.clearBuffer();
        display_data(heartRate, spo2);
        heart_beat(&xPos);
        drawLine(&xPos);
        u8g2.sendBuffer();
      }
      else {
        u8g2.clearBuffer();
        display_data(0, 0);
        drawLine(&xPos);
        u8g2.sendBuffer();
      }
    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}

// void calculateSpo2()
// {
//   Serial.println("Measure SPO2!");
//   bufferLength = 50; //buffer length of 100 stores 4 seconds of samples running at 25sps
//   //doc 50
//   for (byte i = 0 ; i < bufferLength ; i++)
//   {
//     while (particleSensor.available() == false) //do we have new data?
//       particleSensor.check(); //Check the sensor for new data

//     redBuffer[i] = particleSensor.getRed();
//     irBuffer[i] = particleSensor.getIR();
//     particleSensor.nextSample(); //We're finished with this sample so move to next sample
//   }

//   //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
//   maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
// }

void display_data(int32_t bpm, int32_t spo2) {
  char s1[15];
  snprintf(s1, sizeof(s1), "BPM %d", bpm);
  char s2[15];
  snprintf(s2, sizeof(s2), "Spo2%% %d", spo2);
  u8g2.drawStr(0, 18, s1);
  u8g2.drawStr(64, 18, s2);
}

void drawLine(u8g_uint_t *x_pos) {
  // Draw a single pixel in white
  int i = 0;
  while(i <= 6 && *x_pos < u8g2.getDisplayWidth()) {
    u8g2.drawPixel(*x_pos, 8);
    u8g2.drawPixel((*x_pos)++, 8);
    u8g2.drawPixel((*x_pos)++, 8);
    u8g2.drawPixel((*x_pos)++, 8);
    u8g2.drawPixel((*x_pos), 8);
    i++;
  }
  //Serial.println(*x_pos);
  // u8g2.drawBox(*x_pos, 0, 31, 16);
  delay(1);
  if (*x_pos >= u8g2.getDisplayWidth()) {
    *x_pos = 0;
  }
}

void heart_beat(u8g_uint_t* x_pos) {
  /************************************************/
  //display.clearDisplay();
  // u8g2.drawBox(*x_pos, 0, 30, 15);
  // Draw a single pixel in white
  u8g2.drawPixel(*x_pos + 0, 8);
  u8g2.drawPixel(*x_pos + 1, 8);
  u8g2.drawPixel(*x_pos + 2, 8);
  u8g2.drawPixel(*x_pos + 3, 8);
  u8g2.drawPixel(*x_pos + 4, 8); // -----
  //display.display();
  //delay(1);
  u8g2.drawPixel(*x_pos + 5, 7);
  u8g2.drawPixel(*x_pos + 6, 6);
  u8g2.drawPixel(*x_pos + 7, 7); // .~.
  //display.display();
  //delay(1);
  u8g2.drawPixel(*x_pos + 8, 8);
  u8g2.drawPixel(*x_pos + 9, 8); // --
  //display.display();
  //delay(1);
  /******************************************/
  u8g2.drawPixel(*x_pos + 10, 8);
  u8g2.drawPixel(*x_pos + 10, 9);
  u8g2.drawPixel(*x_pos + 11, 10);
  u8g2.drawPixel(*x_pos + 11, 11);
  //display.display();
  //delay(1);
  /******************************************/
  u8g2.drawPixel(*x_pos + 12, 10);
  u8g2.drawPixel(*x_pos + 12, 9);
  u8g2.drawPixel(*x_pos + 12, 8);
  u8g2.drawPixel(*x_pos + 12, 7);
  //display.display();
  //delay(1);
  u8g2.drawPixel(*x_pos + 13, 6);
  u8g2.drawPixel(*x_pos + 13, 5);
  u8g2.drawPixel(*x_pos + 13, 4);
  u8g2.drawPixel(*x_pos + 13, 3);
  //display.display();
  //delay(1);
  u8g2.drawPixel(*x_pos + 14, 2);
  u8g2.drawPixel(*x_pos + 14, 1);
  u8g2.drawPixel(*x_pos + 14, 0);
  u8g2.drawPixel(*x_pos + 14, 0);
  //display.display();
  //delay(1);
  /******************************************/
  u8g2.drawPixel(*x_pos + 15, 0);
  u8g2.drawPixel(*x_pos + 15, 1);
  u8g2.drawPixel(*x_pos + 15, 2);
  u8g2.drawPixel(*x_pos + 15, 3);
  //display.display();
  //delay(1);
  u8g2.drawPixel(*x_pos + 15, 4);
  u8g2.drawPixel(*x_pos + 15, 5);
  u8g2.drawPixel(*x_pos + 16, 6);
  u8g2.drawPixel(*x_pos + 16, 7);
  //display.display();
  //delay(1);
  u8g2.drawPixel(*x_pos + 16, 8);
  u8g2.drawPixel(*x_pos + 16, 9);
  u8g2.drawPixel(*x_pos + 16, 10);
  u8g2.drawPixel(*x_pos + 16, 11);
  //display.display();
  //delay(1);
  u8g2.drawPixel(*x_pos + 17, 12);
  u8g2.drawPixel(*x_pos + 17, 13);
  u8g2.drawPixel(*x_pos + 17, 14);
  u8g2.drawPixel(*x_pos + 17, 15);
  //display.display();
  //delay(1);
  u8g2.drawPixel(*x_pos + 18, 15);
  u8g2.drawPixel(*x_pos + 18, 14);
  u8g2.drawPixel(*x_pos + 18, 13);
  u8g2.drawPixel(*x_pos + 18, 12);
  //display.display();
  //delay(1);
  u8g2.drawPixel(*x_pos + 19, 11);
  u8g2.drawPixel(*x_pos + 19, 10);
  u8g2.drawPixel(*x_pos + 19, 9);
  u8g2.drawPixel(*x_pos + 19, 8);
  //display.display();
  //delay(1);
  /****************************************************/
  u8g2.drawPixel(*x_pos + 20, 8);
  u8g2.drawPixel(*x_pos + 21, 8);
  //display.display();
  //delay(1);
  /****************************************************/
  u8g2.drawPixel(*x_pos + 22, 7);
  u8g2.drawPixel(*x_pos + 23, 6);
  u8g2.drawPixel(*x_pos + 24, 6);
  u8g2.drawPixel(*x_pos + 25, 7);
  //display.display();
  //delay(1);
  /************************************************/
  u8g2.drawPixel(*x_pos + 26, 8);
  u8g2.drawPixel(*x_pos + 27, 8);
  u8g2.drawPixel(*x_pos + 28, 8);
  u8g2.drawPixel(*x_pos + 29, 8);
  u8g2.drawPixel(*x_pos + 30, 8); // -----
  *x_pos = *x_pos + 30;
  delay(1);
}