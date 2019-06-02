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
  DEBUG_PRINT("IP address: ");
  DEBUG_PRINT(WiFi.softAPIP().toString().c_str());

  DEBUG_PRINT("load config from file");
  config_get();

  DEBUG_PRINT("initialize DB");
  db_fetchData();
  
  DEBUG_PRINT("initialize BME280");
  bme280_initialize();
  
  DEBUG_PRINT("initialize ePaper");
  display_update();

  DEBUG_PRINT("initialize GPS");
  gps_initialize();

  DEBUG_PRINT("initialize RTC DS3231");
  ds3231_initialize();

  DEBUG_PRINT("start webserver");
  webserver_initialize();

  DEBUG_PRINT("****( complete )****");

}

/*************************************(Hauptprogramm)**************************************/
void loop()
{
  
  if ( gps_getEpoch() - CONFIG.DataUpdateInterval >= lastDataUpdate ) {
    DEBUG_PRINT("Writing sensor data to db");
    db_pushData( gps_getLat(),
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
        gps_getEpoch() );
    lastDataUpdate = gps_getEpoch();
  }

  if (( gps_getMinute() %5 == 0) && (gps_getEpoch() - CONFIG.DisplayUpdateInterval >= lastDisplayUpdate )) {
    DEBUG_PRINT("Update display");
    lastDisplayUpdate = gps_getEpoch(); // due to long running db fetch reset timer at the beginning
    db_fetchData();
    display_update();
  }

  gps_delay(1000);
}