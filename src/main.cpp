// чтение, асинхронный вариант

#include <GyverHTU21D.h>
#include <GyverOLED.h>
#include <Wire.h>
#include <SPI.h>
#include <GyverMAX6675_SPI.h>
#include <GyverDS18.h>

GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;
//GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
GyverHTU21D htu;
GyverMAX6675_SPI<5> sens; //CS pin
GyverDS18Single ds(4); 

float TmpHTU = 0;
float HumHTU = 0;

int TmpDS18 = 0;
int TmpMAX = 0;

void setup() {
  pinMode(4, 1);
  Serial.begin(9600);
  htu.setResolution(HTU21D_RES_HIGH);
    // первый запрос на измерение. Запрос ВСЕМ датчикам на линии
  ds.requestTemp();  // первый запрос на измерение
  oled.init();   
  oled.clear();
  oled.setScale(1);

}
float calculateDewPoint(float temperature, float humidity) {
    const float a = 17.27;
    const float b = 237.7;
    
    float gamma = (a * temperature) / (b + temperature) + log(humidity / 100.0);
    float dewPoint = (b * gamma) / (a - gamma);
    
    return dewPoint;
}
void loop() {
  if (htu.readTick()) {
  TmpHTU = htu.getTemperatureWait();
  HumHTU = htu.getHumidityWait();
  float dewPoint = calculateDewPoint(TmpHTU, HumHTU);

  }
 // delay(1000);     
if (sens.readTemp()) {     
    TmpMAX = sens.getTempInt();       // Читаем температуру
    Serial.print("Temp MAX: ");         // Если чтение прошло успешно - выводим в Serial
    Serial.print(TmpMAX);   // Забираем температуру через getTemp
    Serial.println(" *C");
  } else Serial.println("Error max");   // ошибка чтения или подключения - выводим лог

 // delay(1000);                      // Немного подождем


    static uint32_t tmr;
    // время таймера НЕ МЕНЬШЕ чем текущее getConversionTime()
    if (millis() - tmr >= ds.getConversionTime()) {
        tmr = millis();

        // чтение
        if (ds.readTemp()) {
            TmpDS18 = ds.getTempInt();
            Serial.print("temp ds18: ");
            Serial.println(TmpDS18);
        } else {
            Serial.println("read 2s18 error");
        }

        // запрос
        if (!ds.requestTemp()) {
            Serial.println("request ds18 error");
        }
    }
  Serial.println("HTU");
  Serial.print(TmpHTU);
  Serial.println(" *C");

  Serial.print(HumHTU);
  Serial.println(" %");

  oled.clear();
  oled.setCursorXY(0, 0);
  oled.print("HTU");
  oled.setCursorXY(0, 12);
  oled.print(TmpHTU);
  oled.print(" *C");
  oled.setCursorXY(0, 24);
  oled.print(HumHTU);
  oled.print(" %");

  oled.setCursorXY(64, 0);
  oled.print("MAX");
  oled.setCursorXY(64, 12);
  oled.print(TmpMAX);
  oled.print(" *C");
  oled.setCursorXY(64, 24);
  oled.print("DS18");
  oled.setCursorXY(64, 36);
  oled.print(TmpDS18);
  oled.print(" *C");
  oled.setCursorXY(64, 48);
  oled.print("Power ");
  oled.print(dewPoint);
  oled.update();

  delay(100);



}

