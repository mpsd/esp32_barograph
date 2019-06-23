#ifndef WEBSERVICE_H
#define WEBSERVICE_H

#include "tools.h"
#include "database.h"   // for read/write config and read logfile chunks
#include "gps.h"        // for GPS functions
#include "ds3231.h"
#include "bme280.h"     // for dewpoint

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void webserver_initialize();

#endif