#include <Arduino.h>
/*******************************************************************************************
 *******************************************************************************************
 **                                www.bastelgarage.ch                                    **
 ** Der Onlineshop mit Videoanleitungen und kompletten Bausätzen für Anfänger und Profis! **
 **                                                                                       **
 ** Verwendete Library:                                                                   **
 ** E-Ink Display Waveshare 200x200 1.54" GxEPD: https://github.com/ZinggJM/GxEPD2         **
 ** BME280: https://github.com/e-radionicacom/BME280-Arduino-Library                      **
 **                                                                                       **
 *******************************************************************************************
  Pinbelegung:
  Display:      ESP32 Devkitv1 / vSPI Interface:
  BUSY          D4: 4
  RST           RX2: 16
  DC            TX2: 17
  CS            D5: 5   (SS)
  CLK           D18: 18 (SCK)
  DIN           D23: 23 (MOSI)
  GND           GND
  3.3V          3V3

  SD MMC:       ESP32 Devkitv1 / HSPI Interface with custom pin assignment to avoid GPIO12 voltage selection:
  CS            15 (SS)
  CLK           14 (SCK)
  DIN           26 (MOSI)
  DOUT          27 (MISO)

  BME280:       ESP32 Devkitv1 / I2C 0x76:
  VCC           3V
  GND           GND
  SCL           D22: 22 (I2C SCL)
  SDA           D21: 21 (I2C SDA)

  DS3231 RTC:   ESP32 Devkitv1 / I2C:
  VCC           3V
  GND           GND
  SCL           D22: 22 (I2C SCL)
  SDA           D21: 21 (I2C SDA)

*/

#include "tools.h"
#include "display.h"
#include "database.h"
#include "bme280.h"
#include "ds3231.h"

#include <WiFi.h>
// used to switch WIFI and BT off

/************************( Importieren der genutzten Bibliotheken )************************/

/**************************(Definieren der genutzten Variabeln)****************************/
// Interval zum aktualisieren vom Display mehr als >= 4min * 60sec
#define DISPLAYUPDATE 240
uint64_t lastDisplayUpdate = 946684800ULL;

#define DATAUPDATE 60
uint64_t lastDataUpdate = 946684800ULL;


/**************************( Eigene Funktionen )******************************************/

/**************************( DB Funktionen )*****************************************************/


/*****************************************( Setup )****************************************/
void setup(void)
{
  Serial.begin(115200);
  DEBUG_PRINT("****( begin )****");
  /* comment out if program space is running low (take ~40% of PROGRAM) */
  DEBUG_PRINT("disable WIFI and BT");
  // github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiBlueToothSwitch/WiFiBlueToothSwitch.ino
  WiFi.mode(WIFI_OFF);
  btStop();

  DEBUG_PRINT("initialize DS3231 RTC");
  ds3231_initialize();

  // RtcDateTime now = RtcDateTime(__DATE__,__TIME__);
  // ds3231_setDateTime( now );
  altitude_fetch();

  DEBUG_PRINT("initialize DB");
  db_fetchData();
  
  DEBUG_PRINT("initialize BME280");
  bme280_initialize();
  
  DEBUG_PRINT("initialize ePaper");
  display_update();
 
  DEBUG_PRINT("****( complete )****");

}

/*************************************(Hauptprogramm)**************************************/
void loop()
{
  if ( !ds3231_IsValid() ) {
    DEBUG_PRINT("RTC lost confidence in the DateTime!");
  }

  if ( ds3231_getEpoch() < lastDataUpdate ) {
    DEBUG_PRINT("Set Time to lastDataUpdate");
    ds3231_setDateTime( lastDataUpdate );
  }
  
  if ( ds3231_getEpoch() - DATAUPDATE >= lastDataUpdate ) {
    DEBUG_PRINT("Writing sensor data to db");
    db_pushData(bme280_getTemperature(), bme280_getHumidity(), bme280_getPressure(), bme280_getAltitude(), bme280_getPressureRaw(),  ds3231_getEpoch() );
    lastDataUpdate = ds3231_getEpoch();
  }

  if (( ds3231_Minute() %5 == 0) && (ds3231_getEpoch() - DISPLAYUPDATE >= lastDisplayUpdate )) {
    DEBUG_PRINT("Update display");
    lastDisplayUpdate = ds3231_getEpoch(); // due to long running db fetch reset timer at the beginning
    db_fetchData();
    display_update();
  }

  delay(500);
}