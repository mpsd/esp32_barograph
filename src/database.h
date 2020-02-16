#ifndef DATABASE_H
#define DATABASE_H

#include "tools.h"

// builtin libraries
#include <SPI.h>
#include <FS.h>
#include <SD.h>

void config_get(void);
void config_set(void);

void datastore_fetch(uint64_t tstnow);
void datastore_push(float_t lat, float_t lon, float_t alt_m, float_t crs, float_t spd, uint32_t sat, float_t hdop, float_t temp_raw, float_t temp, 
  float_t temp_offset, float_t hum_raw, float_t hum, float_t press_raw, float_t press, float_t alt, uint64_t tst);

struct datasetstruct {
    float_t lat;
    float_t lon;
    float_t altitude_m;
    float_t course;
    float_t speed;
    uint32_t sat;
    float_t hdop;
    float_t temperature_raw;
    float_t temperature;
    float_t temperature_offset;
    float_t humidity_raw;
    float_t humidity;
    float_t pressure_raw;
    float_t pressure;
    float_t altitude;
    uint64_t timestamp;
};

#endif