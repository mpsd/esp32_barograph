#ifndef TOOLS_H
#define TOOLS_H

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(p) Serial.printf( "%s() in file: %s(%d) - %s\n", __func__ , __FILE__, __LINE__, p );
#endif

#ifndef UBOUND
#define UBOUND(a) (sizeof(a) / sizeof(a[0]))
#endif

/**************************(Declare global Variables)****************************/
struct config_param {
  float_t Altitude = 540.0F;
  uint64_t DataUpdateInterval = 60ULL;
  uint64_t DisplayUpdateInterval = 300ULL;

  uint32_t SDSpeed = 27000000U;
  const char* AltitudeFile = "/config/altitude";
  const char* SQLiteFile = "/sd/tables.sqlite3";
};
extern config_param CONFIG;

struct db_hourly_value {
  float_t temperature;
  float_t pressure;
  float_t humidity;
  uint64_t timestamp;
  float_t chg_pressure;
};
extern db_hourly_value db_hourly_values[];
#define HOURLY_VALUES 7

struct db_pressure_graph_value {
  uint16_t x;
  float_t pressure;
  uint64_t timestamp;
};
extern db_pressure_graph_value db_pressure_graph_values[];
#define GRAPH_VALUES 201

#endif