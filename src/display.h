#ifndef DISPLAY_H
#define DISPLAY_H

#include "tools.h"

#include <GxEPD2.h>

// Genutztes Display 200x200 1.54inch Schwarz / Weiss E-Ink Display
/*  BUSY          D4: 4
  RST           RX2: 16
  DC            TX2: 17
  CS            D5: 5   (SS)
  CLK           D18: 18 (SCK)
  DIN           D23: 23 (MOSI)
*/
#define _epd_busy 4
#define _epd_rst 16
#define _epd_dc 17
#define _epd_spi_cs 5
#define _epd_spi_clk 18
#define _epd_spi_mosi 23

#define ENABLE_GxEPD2_GFX 0
// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
// select one and adapt to your mapping, can use full buffer size (full HEIGHT)
#include <bitmaps/Bitmaps200x200.h> // 1.54" b/w

void display_initialize(void);
void display_drawDottedLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void display_update(void);

#endif