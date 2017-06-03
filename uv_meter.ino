#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"

#define LOOP_DELAY 1000

//5110 LCD pins:
/*
 * 1 RST
 * 2 CS chip select
 * 3 D/C
 * 4 DIN
 * 5 CLK
 * 6 VCC
 * 7 BL
 * 8 GND
 */


// Software SPI (slower updates, more flexible pin options):
#define LCD_SCLK 2 //Serial clock out (SCLK)
#define LCD_DIN 3 //Serial data out (DIN)
#define LCD_DC 4 //Data/Command select (D/C)
#define LCD_CS 5 //LCD chip select (CS)
#define LCD_RST 6 //LCD reset (RST)

//GUVA-S12SD
#define UV_SENSOR 6

Adafruit_PCD8544 lcd = Adafruit_PCD8544(LCD_SCLK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

int sht31_connected = 0;

void setup() {
//  Serial.begin(9600);

  //EREF connected to 3.3V
  analogReference(EXTERNAL);

  lcd.begin();


  // you can change the contrast around to adapt the display
  // for the best viewing!
  lcd.setContrast(50);

  displayWelcomeScreen();
  delay(50);

  if (sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    sht31_connected = 1;
  } else {
    sht31_connected = 0;
    displayShowError("SHT31 not found");
    delay(2000);
  }
}

void loop() {
  float t = 0;
  int h = 0;
  if (sht31_connected){
    t = sht31.readTemperature();
    h = sht31.readHumidity();
  }

  float uv_sensor_output = readUVSensor();
  int uv_index = convertSensorOutputToUVIndex(uv_sensor_output);
  
  displayData(t, h, uv_sensor_output, uv_index);
  delay(LOOP_DELAY);
}

/**
 * returns voltage on the output pin of GUVA biard
 */
float readUVSensor(){
  int adc = analogRead(UV_SENSOR);

  //convert to voltage. Reference voltage is 3.3V. ADC resolution is 1024
  return adc * 3.3 / 1024;
}

/**
 * hope this works http://www.sunrom.com/p/ultravoilet-uv-light-radiation-sensor
 */
float uvA = (11 - 1) / (1.17 - 0.27);
float uvB = 11 - uvA * 1.17;
int convertSensorOutputToUVIndex(float voltage){
  if (voltage < 0.27){
    return 0;
  }

  // 0.25V = 1 UV Index, 1.17V = 11 UV Index
  // y = Ax + B
  // A = (11 - 1) / (1.17 - 0.25) = 10,8695652, 
  // B = 1 - A * 0.25 = -1,71739

  return uvA * voltage + uvB + 0.5; //0.5 for the correct rounding
}

void displayWelcomeScreen(){
  lcd.clearDisplay();
  lcd.setTextColor(BLACK);
  lcd.setTextSize(1);
  lcd.setCursor(0, 0);
  lcd.print("UV Meter");
  lcd.setCursor(40, 30);
  lcd.println("by Otzy");
  lcd.display();
}

void displayShowError(char *message){
  lcd.clearDisplay();
  lcd.setCursor(0, 15);
  lcd.println(message);
  lcd.display();
}

void displayData(float temperature, int humidity, float uv_sensor_voltage, int uv_index){
  lcd.clearDisplay();
  lcd.setCursor(0,0);
  lcd.setTextSize(1.8);
  lcd.print(temperature);
  lcd.println("  C");
  lcd.print("Hum ");
  lcd.print(humidity);
  lcd.println("%");
  
  lcd.print("UV raw ");
  lcd.print(uv_sensor_voltage);
  lcd.println("V");
  lcd.print("UV Index ");
  lcd.print(uv_index);

  //grad sign for temperature
  lcd.setTextSize(0.25);
  lcd.setCursor(35,-2);
  lcd.print('o');
  
  lcd.display();  
}


