#include <Arduino.h>
#include "tools.h"
#include "display.h"
#include "database.h"
#include "bme280.h"
#include "gps.h"
#include "ds3231.h"
#include "webservice.h"

#include <WiFi.h>
// used to switch WIFI and BT off

/**************************(Definieren der genutzten Variabeln)****************************/
config_param CONFIG;

// Interval zum aktualisieren vom Display mehr als >= 4min * 60sec
uint64_t lastDisplayUpdate = 946684800ULL;
uint64_t lastDataUpdate = 946684800ULL;

/*****************************************( Setup )****************************************/

void setup(void)
{
  Serial.begin(115200);
  DEBUG_PRINT("****( begin )****");

  /* comment out if program space is running low (takes ~40% of PROGRAM) */
  DEBUG_PRINT("disable BT");
  btStop();

  DEBUG_PRINT("Start WIFI AP");
  // github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiBlueToothSwitch/WiFiBlueToothSwitch.ino
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(CONFIG.APSSID, CONFIG.APPASS);
  
  DEBUG_PRINT("load config from file");
  config_get();

  DEBUG_PRINT("initialize RTC DS3231");
  ds3231_initialize();
  
  DEBUG_PRINT("initialize BME280");
  bme280_initialize();

  DEBUG_PRINT("initialize GPS");
  gps_initialize();

  if ( gps_DateTimeIsValid() ) {
    DEBUG_PRINT("Set RTC to GPS datetime");
    gps_delay(2000);   // get most recent values
    ds3231_setDateTimeEpoch( gps_getEpoch() );
  }

  DEBUG_PRINT("start webserver");
  webserver_initialize();

  DEBUG_PRINT("****( complete )****");
}

/*************************************(Hauptprogramm)**************************************/
void loop()
{
  
  if ( ds3231_getEpoch() - CONFIG.DataUpdateInterval >= lastDataUpdate ) {
    DEBUG_PRINT("Writing sensor data to db");
    lastDataUpdate = ds3231_getEpoch();
    datastore_push( gps_getLat(),
        gps_getLon(),
        gps_getAltitude(),
        gps_getCourse(),
        gps_getSpeed(),
        gps_getSatellites(),
        gps_getHDOP(),
        bme280_getTemperatureRaw(),
        bme280_getTemperature(),
        bme280_getTemperatureOffset(),
        bme280_getHumidityRaw(),
        bme280_getHumidity(),
        bme280_getPressureRaw(),
        bme280_getPressure(),
        bme280_getAltitude(),
        ds3231_getEpoch() );
    create_index_html(); 
  }

  if (( ds3231_getMinute() %5 == 0) && (ds3231_getEpoch() - CONFIG.DisplayUpdateInterval >= lastDisplayUpdate )) {
    DEBUG_PRINT("Update display");
    lastDisplayUpdate = ds3231_getEpoch(); // due to long running db fetch reset timer at the beginning
    datastore_fetch( ds3231_getEpoch() );
    gps_delay(1000);
    display_update();
  }
  
  if ( gps_DateTimeIsValid() && (max(gps_getEpoch(), ds3231_getEpoch()) - min(gps_getEpoch(), ds3231_getEpoch()) > 10ULL) ) {
      DEBUG_PRINT("RTC more than 10s off - resync RTC to GPS");
      gps_delay(2000);                    // get most recent values
      ds3231_setDateTimeEpoch( gps_getEpoch() );
  }
  
  gps_delay(1000);

  Serial.printf("%02u/%02u/%04u %02u:%02u:%02u - RTC Epoch: %llu (%s) - free heap: %u\n", ds3231_getDayOfMonth(), 
    ds3231_getMonth(),
    ds3231_getYear(),
    ds3231_getHour(),
    ds3231_getMinute(),
    ds3231_getSecond(),
    ds3231_getEpoch(),
    ( ds3231_IsValid() ? "valid" : "invalid" ),
    ESP.getFreeHeap() );  
}