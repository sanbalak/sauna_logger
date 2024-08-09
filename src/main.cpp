// чтение, асинхронный вариант

#include <GyverHTU21D.h>
#include <GyverOLED.h>
#include <Wire.h>
#include <SPI.h>
#include <GyverMAX6675_SPI.h>
#include <GyverDS18.h>
#include <cmath>  // Для использования функции round()

GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;
//GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
GyverHTU21D htu;
GyverMAX6675_SPI<5> sens; //CS pin
GyverDS18Single ds(4); 

float TmpHTU = 0;
float HumHTU = 0;

int TmpDS18 = 0;
int TmpMAX = 0;
float dewPoint;

const static uint8_t icons_8x8[][8] PROGMEM = {
    {0x06, 0x09, 0x09, 0x06, 0x78, 0x84, 0x84, 0x48}, //0 celsius
    {0x06, 0x09, 0x09, 0x06, 0x08, 0xfc, 0x88, 0xc0}, //1 temperature
  {0x7e, 0x81, 0x91, 0xa1, 0x91, 0x89, 0x84, 0x72}, //2 checked
  {0x7e, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7e}, //3 unchecked
  {0x1c, 0x3e, 0x73, 0xe1, 0xe1, 0x73, 0x3e, 0x1c}, //4 map
};

void setup() {
  Serial.begin(9600);
  htu.setResolution(HTU21D_RES_HIGH);
  ds.requestTemp();  // первый запрос на измерение
  oled.init();   
  oled.clear();
  oled.setScale(1);
    oled.textMode(BUF_ADD);
  // BUF_ADD - наложить текст
  // BUF_SUBTRACT - вычесть текст
  // BUF_REPLACE - заменить (весь прямоугольник буквы)

}

void drawIcon8x8(byte index) {
  size_t s = sizeof icons_8x8[index];//можна так, а можна просто 8 
  for(unsigned int i = 0; i < s; i++) {
    oled.drawByte(pgm_read_byte(&(icons_8x8[index][i])));
  }
}

// Функция округления до десятых и форматирования без лишних нулей
void roundToTenth(float value, char* output) {
    float roundedValue = round(value * 10) / 10;
    dtostrf(roundedValue, 3, 1, output);  // Форматирование с 1 знаком после запятой
}
float calculateDewPoint(float temperature, float humidity) {
    const float a = 17.27;
    const float b = 237.7; 
    float gamma = (a * temperature) / (b + temperature) + log(humidity / 100.0);
    float dewPoint = (b * gamma) / (a - gamma);
    return dewPoint;
}

void readData(){
  if (htu.readTick()) {
    TmpHTU = htu.getTemperatureWait();
    HumHTU = htu.getHumidityWait();
    dewPoint = calculateDewPoint(TmpHTU, HumHTU);
    }
  delay(50);     
  if (sens.readTemp()) {     
      TmpMAX = sens.getTempInt();       // Читаем температуру
    } else Serial.println("Error max");   // ошибка чтения или подключения - выводим лог
  delay(50);     
  static uint32_t tmr;
  // время таймера НЕ МЕНЬШЕ чем текущее getConversionTime()
  if (millis() - tmr >= ds.getConversionTime()) {
      tmr = millis();
      // чтение
      if (ds.readTemp()) {
          TmpDS18 = ds.getTempInt();
      } else {
          Serial.println("read 2s18 error");
      }
      // запрос
      if (!ds.requestTemp()) {
          Serial.println("request ds18 error");
      }
  }
}
void printData(){
  oled.clear();

  oled.roundRect(0, 5, 45, 35, OLED_STROKE);
  oled.line(3, 5, 33, 5, 0);// remove line

  oled.setCursorXY(4, 0);
  oled.print("Полок");
  oled.setCursorXY(4, 10);
  oled.print(TmpHTU);
  drawIcon8x8(0);//*C
  oled.setCursorXY(4, 24);
  oled.print(HumHTU);
  oled.print("%");

  oled.roundRect(0, 45, 45, 60, OLED_STROKE);
  oled.line(3, 45, 20, 45, 0);// remove line

  oled.setCursorXY(4, 40);
  oled.print("Ттр");
  oled.setCursorXY(4, 50);
  oled.print(dewPoint);
  drawIcon8x8(0);//*C

  oled.roundRect(63, 0, 100, 24, OLED_STROKE);
  oled.line(66, 0, 90, 66, 0);// remove line
  oled.setCursorXY(67, 0);
  oled.print("Потолок");
  oled.setCursorXY(67, 12);
  oled.print(TmpDS18);
  drawIcon8x8(0);//*C

  oled.setCursorXY(67, 24);
  oled.print("Печь");
  oled.setCursorXY(67, 36);
  oled.print(TmpMAX);
  drawIcon8x8(0);//*C

  oled.update();
}
void loop() {
readData();
printData();

delay(100);
}

