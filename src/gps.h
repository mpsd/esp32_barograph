#ifndef GPS_H
#define GPS_H

#include "tools.h"

#include <TinyGPS++.h>

#define DEBUG_PRINT_GPS Serial.printf("%02u/%02u/%04u %02u:%02u:%02u - Sat: %02u, Lat: %08.6f, Lon: %08.6f, HDOP: %04.2f, Alt: %4.0f, Course: %3.0f, Speed: %2.0f, GMEpoch: %llu (%s)\n", \
    gps_getDayOfMonth(),\
    gps_getMonth(),     \
    gps_getYear(),      \
    gps_getHour(),      \
    gps_getMinute(),    \
    gps_getSecond(),    \
    gps_getSatellites(),\
    gps_getLat(),       \
    gps_getLon(),       \
    gps_getHDOP(),      \
    gps_getAltitude(),  \
    gps_getCourse(),    \
    gps_getSpeed(),     \
    gps_getEpoch(),     \
    (gps_DateTimeIsValid() ?  "valid" : "false" ) );   

void gps_initialize();
void gps_close();
void gps_delay(unsigned long ms);

bool gps_LocationIsValid();
bool gps_DateTimeIsValid();

uint32_t gps_getSatellites();
float_t gps_getHDOP();

float_t gps_getLat();
float_t gps_getLon();
float_t gps_getAltitude();  // m
float_t gps_getCourse();    // deg
float_t gps_getSpeed();     // m/s

uint16_t gps_getYear();
uint8_t gps_getMonth();
uint8_t gps_getDayOfMonth();
uint8_t gps_getHour();
uint8_t gps_getMinute();
uint8_t gps_getSecond();

uint64_t gps_getEpoch();

char * gps_DecimalToDegreeMinutes(float_t decimal);

/* private functions */
struct tm gps_getGMNow();
struct tm gps_getLocalNow();

#endif