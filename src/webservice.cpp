#include <Arduino.h>
#include "webservice.h"
#include "database.h"   // for write config_set()
#include "gps.h"        // for GPS functions

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer webserver(80);

void webserver_initialize() {

    webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->addHeader("Server","ESP Async Web Server");
        response->print("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />");
        response->printf("<title>Webpage at %s</title>", request->url().c_str());
        response->print("<style> body {font: normal 10px Verdana, Arial, sans-serif;} </style>");
        response->print("</head><body>");

        response->print("Client ");
        response->print(request->client()->remoteIP());
        response->printf(" / system uptime: %lu s", millis()/1000);
        
        response->printf("<h2>Current Values at %02u:%02u:%02u on %02u/%02u/%04u</h2>", gps_getHour(), gps_getMinute(), gps_getSecond(), gps_getDayOfMonth(), gps_getMonth(), gps_getYear() );
        response->printf("0h:%4.0fhPa %+4.1f / %2.0fC / %2.0f%%<br>", db_hourly_values[0].pressure, db_hourly_values[0].chg_pressure, db_hourly_values[0].temperature, db_hourly_values[0].humidity);
        response->printf("3h:%4.0fhPa %+4.1f / %2.0fC / %2.0f%%<br>", db_hourly_values[3].pressure, db_hourly_values[3].chg_pressure, db_hourly_values[3].temperature, db_hourly_values[3].humidity);
        response->printf("6h:%4.0fhPa %+4.1f / %2.0fC / %2.0f%%<br>", db_hourly_values[6].pressure, db_hourly_values[6].chg_pressure, db_hourly_values[6].temperature, db_hourly_values[6].humidity);

        response->printf( "GPS: Sat: %02u, Lat: %08.6f, Lon: %08.6f, HDOP: %04.2f, Alt: %4.0f, Course: %3.0f, Speed: %2.0f<br>",
            gps_getSatellites(),
            gps_getLat(),
            gps_getLon(),
            gps_getHDOP(),
            gps_getAltitude(),
            gps_getCourse(),
            gps_getSpeed() );
            
        response->print("<h2>Current Config</h2>");
        response->print("<form action=\"/configsave\" method=\"post\">");
        response->printf("<label for=\"alt\">%s:</label><input id=\"alt\" name=\"alt\" type=\"number\" value=\"%0.0f\"><br>", CONFIG.AltitudeFile, CONFIG.Altitude);
        response->printf("<label for=\"tz\">%s:</label><input id=\"tz\" name=\"tz\" type=\"number\" value=\"%d\"><br>", CONFIG.TZOffsetFile, CONFIG.TZOffset);
        response->print("<button type=\"submit\">Save</button>");
        response->print("</form>");
        
        response->print("</body></html>");
        //send the response last
        request->send(response);
    });

    webserver.on("/configsave", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->addHeader("Server","ESP Async Web Server");
        response->print("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />");
        response->printf("<title>Webpage at %s</title>", request->url().c_str());
        response->print("<style> body {font: normal 10px Verdana, Arial, sans-serif;} </style>");
        response->print("</head><body>");

        int params = request->params();
        for(int i=0;i<params;i++) {
            AsyncWebParameter* p = request->getParam(i);
            if (p->isPost()) {
                response->printf("POST[%s]: %s<br>", p->name().c_str(), p->value().c_str());

                if ( strcmp(p->name().c_str(), "alt") == 0) {
                    DEBUG_PRINT("Set Altitude");
                    CONFIG.Altitude = p->value().toFloat();
                }
                if ( strcmp(p->name().c_str(),"tz") == 0 ) {
                    DEBUG_PRINT("Set TZ offset");
                    CONFIG.TZOffset = p->value().toInt();
                }

            } else {
                response->printf("GET[%s]: %s<br>", p->name().c_str(), p->value().c_str());
            }
        }

        response->print("<br><br><a href=\"/\">back</a></body></html>");
        //send the response last
        request->send(response);

        config_set();
    });

    webserver.on("/debug", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->addHeader("Server","ESP Async Web Server");
        response->print("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />");
        response->printf("<title>Webpage at %s</title>", request->url().c_str());
        response->print("<style> body {font: normal 10px Verdana, Arial, sans-serif;} </style>");
        response->print("</head><body>");

        response->print("<h2>General</h2>");
        response->print("<ul>");
        response->printf("<li>Version: HTTP/1.%u</li>", request->version());
        response->printf("<li>Method: %s</li>", request->methodToString());
        response->printf("<li>URL: %s</li>", request->url().c_str());
        response->printf("<li>Host: %s</li>", request->host().c_str());
        response->printf("<li>ContentType: %s</li>", request->contentType().c_str());
        response->printf("<li>ContentLength: %u</li>", request->contentLength());
        response->printf("<li>Multipart: %s</li>", request->multipart()?"true":"false");
        response->print("</ul>");

        response->print("<h2>Headers</h2>");
        response->print("<ul>");
        int headers = request->headers();
        for(int i=0;i<headers;i++){
        AsyncWebHeader* h = request->getHeader(i);
        response->printf("<li>%s: %s</li>", h->name().c_str(), h->value().c_str());
        }
        response->print("</ul>");

        response->print("<h2>Parameters</h2>");
        response->print("<ul>");
        int params = request->params();
        for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isFile()){
                response->printf("<li>FILE[%s]: %s, size: %u</li>", p->name().c_str(), p->value().c_str(), p->size());
            } else if(p->isPost()){
                response->printf("<li>POST[%s]: %s</li>", p->name().c_str(), p->value().c_str());
            } else {
                response->printf("<li>GET[%s]: %s</li>", p->name().c_str(), p->value().c_str());
            }
        }
        response->print("</ul>");
        response->print("</body></html>");
        //send the response last
        request->send(response);
    });

    webserver.begin();
}