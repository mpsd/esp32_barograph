#include <Arduino.h>
#include "bme280.h"

Adafruit_BME280 bme; // I2C

void bme280_initialize() {
  
  DEBUG_PRINT("initialize BME280");
 
  while ( ! bme.begin(0x76) ) {
    DEBUG_PRINT("BME280 not found");
    delay(1000);
  }

  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X4,   // temperature
                  Adafruit_BME280::SAMPLING_X4,   // pressure
                  Adafruit_BME280::SAMPLING_X4,   // humidity
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_1000 );

}

float_t bme280_getTemperature() {
    return bme.readTemperature();
}

float_t bme280_getHumidity() {
    return  bme.readHumidity();
}

float_t bme280_getPressureRaw() {
    return (bme.readPressure()/100.0F);
}

float_t bme280_getPressure() {
    // https://de.wikipedia.org/wiki/Barometrische_H%C3%B6henformel#Reduktion_auf_Meeresh%C3%B6he
    return (bme.readPressure()/100.0F)*pow(((bme.readTemperature() + 273.15F)/(bme.readTemperature() + 273.15F + 0.0065F * CONFIG.Altitude)),-5.255);
}

float_t bme280_getAltitude() {
  return CONFIG.Altitude;
}