#include <Arduino.h>
#include "database.h"

SPIClass _spiSD(HSPI);
sqlite3 *dbconn = NULL;

char sqlbuffer[SQLBUFFSIZE];

db_hourly_value db_hourly_values[HOURLY_VALUES];
db_graph_value db_graph_values[GRAPH_VALUES];

void config_get() {
  
  // SD on HSPI Port
  // github.com/espressif/arduino-esp32/issues/1219
  _spiSD.begin(/* CLK */ _sd_clk, /* MISO */ _sd_miso, /* MOSI */ _sd_mosi, /* CS */ _sd_cs);

  if ( !SD.begin( _sd_cs, _spiSD, CONFIG.SDSpeed) ) {
    DEBUG_PRINT("Card Mount Failed");
    return;
  }

  DEBUG_PRINT("Read Altitude from SD Card");
  File confile = SD.open(CONFIG.AltitudeFile);
  String buffer;

  if (!confile) {
    DEBUG_PRINT("Altitude config file not found");
  }
  else {
    CONFIG.Altitude = confile.readStringUntil('\n').toFloat();
    DEBUG_PRINT("Altitude defined");
  }
  confile.close();

  DEBUG_PRINT("Read Timezone offset from SD Card");
  confile = SD.open(CONFIG.TZOffsetFile);
  
  if (!confile) {
    DEBUG_PRINT("Timezone config file not found");
  }
  else {
    CONFIG.TZOffset = confile.readStringUntil('\n').toInt();
    DEBUG_PRINT("TZOffset defined");
  }
  confile.close();

  DEBUG_PRINT("Read Temperature offset from SD Card");
  confile = SD.open(CONFIG.TemperatureOffsetFile);
  
  if (!confile) {
    DEBUG_PRINT("Temperature offset config file not found");
  }
  else {
    CONFIG.TemperatureOffset = confile.readStringUntil('\n').toFloat();
    DEBUG_PRINT("TemperatureOffset defined");
  }
  confile.close();

  _spiSD.end();
}

void config_set() {
  
  // SD on HSPI Port
  // github.com/espressif/arduino-esp32/issues/1219
  _spiSD.begin(/* CLK */ _sd_clk, /* MISO */ _sd_miso, /* MOSI */ _sd_mosi, /* CS */ _sd_cs);

  if ( !SD.begin( _sd_cs, _spiSD, CONFIG.SDSpeed) ) {
    DEBUG_PRINT("Card Mount Failed");
    return;
  }

  DEBUG_PRINT("Write Altitude to SD Card");
  File confile = SD.open(CONFIG.AltitudeFile, FILE_WRITE);
  String buffer;

  confile.printf("%0.1fF", CONFIG.Altitude);
  confile.println();

  confile.close();
  DEBUG_PRINT("Altitude defined");
  
  DEBUG_PRINT("Write Timezone Offset to SD Card");
  confile = SD.open(CONFIG.TZOffsetFile, FILE_WRITE);
  confile.printf("%d", CONFIG.TZOffset);
  confile.println();

  confile.close();
  DEBUG_PRINT("TZOffset defined");
  
  DEBUG_PRINT("Write Temperature offset to SD Card");
  confile = SD.open(CONFIG.TemperatureOffsetFile, FILE_WRITE);
  confile.printf("%0.1fF", CONFIG.TemperatureOffset);
  confile.println();

  confile.close();
  DEBUG_PRINT("Temperature offset defined");

  _spiSD.end();
}

void db_initialize() {
  DEBUG_PRINT("initialize SD Card");
  // SD on HSPI Port
  // github.com/espressif/arduino-esp32/issues/1219
  _spiSD.begin(/* CLK */ _sd_clk, /* MISO */ _sd_miso, /* MOSI */ _sd_mosi, /* CS */ _sd_cs);


  if ( !SD.begin( _sd_cs, _spiSD, CONFIG.SDSpeed) ) {
    DEBUG_PRINT("Card Mount Failed");
    return;
  }

  if (dbconn != NULL)
    sqlite3_close(dbconn);

  DEBUG_PRINT("initialize SQLite3");
  sqlite3_initialize();

  DEBUG_PRINT("open SQLite3 DBfile");
  if ( sqlite3_open(CONFIG.SQLiteFile, &dbconn) ) {
    DEBUG_PRINT("Unable to open database file - ");
    DEBUG_PRINT(CONFIG.SQLiteFile);
    DEBUG_PRINT(sqlite3_errmsg(dbconn));
  }

}

void db_close() {
  DEBUG_PRINT("close DBfile");

  sqlite3_close(dbconn);
  sqlite3_db_release_memory(dbconn);
  sqlite3_shutdown();
 
// SD.end(); // seems to leak the heap by 24b

  _spiSD.end();  
}

