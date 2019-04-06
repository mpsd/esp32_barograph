#include <Arduino.h>
#include "database.h"

SPIClass _spiSD(HSPI);
sqlite3 *dbconn;

const char* db_file=DB_FILE;

db_hourly_value db_hourly_values[HOURLY_VALUES];
db_pressure_graph_value db_pressure_graph_values[GRAPH_VALUES];

float_t altitude = ALTITUDE_STD;

void altitude_fetch() {
  DEBUG_PRINT("Read Altitude from SD Card");
  // SD on HSPI Port
  // github.com/espressif/arduino-esp32/issues/1219
  _spiSD.begin(/* CLK */ _sd_clk, /* MISO */ _sd_miso, /* MOSI */ _sd_mosi, /* CS */ _sd_cs);

  if ( !SD.begin( _sd_cs, _spiSD, SDSPEED) ) {
    DEBUG_PRINT("Card Mount Failed");
    return;
  }

  File altfile = SD.open(ALTITUDE_FILE);
  String buffer;

  if (!altfile) {
    DEBUG_PRINT("Altitude config file not found");
  }
  else {
    altitude = altfile.readStringUntil('\n').toFloat();
    DEBUG_PRINT("altitude defined");
  }
  altfile.close();
  _spiSD.end();
}

void db_initialize() {
  DEBUG_PRINT("initialize SD Card");
  // SD on HSPI Port
  // github.com/espressif/arduino-esp32/issues/1219
  _spiSD.begin(/* CLK */ _sd_clk, /* MISO */ _sd_miso, /* MOSI */ _sd_mosi, /* CS */ _sd_cs);

  if ( !SD.begin( _sd_cs, _spiSD, SDSPEED) ) {
    DEBUG_PRINT("Card Mount Failed");
    return;
  }

  DEBUG_PRINT("initialize SQLite3");
  sqlite3_initialize();

  DEBUG_PRINT("open DBfile");  
  if ( sqlite3_open(db_file, &dbconn) ) {
    DEBUG_PRINT("Unable to open database file");
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
  sqlite3_stmt *res2;
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

  sprintf(sqlbuffer, "SELECT max(timestamp) FROM t_bme280_values;");
  DEBUG_PRINT(sqlbuffer);
  
  error = sqlite3_prepare_v2(dbconn, sqlbuffer, -1, &res1, NULL);
  if ( error != SQLITE_OK ) {
    DEBUG_PRINT("No data found");
    DEBUG_PRINT(sqlite3_errstr(error));
    sqlite3_finalize(res1);
    return;
  }
  while (sqlite3_step(res1) == SQLITE_ROW) {
    current_timestamp = sqlite3_column_int(res1, 0);
  }
  sqlite3_finalize(res1);


  DEBUG_PRINT("Retrieve data for graph (200px)");

  sprintf(sqlbuffer, "SELECT id, (%ld + timestampoffset) as lookup_timestamp FROM t_24h_200px ORDER BY id ASC;", current_timestamp);
  DEBUG_PRINT(sqlbuffer);

  error = sqlite3_prepare_v2(dbconn, sqlbuffer, -1, &res1, NULL);
  if ( error != SQLITE_OK ) {
    DEBUG_PRINT("No data found");
    DEBUG_PRINT(sqlite3_errstr(error));
    sqlite3_finalize(res1);
    return;
  }

  sprintf(sqlbuffer, "SELECT timestamp, pressure FROM t_bme280_values WHERE abs(?1 - timestamp) < 300 ORDER BY abs(?1 - timestamp) ASC LIMIT 1;");
  DEBUG_PRINT(sqlbuffer);

  error = sqlite3_prepare_v2(dbconn, sqlbuffer, -1, &res2, NULL);
  if ( error != SQLITE_OK ) {
    DEBUG_PRINT(sqlite3_errstr(error));
    DEBUG_PRINT(sqlite3_errmsg(dbconn));
  }
  
  while (sqlite3_step(res1) == SQLITE_ROW) {
    id = sqlite3_column_int(res1, 0);

    error = sqlite3_bind_int(res2, 1, sqlite3_column_int(res1, 1));
    if ( error != SQLITE_OK ) {
      DEBUG_PRINT(sqlite3_errstr(error));
      DEBUG_PRINT(sqlite3_errmsg(dbconn));
    }

    db_pressure_graph_values[ id ].x = id; 
    
    while (sqlite3_step(res2) == SQLITE_ROW) {
      DEBUG_PRINT( sqlite3_column_text(res1, 0) );
      db_pressure_graph_values[ id ].timestamp = sqlite3_column_int(res2, 0); 
      db_pressure_graph_values[ id ].pressure  = sqlite3_column_double(res2, 1);
    }
    sqlite3_clear_bindings(res2);
    sqlite3_reset(res2);
  }
  sqlite3_finalize(res2);
  sqlite3_finalize(res1);
  
  for (int i=0; i < UBOUND(db_hourly_values); i++) {
    sprintf(sqlbuffer, 
        "SELECT temperature, humidity, pressure, timestamp FROM t_bme280_values WHERE abs(%ld - timestamp) < 300 ORDER BY abs(%ld - timestamp) ASC LIMIT 1;",
        current_timestamp - (i * 3600),
        current_timestamp - (i * 3600));
    DEBUG_PRINT(sqlbuffer);

    error = sqlite3_prepare_v2(dbconn, sqlbuffer, -1, &res1, NULL);

    if ( error != SQLITE_OK ) {
      DEBUG_PRINT("No data found");
      DEBUG_PRINT(sqlite3_errstr(error));
      sqlite3_finalize(res1);
      return;
    }

    DEBUG_PRINT("Retrieve data");
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
  if (db_hourly_values[3].pressure > 0 ) { db_hourly_values[3].chg_pressure = db_hourly_values[0].pressure - db_hourly_values[3].pressure; }
  if (db_hourly_values[6].pressure > 0 ) { db_hourly_values[6].chg_pressure = db_hourly_values[0].pressure - db_hourly_values[6].pressure; }
  
  db_close();
  DEBUG_PRINT("****( complete )****");
}

void db_pushData(float_t temp = 0, float_t hum = 0, float_t press = 0,  float_t alt = 0, float_t press_raw = 0, uint64_t tst = 0) {
  DEBUG_PRINT("****( begin )****");
  db_initialize();

  int error = 0;
  char sqlbuffer[SQLBUFFSIZE];
  
  sprintf(sqlbuffer, "INSERT INTO t_bme280_values(temperature, humidity, pressure, altitude, pressure_raw, timestamp) VALUES(%0.2f, %0.2f, %0.2f, %0.2f, %0.2f, %llu);", temp, hum, press, alt, press_raw, tst);
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
