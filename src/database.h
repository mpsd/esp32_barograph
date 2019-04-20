#ifndef DATABASE_H
#define DATABASE_H

#include "tools.h"

#include <sqlite3.h>
// builtin libraries
#include <SPI.h>
#include <FS.h>
#include <SD.h>

#define SQLBUFFSIZE 450

void config_get(void);

void db_initialize(void);
void db_close(void);
void db_fetchData(void);
void db_pushData(float_t temp, float_t hum, float_t press, float_t alt, float_t press_raw, uint64_t tst);

#endif