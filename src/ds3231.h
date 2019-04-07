#ifndef DS3231_H
#define DS3231_H

#include "tools.h"

#include <RtcDS3231.h>
// builtin libraries
#include <Wire.h>

void ds3231_initialize();

void ds3231_setDateTime(RtcDateTime now);
void ds3231_setDateTime(uint64_t tst);

bool ds3231_IsValid();

uint64_t ds3231_getEpoch();
uint8_t ds3231_Hour();
uint8_t ds3231_Minute();
uint8_t ds3231_DayOfMonth();
uint8_t ds3231_Month();

#endif