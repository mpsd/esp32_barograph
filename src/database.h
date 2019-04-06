#ifndef DATABASE_H
#define DATABASE_H

#include "tools.h"

#include <sqlite3.h>
// builtin libraries
#include <SPI.h>
#include <FS.h>
#include <SD.h>

#define SQLBUFFSIZE 450
#define SDSPEED 27000000

// Use HSPI for SD Card with custom pin assignment
#define _sd_cs 15
#define _sd_clk 14
#define _sd_mosi 26
#define _sd_miso 27

void altitude_fetch(void);

void db_initialize(void);
void db_close(void);
void db_fetchData(void);
void db_pushData(float_t temp, float_t hum, float_t press, float_t alt, float_t press_raw, uint64_t tst);

#endif