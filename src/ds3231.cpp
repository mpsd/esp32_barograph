#include <Arduino.h>
#include "ds3231.h"

RtcDS3231<TwoWire> Rtc(Wire);

void ds3231_initialize() {

  DEBUG_PRINT("initialize DS3231");
  // example DS3231 Simple
  Rtc.Begin();
  
  if ( !Rtc.IsDateTimeValid() ) 
  {
      if (Rtc.LastError() != 0)
      {
          // we have a communications error
          // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
          // what the number means
          DEBUG_PRINT("RTC communications error");
          // DEBUG_PRINT(Rtc.LastError());
      }
      else
      {
          // Common Cuases:
          //    1) first time you ran and the device wasn't running yet
          //    2) the battery on the device is low or even missing

          DEBUG_PRINT("RTC lost confidence in the DateTime!");

          // following line sets the RTC to the date & time this sketch was compiled
          // it will also reset the valid flag internally unless the Rtc device is
          // having an issue
      }
  }

  if ( !Rtc.GetIsRunning() )
  {
      DEBUG_PRINT("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  char datestring[40];

  snprintf_P(datestring, 
    UBOUND(datestring),
    PSTR("Current Time: %02u/%02u/%04u %02u:%02u:%02u"),
    now.Month(),
    now.Day(),
    now.Year(),
    now.Hour(),
    now.Minute(),
    now.Second() );
  DEBUG_PRINT(datestring);

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 

}

void ds3231_setDateTime(RtcDateTime now) {
    Rtc.SetDateTime( now );

    RtcDateTime dt = Rtc.GetDateTime();
    char datestring[40];

    snprintf_P(datestring, 
            UBOUND(datestring),
            PSTR("Set to: %02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    DEBUG_PRINT(datestring);
}

void ds3231_setDateTime(uint64_t tst) {
    RtcDateTime now = RtcDateTime(  (uint32_t)(tst - 946684800ULL) );
    ds3231_setDateTime( now );
}

bool ds3231_IsValid() {
    return Rtc.IsDateTimeValid();
}

uint64_t ds3231_getEpoch() {
    RtcDateTime now = Rtc.GetDateTime();
    return now.Epoch64Time();
}

uint8_t ds3231_getHour() {
    return (uint8_t)ds3231_getLocalNow()->tm_hour;
}

uint8_t ds3231_getMinute() {
    return (uint8_t)ds3231_getLocalNow()->tm_min;
}

uint8_t ds3231_getSecond() {
    return (uint8_t)ds3231_getLocalNow()->tm_sec;
}

uint8_t ds3231_getDayOfMonth() {
    return (uint8_t)ds3231_getLocalNow()->tm_mday;
}

uint8_t ds3231_getMonth() {
    return (uint8_t)ds3231_getLocalNow()->tm_mon + 1;
}

uint16_t ds3231_getYear() {
    return (uint16_t)ds3231_getLocalNow()->tm_year + 1900;
}

/* private functions, epoch style tm structures (after 01/01/1900) */
tm * ds3231_getLocalNow() {
    time_t rawtime = (time_t)ds3231_getEpoch() + (time_t)(CONFIG.TZOffset * 3600);
    return gmtime( &rawtime );
}

char * ds3231_getTimeFormatted(uint64_t tst) {
    char * retval = new char[20];
    time_t rawtime = (time_t)tst  + (time_t)(CONFIG.TZOffset * 3600);
    tm * structime;

    structime = gmtime( &rawtime );

    sprintf(retval, "%02d:%02d.%02d", structime->tm_hour, structime->tm_min, structime->tm_sec);

    return retval;
}
