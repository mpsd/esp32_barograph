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

  // Rtc.SetDateTime( RtcDateTime(__DATE__, __TIME__) );

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
    Rtc.SetIsRunning(false);
    Rtc.SetDateTime( now );
    Rtc.SetIsRunning(true);

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

void ds3231_setDateTimeEpoch(uint64_t tst) {
    RtcDateTime now;
    now.InitWithEpoch64Time( tst );
    ds3231_setDateTime( now );
}

bool ds3231_IsValid() {
    return Rtc.IsDateTimeValid();
}

uint64_t ds3231_getEpoch() {
    RtcDateTime now = Rtc.GetDateTime();
    return (Rtc.IsDateTimeValid() ? now.Epoch64Time() : 0);
}

uint8_t ds3231_getHour() {
    RtcDateTime now = RtcDateTime( ds3231_getEpoch() - 946684800U + (uint32_t)(CONFIG.TZOffset * 3600) );
    return now.Hour();
}

uint8_t ds3231_getMinute() {
    RtcDateTime now = RtcDateTime( ds3231_getEpoch() - 946684800U + (uint32_t)(CONFIG.TZOffset * 3600) );
    return now.Minute();
}

uint8_t ds3231_getSecond() {
    RtcDateTime now = RtcDateTime( ds3231_getEpoch() - 946684800U + (uint32_t)(CONFIG.TZOffset * 3600) );
    return now.Second();
}

uint8_t ds3231_getDayOfMonth() {
    RtcDateTime now = RtcDateTime( ds3231_getEpoch() - 946684800U + (uint32_t)(CONFIG.TZOffset * 3600) );
    return now.Day();
}

uint8_t ds3231_getMonth() {
    RtcDateTime now = RtcDateTime( ds3231_getEpoch() - 946684800U + (uint32_t)(CONFIG.TZOffset * 3600) );
    return now.Month();
}

uint16_t ds3231_getYear() {
    RtcDateTime now = RtcDateTime( ds3231_getEpoch() - 946684800U + (uint32_t)(CONFIG.TZOffset * 3600) );
    return now.Year();
}

/* private function to apply timezoneoffset */
RtcDateTime ds3231_getLocalNow() {
    RtcDateTime now = RtcDateTime( ds3231_getEpoch() - 946684800U + (uint32_t)(CONFIG.TZOffset * 3600) );
    return now;
}
