#ifndef TOOLS_H
#define TOOLS_H

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(p) Serial.printf( "%s() in file: %s(%d) - %s\n", __func__ , __FILE__, __LINE__, p );
#endif

#ifndef UBOUND
#define UBOUND(a) (sizeof(a) / sizeof(a[0]))
#endif

#define ALTITUDE_FILE "/config/altitude"
#define ALTITUDE_STD 540.0F
extern float_t altitude;

#define DB_FILE "/sd/tables.sqlite3"

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