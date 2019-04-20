#ifndef GPS_H
#define GPS_H

#include "tools.h"

#include <TinyGPS++.h>

#define DEBUG_PRINT_GPS Serial.printf("%02u/%02u/%04u %02u:%02u:%02u - Sat: %02u, Lat: %03.2f, Lon: %03.2f, Alt: %4.0f, Course: %3.0f, Speed: %3.0f", \
    gps_getDayOfMonth(),\
    gps_getMonth(),     \
    gps_getYear(),      \
    gps_getHour(),      \
    gps_getMinute(),    \
    gps_getSecond(),    \
    gps_getSatellites(),\
    gps_getLat(),       \
    gps_getLon(),       \
    gps_getAltitude(),  \
    gps_getCourse(),    \
    gps_getSpeed() );   

void gps_initialize();
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

#endif