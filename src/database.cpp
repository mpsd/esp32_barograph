#include <Arduino.h>
#include "database.h"

SPIClass _spiSD(HSPI);
sqlite3 *dbconn;

db_hourly_value db_hourly_values[HOURLY_VALUES];
db_pressure_graph_value db_pressure_graph_values[GRAPH_VALUES];

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

  DEBUG_PRINT("Read Timezone Offset from SD Card");
  confile = SD.open(CONFIG.TZOffsetFile);
  
  if (!confile) {
    DEBUG_PRINT("Timezone config file not found");
  }
  else {
    CONFIG.TZOffset = confile.readStringUntil('\n').toInt();
    DEBUG_PRINT("TZOffset defined");
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

  DEBUG_PRINT("initialize SQLite3");
  sqlite3_initialize();

  DEBUG_PRINT("open DBfile");  
  if ( sqlite3_open(CONFIG.SQLiteFile, &dbconn) ) {
    DEBUG_PRINT("Unable to open database file - ");
    DEBUG_PRINT(CONFIG.SQLiteFile);
    return;
  }
}

void db_close() {
  sqlite3_close(dbconn);
  _spiSD.end();
}

void db_fetchData() {

  DEBUG_PRINT("****( begin )****");
  db_initialize();

  int error = 0;
  sqlite3_stmt *res1;
  // sqlite3_stmt *res2;
  char sqlbuffer[SQLBUFFSIZE];

  time_t current_timestamp = 0;
  uint32_t id = 0;

  DEBUG_PRINT("Reset variables");

  for (int i=0; i < UBOUND(db_hourly_values); i++) {
    db_hourly_values[i].temperature  = 0;
    db_hourly_values[i].humidity     = 0;
    db_hourly_values[i].pressure     = 0;
    db_hourly_values[i].timestamp    = 0;
    db_hourly_values[i].chg_pressure = 0;
  }

  for (int i=0; i < UBOUND(db_pressure_graph_values); i++){
    db_pressure_graph_values[i].x = 0;
    db_pressure_graph_values[i].pressure = 0;
    db_pressure_graph_values[i].timestamp = 0;
  }

  sprintf(sqlbuffer, "SELECT max(gmtimestamp) FROM t_datalog;");
  DEBUG_PRINT(sqlbuffer);
  
  error = sqlite3_prepare_v2(dbconn, sqlbuffer, -1, &res1, NULL);
  if ( error != SQLITE_OK ) {
    DEBUG_PRINT("No data found");
    DEBUG_PRINT(sqlite3_errstr(error));
    sqlite3_finalize(res1);
    db_close();
    return;
  }
  while (sqlite3_step(res1) == SQLITE_ROW) {
    current_timestamp = sqlite3_column_int(res1, 0);
  }
  sqlite3_finalize(res1);

  DEBUG_PRINT("Retrieve data for graph (200px)");
  sprintf(sqlbuffer, "SELECT gmtimestamp, pressure, (%ld - gmtimestamp) as timestampoffset FROM t_datalog WHERE gmtimestamp >= (%ld - (24*3600)) ORDER BY gmtimestamp DESC;",
      current_timestamp,
      current_timestamp);
  DEBUG_PRINT(sqlbuffer);

  error = sqlite3_prepare_v2(dbconn, sqlbuffer, -1, &res1, NULL);
  if ( error != SQLITE_OK ) {
    DEBUG_PRINT( sqlite3_errstr(error) );
    DEBUG_PRINT( sqlite3_errmsg(dbconn) );
  }

  while (sqlite3_step(res1) == SQLITE_ROW) {
    id = floor(200-(sqlite3_column_int(res1, 2)*200/(24*3600)));
    Serial.printf("id: %u, tst: %d, tstoffset: %d \n", id, sqlite3_column_int(res1, 0), sqlite3_column_int(res1, 2));

    db_pressure_graph_values[ id ].x = id;
    db_pressure_graph_values[ id ].pressure = sqlite3_column_double(res1, 1);
    db_pressure_graph_values[ id ].timestamp = sqlite3_column_int(res1, 0);
  }
  sqlite3_finalize(res1);


  DEBUG_PRINT("Retrieve hourly data");
  for (int i=0; i < UBOUND(db_hourly_values); i++) {
    sprintf(sqlbuffer, 
        "SELECT temperature, humidity, pressure, gmtimestamp FROM t_datalog WHERE abs(%ld - gmtimestamp) < 300 ORDER BY abs(%ld - gmtimestamp) ASC LIMIT 1;",
        current_timestamp - (i * 3600),
        current_timestamp - (i * 3600));
    DEBUG_PRINT(sqlbuffer);

    error = sqlite3_prepare_v2(dbconn, sqlbuffer, -1, &res1, NULL);

    if ( error != SQLITE_OK ) {
      DEBUG_PRINT("No data found");
      DEBUG_PRINT(sqlite3_errstr(error));
      sqlite3_finalize(res1);
      db_close();
      return;
    }

    while (sqlite3_step(res1) == SQLITE_ROW) {
      db_hourly_values[i].temperature = sqlite3_column_double(res1, 0);
      db_hourly_values[i].humidity    = sqlite3_column_double(res1, 1);
      db_hourly_values[i].pressure    = sqlite3_column_double(res1, 2);
      db_hourly_values[i].timestamp   = sqlite3_column_int(res1, 3);
    }
    sqlite3_finalize(res1);
  }

  DEBUG_PRINT("Calculate pressure changes");
  if (db_hourly_values[1].pressure > 0 ) { db_hourly_values[0].chg_pressure = db_hourly_values[0].pressure - db_hourly_values[1].pressure; }
  
  for (int i=1; i < UBOUND(db_hourly_values); i++) {
    if ( db_hourly_values[i].pressure > 0 ) { db_hourly_values[i].chg_pressure = db_hourly_values[0].pressure - db_hourly_values[i].pressure; }
  }
  
  db_close();
  DEBUG_PRINT("****( complete )****");
}

void db_pushData(float_t lat = 0, float_t lon = 0, float_t alt_m = 0, float_t crs = 0, float_t spd = 0, uint32_t sat = 0, float_t hdop = 0, float_t temp = 0, float_t hum = 0, float_t press_raw = 0, float_t press = 0,  float_t alt = 0, uint64_t tst = 0) {
  DEBUG_PRINT("****( begin )****");
  db_initialize();

  int error = 0;
  char sqlbuffer[SQLBUFFSIZE];
  
  sprintf(sqlbuffer, 
    "INSERT INTO t_datalog(lat, lon, altitude_m, course_deg, speed_ms, satellites, hdop, temperature, humidity, pressure_raw, pressure, altitude, gmtimestamp) VALUES(%0.6f, %0.6f, %0.2f, %0.2f, %0.2f, %u, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f, %llu);",
    lat, lon, alt_m, crs, spd, sat, hdop, temp, hum, press_raw, press, alt, tst);
  DEBUG_PRINT(sqlbuffer);

  error = sqlite3_exec(dbconn, sqlbuffer, 0, 0, NULL);
  if (error != SQLITE_OK ) {
    DEBUG_PRINT("SQL error");
    DEBUG_PRINT(sqlite3_errstr(error));
    DEBUG_PRINT(sqlite3_errmsg(dbconn));
  }

  db_close();
  DEBUG_PRINT("****( complete )****");
}