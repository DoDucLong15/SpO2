#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "MAX30102_PulseOximeter.h"
#define REPORTING_PERIOD_MS 500

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

typedef u8g2_uint_t u8g_uint_t;
u8g_uint_t xPos = 0;
u8g_uint_t x;
u8g_uint_t y;
int bpm = 0;
int spo2 = 0;
//khoi tao max30102
PulseOximeter pox;
uint32_t tsLastReport = 0;
// Callback (registered below) fired when a pulse is detected 
void onBeatDetected()
{
  Serial.println("Beat!");
}

void setup()
{
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);

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
  delay(2000);

  // Initialize the PulseOximeter instance
  if (!pox.begin()) {
    Serial.println("FAILED");
    for(;;);
  } else {
    Serial.println("SUCCESS");
  }
  // The default current for the IR LED is 50mA and is changed below
  pox.setIRLedCurrent(MAX30102_LED_CURR_7_6MA);
  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
  u8g2.clearBuffer();
  drawLine(&xPos);
  display_data(0,0);
  u8g2.sendBuffer();
}

void loop()
{
  // Make sure to call update as fast as possible
  pox.update();
  int bpm = 0;
  int spo2 = 0;
    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
  bpm = pox.getHeartRate();
  spo2 = pox.getSpO2();
  u8g2.clearBuffer();
  display_data(bpm, spo2);
  if(bpm > 0) {
    drawLine(&xPos);
    heart_beat(&xPos);
  }
  else {
    drawLine(&xPos);
  }
  u8g2.sendBuffer();
  delay(100);
}

void display_data(int bpm, int spo2) {
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