void db_fetchData() {

  DEBUG_PRINT("****( begin )****");
  db_initialize();

  int error = 0;
  sqlite3_stmt *res1;

  uint64_t current_timestamp = 0;
  
  DEBUG_PRINT("Reset variables");

  for (int i=0; i < UBOUND(db_hourly_values); i++) {
    db_hourly_values[i].temperature  = 0;
    db_hourly_values[i].humidity     = 0;
    db_hourly_values[i].pressure     = 0;
    db_hourly_values[i].timestamp    = 0;
    db_hourly_values[i].chg_pressure = 0;
  }

  for (int i=0; i < UBOUND(db_graph_values); i++){
    db_graph_values[i].x = 0;
    db_graph_values[i].temperature = 0;
    db_graph_values[i].pressure = 0;
    db_graph_values[i].humidity = 0;
    db_graph_values[i].timestamp = 0;
  }

  current_timestamp = ds3231_getEpoch();

  DEBUG_PRINT("Retrieve data for graph (200px)");
  sprintf(sqlbuffer, 
      "SELECT 200-((%llu - gmtimestamp)*200/(24*3600)) as id, gmtimestamp, temperature, pressure, humidity, (%llu - gmtimestamp) as timestampoffset FROM t_datalog WHERE gmtimestamp >= (%llu - 24*3600) ORDER BY gmtimestamp ASC LIMIT 2000;",
      current_timestamp,
      current_timestamp,
      current_timestamp);

  DEBUG_PRINT(sqlbuffer);

  error = sqlite3_prepare_v2(dbconn, sqlbuffer, -1, &res1, NULL);
  if ( error != SQLITE_OK ) {
    DEBUG_PRINT( sqlite3_errstr(error) );
    DEBUG_PRINT( sqlite3_errmsg(dbconn) );
  }

  while (sqlite3_step(res1) == SQLITE_ROW) {
    Serial.printf("id (x): %d, tst: %d, tstoffset: %d \n", sqlite3_column_int(res1, 0), sqlite3_column_int(res1, 1), sqlite3_column_int(res1, 5));

    if ( sqlite3_column_int(res1, 0) < UBOUND(db_graph_values) ) {
      db_graph_values[ sqlite3_column_int(res1, 0) ].x = sqlite3_column_int(res1, 0);
      db_graph_values[ sqlite3_column_int(res1, 0) ].timestamp = sqlite3_column_int(res1, 1);
      db_graph_values[ sqlite3_column_int(res1, 0) ].temperature = sqlite3_column_double(res1, 2);
      db_graph_values[ sqlite3_column_int(res1, 0) ].pressure = sqlite3_column_double(res1, 3);
      db_graph_values[ sqlite3_column_int(res1, 0) ].humidity = sqlite3_column_double(res1, 4);
    }
  }
  sqlite3_finalize(res1);


  DEBUG_PRINT("Retrieve hourly data");
  sprintf(sqlbuffer,
      "SELECT abs(%llu - gmtimestamp)/3600 as id, temperature, humidity, pressure, gmtimestamp, abs(%llu - gmtimestamp)%%3600 as timestampoffset FROM t_datalog WHERE timestampoffset < 300 ORDER BY id ASC, timestampoffset DESC LIMIT 100;",
      current_timestamp,
      current_timestamp);
  DEBUG_PRINT(sqlbuffer);
  
  error = sqlite3_prepare_v2(dbconn, sqlbuffer, -1, &res1, NULL);
  if ( error != SQLITE_OK ) {
    DEBUG_PRINT( sqlite3_errstr(error) );
    DEBUG_PRINT( sqlite3_errmsg(dbconn) );
  }

  while (sqlite3_step(res1) == SQLITE_ROW) {
    Serial.printf("id: %d, tst: %d, tstoffset: %d \n", sqlite3_column_int(res1, 0), sqlite3_column_int(res1, 4), sqlite3_column_int(res1, 5));

    if ( sqlite3_column_int(res1, 0) < UBOUND(db_hourly_values) ) {
      db_hourly_values[ sqlite3_column_int(res1, 0) ].temperature = sqlite3_column_double(res1, 1);
      db_hourly_values[ sqlite3_column_int(res1, 0) ].humidity    = sqlite3_column_double(res1, 2);
      db_hourly_values[ sqlite3_column_int(res1, 0) ].pressure    = sqlite3_column_double(res1, 3);
      db_hourly_values[ sqlite3_column_int(res1, 0) ].timestamp   = sqlite3_column_int(res1, 4);
    }
  }
  sqlite3_finalize(res1);

  DEBUG_PRINT("Calculate pressure changes");
  if (db_hourly_values[1].pressure > 0 ) { db_hourly_values[0].chg_pressure = db_hourly_values[0].pressure - db_hourly_values[1].pressure; }
  
  for (int i=1; i < UBOUND(db_hourly_values); i++) {
    if ( db_hourly_values[i].pressure > 0 ) { db_hourly_values[i].chg_pressure = db_hourly_values[0].pressure - db_hourly_values[i].pressure; }
  }
  
  db_close();
  DEBUG_PRINT("****( complete )****");
}

void db_pushData(float_t lat, float_t lon, float_t alt_m, float_t crs, float_t spd, uint32_t sat, float_t hdop, float_t temp_raw, float_t temp, float_t temp_offset, float_t hum_raw, float_t hum, float_t press_raw, float_t press, float_t alt, uint64_t tst) {
  DEBUG_PRINT("****( begin )****");
  
  db_initialize();
 
  sprintf(sqlbuffer, 
    "INSERT INTO t_datalog(lat, lon, altitude_m, course_deg, speed_ms, satellites, hdop, temperature_raw, temperature, temperature_offset, humidity_raw, humidity, pressure_raw, pressure, altitude, gmtimestamp) VALUES(%0.6f, %0.6f, %0.2f, %0.2f, %0.2f, %u, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f, %llu);",
    lat, lon, alt_m, crs, spd, sat, hdop, temp_raw, temp, temp_offset, hum_raw, hum, press_raw, press, alt, tst);
  DEBUG_PRINT(sqlbuffer);
  
  int32_t error = sqlite3_exec(dbconn, sqlbuffer, 0, 0, NULL);
  if (error != SQLITE_OK ) {
    DEBUG_PRINT("SQL error");
    DEBUG_PRINT(sqlite3_errstr(error));
    DEBUG_PRINT(sqlite3_errmsg(dbconn));

    if (error == SQLITE_NOMEM) {
      db_close();
      ESP.restart();
    }
  }

  db_close();

  DEBUG_PRINT("****( complete )****");
}