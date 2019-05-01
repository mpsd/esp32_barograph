#include <Arduino.h>
#include "webservice.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define FORM_ALTITUDE "alt"
#define FORM_TZOFFSET "tzo"
#define FORM_TEMPOFFSET "tempofst"

AsyncWebServer webserver(80);

void webserver_initialize() {

    webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->addHeader("Server","ESP Async Web Server");
        response->print("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />");
        response->print("<meta http-equiv=\"refresh\" content=\"20\" />");
        response->printf("<title>Webpage at %s</title>", request->url().c_str());
        response->print("<style> body {font: normal 12px Verdana, Arial, sans-serif;} </style>");
        response->print("</head><body>");

        response->print("Client ");
        response->print(request->client()->remoteIP());
        response->printf(" / system uptime: %02lu:%02lu.%02lu<br>", (millis()/1000/60/60) % 24, (millis()/1000/60) % 60, (millis()/1000) % 60);
        
        response->printf("<h2>Current Values at %02u:%02u:%02u on %02u/%02u/%04u</h2>", gps_getHour(), gps_getMinute(), gps_getSecond(), gps_getDayOfMonth(), gps_getMonth(), gps_getYear() );
        response->print("<h3>Climate data</h3>");
        for (int i=0; i < UBOUND(db_hourly_values); i++) {
            response->printf("%dh: %4.0fhPa %+4.1f / %2.0fC / %2.0f%%<br>", i, db_hourly_values[i].pressure, db_hourly_values[i].chg_pressure, db_hourly_values[i].temperature, db_hourly_values[i].humidity);
        }
        response->printf("<br>Dewpoint: %+4.1fC<br>", bme280_getDewPoint());

        response->print("<h3>GPS data</h3>");
        response->printf( "Sat: %02u, HDOP: %04.2f<br>Lat: %08.6f, Lon: %08.6f<br>Alt: %4.0f<br>Course: %3.0f<br>Speed: %2.0f<br>",
            gps_getSatellites(),
            gps_getHDOP(),
            gps_getLat(),
            gps_getLon(),
            gps_getAltitude(),
            gps_getCourse(),
            gps_getSpeed() );
        response->printf( "<br>Lat/Lon (Deg MM.MM): %s / %s<br>", gps_DecimalToDegreeMinutes(gps_getLat()), gps_DecimalToDegreeMinutes(gps_getLon()));
        
        response->print("<h2>Current Config</h2>");
        response->print("<form action=\"/configsave\" method=\"post\">");
        response->printf("<label for=\"%s\">%s:</label><input id=\"%s\" name=\"%s\" type=\"number\" value=\"%0.0f\"><br>", FORM_ALTITUDE, CONFIG.AltitudeFile, FORM_ALTITUDE, FORM_ALTITUDE, CONFIG.Altitude);
        response->printf("<label for=\"%s\">%s:</label><input id=\"%s\" name=\"%s\" type=\"number\" value=\"%d\"><br>", FORM_TZOFFSET, CONFIG.TZOffsetFile, FORM_TZOFFSET, FORM_TZOFFSET, CONFIG.TZOffset);
        response->printf("<label for=\"%s\">%s:</label><input id=\"%s\" name=\"%s\" type=\"number\" value=\"%0.0f\"><br>", FORM_TEMPOFFSET, CONFIG.TemperatureOffsetFile, FORM_TEMPOFFSET, FORM_TEMPOFFSET, CONFIG.TemperatureOffset);
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

                if ( strcmp(p->name().c_str(), FORM_ALTITUDE) == 0) {
                    DEBUG_PRINT("Set Altitude");
                    CONFIG.Altitude = p->value().toFloat();
                }
                if ( strcmp(p->name().c_str(), FORM_TZOFFSET) == 0 ) {
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