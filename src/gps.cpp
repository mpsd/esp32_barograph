#include <Arduino.h>
#include "gps.h"

TinyGPSPlus gps;
HardwareSerial _UARTGPS(1);

void gps_initialize() {
    _UARTGPS.begin( CONFIG.GPSBaud, SERIAL_8N1, _gps_rx, _gps_tx);
    while ( ! _UARTGPS.available() ) {
        DEBUG_PRINT("GPS not found");
        delay(1000);
    }
}

void gps_delay( unsigned long ms ) {
    unsigned long start = millis();
    do 
    {
        while ( _UARTGPS.available() ) {
            gps.encode( _UARTGPS.read() );
        }
    } while (millis() - start < ms);
 
    DEBUG_PRINT_GPS;
}

bool gps_LocationIsValid() {
    return gps.location.isValid();
}

bool gps_DateTimeIsValid() {
    return ( gps.date.isValid() && gps.time.isValid() );
}

uint32_t gps_getSatellites() {
    if ( gps.satellites.isValid() ) {
        return gps.satellites.value();
    } else {
        return 0U;
    }
}

float_t gps_getHDOP() {
    if ( gps.hdop.isValid() ) {
        return gps.hdop.hdop();
    } else {
        return 0.0F;
    }
}

float_t gps_getLat() {
    if ( gps.location.isValid() ) {
        return gps.location.lat();
    } else {
        return 0.0F;
    }
}

float_t gps_getLon() {
    if ( gps.location.isValid() ) {
        return gps.location.lng();
    } else {
        return 0.0F;
    }
}

float_t gps_getAltitude() {
    if ( gps.altitude.isValid() ) {
        return gps.altitude.meters();
    } else {
        return 0.0F;
    }
}

float_t gps_getCourse() {
    if ( gps.course.isValid() ) {
        return gps.course.deg();
    } else {
        return 0.0F;
    }
}

float_t gps_getSpeed() {
    if ( gps.speed.isValid() ) {
        return gps.speed.mps();
    } else {
        return 0.0F;
    }
}

uint16_t gps_getYear() {
    return gps.date.year();
}

uint8_t gps_getMonth() {
    return gps.date.month();
}

uint8_t gps_getDayOfMonth() {
    return gps.date.day();
}

uint8_t gps_getHour() {
    return gps.time.hour();
}

uint8_t gps_getMinute() {
    return gps.time.minute();
}

uint8_t gps_getSecond() {
    return gps.time.second();
}