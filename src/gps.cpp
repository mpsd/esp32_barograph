#include <Arduino.h>
#include "gps.h"

TinyGPSPlus gps;
HardwareSerial _UARTGPS(1); // 1 is used internally, however pins are set in ...begin()

void gps_initialize() {
    _UARTGPS.begin( CONFIG.GPSBaud, SERIAL_8N1, _gps_rx, _gps_tx);
    while ( ! _UARTGPS.available() ) {
        DEBUG_PRINT("GPS not found");
        delay(1000);
    }
    gps_delay(1000);
    DEBUG_PRINT("GPS initialized");
}

void gps_close() {
    DEBUG_PRINT("close GPS");
    _UARTGPS.flush();
    _UARTGPS.end();
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
    return ( gps.location.isValid() && (gps.location.age() < 2000) );
}

bool gps_DateTimeIsValid() {
    return ( gps.date.isValid() && gps.time.isValid() && (gps.date.age() < 2000) && (gps.time.age() < 2000) );
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
    return (uint16_t)gps_getLocalNow()->tm_year + 1900; // epoch style correction
}

uint8_t gps_getMonth() {
    return (uint8_t)gps_getLocalNow()->tm_mon + 1;      // tm_mon starts with 0
}

uint8_t gps_getDayOfMonth() {
    return (uint8_t)gps_getLocalNow()->tm_mday;
}

uint8_t gps_getHour() {
    return (uint8_t)gps_getLocalNow()->tm_hour;
}

uint8_t gps_getMinute() {
    return (uint8_t)gps_getLocalNow()->tm_min;
}

uint8_t gps_getSecond() {
    return (uint8_t)gps_getLocalNow()->tm_sec;
}

uint64_t gps_getEpoch() {
    return (uint64_t)mktime( gps_getGMNow() );
}

char * gps_DecimalToDegreeMinutes(float_t decimal) {
    char * retval = new char[20];

    if (decimal > 0.0F) {
        sprintf(retval, "%d %5.2f", int(floor(decimal)), (decimal - floor(decimal))*60);
    } else {
        sprintf(retval, "%d %5.2f", int(ceil(decimal)), ((decimal - ceil(decimal))*-1*60));
    }
    return retval;
}

/* private functions, epoch style tm structures (after 01/01/1900) */
tm * gps_getGMNow() {
    struct tm * gmnow;
    time_t rawtime;

    time( &rawtime );
    gmnow = gmtime( &rawtime );

    gmnow->tm_year =  gps.date.year() - 1900;
    gmnow->tm_mon =   gps.date.month() - 1;
    gmnow->tm_mday =  gps.date.day();
    gmnow->tm_hour =  gps.time.hour();
    gmnow->tm_min =   gps.time.minute();
    gmnow->tm_sec =   gps.time.second();

    return gmnow;
}

tm * gps_getLocalNow() {
    time_t rawtime = (time_t)gps_getEpoch() + (time_t)(CONFIG.TZOffset * 3600);
    return gmtime( &rawtime );
}
