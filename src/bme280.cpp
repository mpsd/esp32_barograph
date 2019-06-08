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

float_t bme280_getTemperatureRaw() {
  return bme.readTemperature();
}

float_t bme280_getTemperature() {
  return (bme.readTemperature() + CONFIG.TemperatureOffset);
}

float_t bme280_getTemperatureOffset() {
  return CONFIG.TemperatureOffset;
}

float_t bme280_getHumidityRaw() {
  return bme.readHumidity();
}

float_t bme280_getHumidity() {
  // https://de.wikipedia.org/wiki/S%C3%A4ttigung_(Physik)
  // Absolute Saettigung = (0.0316*Temp^2 - 0.2214*Temp + 8.8679) für 10 <= Temp <= 40
  return (0.0316F*pow(bme.readTemperature(), 2)-0.2214F*bme.readTemperature()+8.8679F)*bme.readHumidity()/(0.0316F*pow(bme280_getTemperature(), 2)-0.2214F*bme280_getTemperature()+8.8679F);
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

float_t bme280_getDewPoint() {
  return bme280_getDewPoint( bme.readHumidity(), bme.readTemperature() );
}

float_t bme280_getDewPoint(float_t hum, float_t temp) {
  // https://www.ajdesigner.com/phphumidity/dewpoint_equation_dewpoint_temperature.php
  return ( pow((hum/100.0F), 0.125F) * (112.0F+0.9F*temp) + 0.1F*temp - 112.0F );

  // https://www.schweizer-fn.de/lueftung/feuchte/feuchte.php#taupunkt
  // return (pow(hum/100.0F, 1/8.02F) * (109.8F + temp) -109.8F)

  // https://www.chemie.de/lexikon/Taupunkt.html#Berechnung_von_Taupunkt_und_Frostpunkt
  // return (241.2F * log(hum/100.0F) + (4222.03716F * temp)/(241.2F + temp))/(17.5043F - log(hum/100.0F) - (17.5043F*temp)/(241.2F+temp));
}