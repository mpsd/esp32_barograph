#ifndef GPS_H
#define GPS_H

#include "tools.h"

#include <TinyGPS++.h>

#define DEBUG_PRINT_GPS Serial.printf("%02u/%02u/%04u %02u:%02u:%02u - Sat: %02u, Lat: %08.6f, Lon: %08.6f, Alt: %4.0f, Course: %3.0f, Speed: %2.0f, GMEpoch: %llu\n", \
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
    gps_getSpeed(),     \
    gps_getEpoch() );   

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

uint64_t gps_getEpoch();

/* private functions */
tm * gps_getGMNow();
tm * gps_getLocalNow();

#endif