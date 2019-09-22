#include <Arduino.h>
#include "webservice.h"

#define FORM_ALTITUDE "alt"
#define FORM_TZOFFSET "tzo"
#define FORM_TEMPOFFSET "tempofst"

AsyncWebServer webserver(80);
char index_html[INDEX_HTML_LEN];

void create_index_html() {
    DEBUG_PRINT("****( begin )****");

    int index = snprintf(index_html, INDEX_HTML_LEN-1, "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /><meta http-equiv=\"refresh\" content=\"30\" /><title>esplogger</title><style> body { font: normal 12px Verdana, Arial, sans-serif; }</style></head><body>");

    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "system uptime: %02lu days %02lu:%02lu.%02lu<br>", (millis()/1000/60/60/24), (millis()/1000/60/60) % 24, (millis()/1000/60) % 60, (millis()/1000) % 60);

    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<h2>Current Values at %02u:%02u:%02u on %02u/%02u/%04u</h2>", ds3231_getHour(), ds3231_getMinute(), ds3231_getSecond(), ds3231_getDayOfMonth(), ds3231_getMonth(), ds3231_getYear() );
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "RTC: %02u:%02u:%02u %02u/%02u/%04u<br>", ds3231_getHour(), ds3231_getMinute(), ds3231_getSecond(), ds3231_getDayOfMonth(), ds3231_getMonth(), ds3231_getYear());
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "GPS: %02u:%02u:%02u %02u/%02u/%04u<br>", gps_getHour(), gps_getMinute(), gps_getSecond(), gps_getDayOfMonth(), gps_getMonth(), gps_getYear());
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<h3>Climate data</h3>");

    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "Current: %4.0fhPa / %2.0fC / %2.0f%% / dewpoint %2.1fC<br><br>", bme280_getPressure(), bme280_getTemperature(), bme280_getHumidity(), bme280_getDewPoint() ); 
    for (int i=0; i < UBOUND(db_hourly_values); i++) {
        index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "%dh: %4.0fhPa %+4.1f / %2.0fC / %2.0f%% / dewpoint %2.1fC<br>", 
            i, db_hourly_values[i].pressure, db_hourly_values[i].chg_pressure, db_hourly_values[i].temperature, db_hourly_values[i].humidity, bme280_getDewPoint(db_hourly_values[i].humidity, db_hourly_values[i].temperature));
    }
    
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<svg height=\"400\" width=\"400\">");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<g stroke-dasharray=\"1,1\" fill=\"none\" stroke=\"black\" stroke-width=\"1\">");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<polyline points=\"0,0 400,0\" /><polyline points=\"0,80 400,80\" /><polyline points=\"0,160 400,160\" />");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<polyline points=\"0,240 400,240\" /><polyline points=\"0,320 400,320\" /><polyline points=\"0,400 400,400\" />");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<polyline points=\"200,0 200,400\" /><polyline points=\"300,0 300,400\" /><polyline points=\"350,0 350,400\" /><polyline points=\"400,0 400,400\" /></g>");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<text x=\"5\" y=\"75\" style=\"fill:red;\">1020 hPa<tspan x=\"5\" y=\"235\">1000 hPa</tspan><tspan x=\"5\" y=\"395\">980 hPa</tspan></text>");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<text x=\"5\" y=\"95\" style=\"fill:blue;\">20 C<tspan x=\"5\" y=\"175\">15 C</tspan><tspan x=\"5\" y=\"255\">10 C</tspan><tspan x=\"5\" y=\"335\">5 C</tspan></text>");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<g fill=\"none\" stroke=\"red\" stroke-width=\"2\"><polyline points=\"");

    for (int i=0; i < UBOUND(db_graph_values); i++) {
         if (db_graph_values[i].pressure > 0) index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "%d,%0.0f ", 2*i, 8*(1030 - db_graph_values[i].pressure));
    }
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "\" /></g>");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<g fill=\"none\" stroke=\"blue\" stroke-width=\"2\"><polyline points=\"");
    for (int i=0; i < UBOUND(db_graph_values); i++) {
        if (db_graph_values[i].temperature > 0) index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "%d,%0.0f ", 2*i, 16*(25 - bme280_getDewPoint(db_graph_values[i].humidity, db_graph_values[i].temperature)));
    }
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "\" /></g></svg>");

    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<h3>GPS data</h3>");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "Sat: %02u, HDOP: %04.2f<br>Lat/Lon (Decimal): %08.6f / %08.6f<br>Lat/Lon (Deg MM.MM): %d %05.2f / %d %05.2f<br>Alt: %4.0f<br>Course: %3.0f<br>Speed: %2.0f<br>",
            gps_getSatellites(),
            gps_getHDOP(),
            gps_getLat(),
            gps_getLon(),
            int(floor(gps_getLat())), (gps_getLat() - floor(gps_getLat()))*60,
            int(floor(gps_getLon())), (gps_getLon() - floor(gps_getLon()))*60,
            gps_getAltitude(),
            gps_getCourse(),
            gps_getSpeed() );

    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<h2>Current Config</h2>");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<form action=\"/configsave\" method=\"post\">");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<label for=\"%s\">%s:</label><input id=\"%s\" name=\"%s\" type=\"number\" value=\"%0.0f\"><br>", FORM_ALTITUDE, CONFIG.AltitudeFile, FORM_ALTITUDE, FORM_ALTITUDE, CONFIG.Altitude);
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<label for=\"%s\">%s:</label><input id=\"%s\" name=\"%s\" type=\"number\" value=\"%d\"><br>", FORM_TZOFFSET, CONFIG.TZOffsetFile, FORM_TZOFFSET, FORM_TZOFFSET, CONFIG.TZOffset);
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<label for=\"%s\">%s:</label><input id=\"%s\" name=\"%s\" type=\"number\" step=\"0.1\" value=\"%0.1f\"><br>", FORM_TEMPOFFSET, CONFIG.TemperatureOffsetFile, FORM_TEMPOFFSET, FORM_TEMPOFFSET, CONFIG.TemperatureOffset);
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "<button type=\"submit\">Save</button>");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "</form><br><form action=\"/esprestart\" method=\"post\"><button type=\"submit\">Restart</button></form><br>");
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "Heap: total %u / free %u / min %u / max blocksize %u<br>", ESP.getHeapSize(), ESP.getFreeHeap(), esp_get_minimum_free_heap_size(), ESP.getMaxAllocHeap() );
    index += snprintf(index_html+index, INDEX_HTML_LEN-index-1, "Content length: %d / %d</body></html>", index+35, INDEX_HTML_LEN);
    
    DEBUG_PRINT(index_html);
    DEBUG_PRINT("****(complete)****");
}

void webserver_initialize() {
    
    create_index_html();

    webserver.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
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
                if ( strcmp(p->name().c_str(), FORM_TEMPOFFSET) == 0) {
                    DEBUG_PRINT("Set Temperature offset");
                    CONFIG.TemperatureOffset = p->value().toFloat();
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

    webserver.on("/esprestart", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />");
        response->printf("<title>Webpage at %s</title>", request->url().c_str());
        response->print("<style> body {font: normal 10px Verdana, Arial, sans-serif;} </style>");
        response->print("</head><body>Restarting ESP</body></html>");

        request->send(response);

        ESP.restart();
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