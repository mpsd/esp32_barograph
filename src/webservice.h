#ifndef WEBSERVICE_H
#define WEBSERVICE_H

#include "tools.h"
#include "database.h"   // for write config_set()
#include "gps.h"        // for GPS functions
#include "ds3231.h"
#include "bme280.h"     // for dewpoint

void webserver_initialize();

#endif