#ifndef BME280_H
#define BME280_H

#include "tools.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

void bme280_initialize();
float_t bme280_getTemperatureRaw();
float_t bme280_getTemperature();
float_t bme280_getTemperatureOffset();
float_t bme280_getHumidityRaw();
float_t bme280_getHumidity();
float_t bme280_getPressureRaw();
float_t bme280_getPressure();
float_t bme280_getAltitude();

float_t bme280_getDewPoint();
float_t bme280_getDewPoint(float_t hum, float_t temp);

#endif