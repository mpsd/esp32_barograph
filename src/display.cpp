#include <Arduino.h>
#include "display.h"

SPIClass _spiDisplay(VSPI);
GxEPD2_BW<GxEPD2_154, GxEPD2_154::HEIGHT> display(GxEPD2_154(/*CS*/ _epd_spi_cs, /*DC*/ _epd_dc, /*RST*/ _epd_rst, /*BUSY*/ _epd_busy));

void display_initialize(void) {

  _spiDisplay.begin(/*SCK*/ _epd_spi_clk,/*MISO*/ 0,/*MOSI*/ _epd_spi_mosi,/*SS*/ _epd_spi_cs);  
  display.init(115200);

}

void display_close() {
  display.powerOff();
  _spiDisplay.end();
}

void display_drawDottedLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  float_t x, fx;
  x = (x2-x1);

  if (x != 0) {
    fx = (y2-y1)/x;
    for (int16_t ix=0;ix<=x; ix++) {
      if (ix % 2 == 0) {
        display.drawPixel(x1+ix, y1+(fx*ix), color);
      }
    }
  } else {
    for (int16_t iy=y1; iy<=y2; iy++) {
      if (iy % 2 == 0) {
        display.drawPixel(x1, iy, color);
      }
    }
  }

}

void display_update(void)
{
  DEBUG_PRINT("------------(begin)-------------");
  display_initialize();
  
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

  display.setFullWindow();
  display.firstPage();

  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds("A", 0, 0, &tbx, &tby, &tbw, &tbh); // tbwidth and tbheight of one letter

  tbh += 3;

  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor( 0, tbh );
    display.printf("%02u:%02u", ds3231_getHour(), ds3231_getMinute());
    display.setCursor( 0, 2*tbh );
    display.printf("%02u/%02u", ds3231_getDayOfMonth(), ds3231_getMonth());
    display.setCursor( 0, 3*tbh + 3 );
    display.printf("0h:%4.0fhPa %+4.1f", db_hourly_values[0].pressure, db_hourly_values[0].chg_pressure);
    display.setCursor( 0, 4*tbh + 3 );
    display.printf("3h:%4.0fhPa %+4.1f", db_hourly_values[3].pressure, db_hourly_values[3].chg_pressure);
    display.setCursor( 0, 5*tbh + 3 );
    display.printf("6h:%4.0fhPa %+4.1f", db_hourly_values[6].pressure, db_hourly_values[6].chg_pressure);
    
    display.setCursor( (display.width() - 5*tbw)/2, tbh );
    display.printf("%5.2f", gps_getLat());
    display.setCursor( (display.width() - 5*tbw)/2, 2*tbh );
    display.printf("%5.2f", gps_getLon());
    
    display.setCursor( (display.width() - 3*tbw), tbh );
    display.printf("%2.0fC", db_hourly_values[0].temperature);
    display.setCursor( (display.width() - 3*tbw), 2*tbh );
    display.printf("%2.0f%%", db_hourly_values[0].humidity);

    display_drawDottedLine( 0,  80, display.width(),  80, GxEPD_BLACK); // 1030
    display_drawDottedLine( 0, 100, display.width(), 100, GxEPD_BLACK); // 1020
    display.setCursor(0, 100-2);
    display.print("1020");
    display_drawDottedLine( 0, 120, display.width(), 120, GxEPD_BLACK); // 1010
    display_drawDottedLine( 0, 140, display.width(), 140, GxEPD_BLACK); // 1000
    display.setCursor(0, 140-2);
    display.print("1000");
    display_drawDottedLine( 0, 160, display.width(), 160, GxEPD_BLACK); //  990
    display_drawDottedLine( 0, 180, display.width(), 180, GxEPD_BLACK); //  980
    display.setCursor(0, 180-2);
    display.print("980");
    
    display.setCursor( 0, display.height()-2 );
    display.print("24h");
    display_drawDottedLine( display.width()/2, 80, display.width()/2, 180, GxEPD_BLACK); //  12h
    display.setCursor( (display.width() - 5*tbw)/2, display.height()-2 );
    display.printf("%4.0fm", CONFIG.Altitude);
    display_drawDottedLine( display.width()*3/4, 80, display.width()*3/4, 180, GxEPD_BLACK); //  6h
    display_drawDottedLine( display.width()*7/8, 80, display.width()*7/8, 180, GxEPD_BLACK); //  3h
    display.setCursor( (display.width() - 2*tbw), display.height()-2 );
    display.print("0h");

    for (int i=0; i<201; i++) {
      display.drawPixel( db_pressure_graph_values[i].x , (80 + 2*(1030 - db_pressure_graph_values[i].pressure)), GxEPD_BLACK);
    }

  }
  while (display.nextPage());
  display.powerOff();

  display_close();
  DEBUG_PRINT("------------(end)-------------");
}
