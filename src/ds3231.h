#ifndef DS3231_H
#define DS3231_H

#include "tools.h"

#include <RtcDS3231.h>
#include <Wire.h>

void ds3231_initialize();

void ds3231_setDateTime(RtcDateTime now);
void ds3231_setDateTime(uint64_t tst);

bool ds3231_IsValid();

uint64_t ds3231_getEpoch();
uint8_t ds3231_getHour();
uint8_t ds3231_getMinute();
uint8_t ds3231_getSecond();

uint8_t ds3231_getDayOfMonth();
uint8_t ds3231_getMonth();
uint16_t ds3231_getYear();

tm * ds3231_getGMNow();
tm * ds3231_getLocalNow();

#endif