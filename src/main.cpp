#include <Arduino.h>
#include "tools.h"
#include "display.h"
#include "database.h"
#include "bme280.h"
#include "ds3231.h"
#include "gps.h"

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
  DEBUG_PRINT("disable WIFI and BT");
  // github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiBlueToothSwitch/WiFiBlueToothSwitch.ino
  WiFi.mode(WIFI_OFF);
  btStop();

  DEBUG_PRINT("initialize DS3231 RTC");
  ds3231_initialize();

  // RtcDateTime now = RtcDateTime(__DATE__,__TIME__);
  // ds3231_setDateTime( now );
  
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
  
  DEBUG_PRINT("****( complete )****");

}

/*************************************(Hauptprogramm)**************************************/
void loop()
{
  if ( !ds3231_IsValid() ) {
    DEBUG_PRINT("RTC lost confidence in the DateTime!");
  }

  if ( ds3231_getEpoch() - CONFIG.DataUpdateInterval >= lastDataUpdate ) {
    DEBUG_PRINT("Writing sensor data to db");
    db_pushData(bme280_getTemperature(), bme280_getHumidity(), bme280_getPressure(), bme280_getAltitude(), bme280_getPressureRaw(),  ds3231_getEpoch() );
    lastDataUpdate = ds3231_getEpoch();
  }

  if (( ds3231_Minute() %5 == 0) && (ds3231_getEpoch() - CONFIG.DisplayUpdateInterval >= lastDisplayUpdate )) {
    DEBUG_PRINT("Update display");
    lastDisplayUpdate = ds3231_getEpoch(); // due to long running db fetch reset timer at the beginning
    db_fetchData();
    display_update();
  }

  gps_delay(1000);
}