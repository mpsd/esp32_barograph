#ifndef TOOLS_H
#define TOOLS_H

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(p) Serial.printf( "%s() in file: %s(%d) - %s\n", __func__ , __FILE__, __LINE__, p );
#endif

#ifndef UBOUND
#define UBOUND(a) (sizeof(a) / sizeof(a[0]))
#endif

/*

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

  uBlox Neo6    ESP32 Devkit1 / UART:
  VCC           3V
  GND           GND
  RX            D34: TX
  TX            D35: RX
*/

// Use HSPI for SD Card with custom pin assignment
#define _sd_cs 15
#define _sd_clk 14
#define _sd_mosi 26
#define _sd_miso 27

#define _epd_busy 4
#define _epd_rst 16
#define _epd_dc 17
#define _epd_spi_cs 5
#define _epd_spi_clk 18
#define _epd_spi_mosi 23

#define _gps_tx 34
#define _gps_rx 35

/**************************(Declare global Variables)****************************/
struct config_param {
  float_t Altitude = 540.0F;
  uint64_t DataUpdateInterval = 60ULL;
  uint64_t DisplayUpdateInterval = 300ULL;

  uint32_t SDSpeed = 27000000U;
  int32_t GPSBaud = 9600;
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