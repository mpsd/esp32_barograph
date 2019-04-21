#include <Arduino.h>
#include "ds3231.h"

RtcDS3231<TwoWire> Rtc(Wire);

/*
	RtcTemperature temp = Rtc.GetTemperature();
	temp.Print(Serial);
	// you may also get the temperature as a float and print it
  // Serial.print(temp.AsFloatDegC());
  Serial.println("C");
  */


void ds3231_initialize() {

  DEBUG_PRINT("initialize DS3231");
  // example DS3231 Simple
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if ( !Rtc.IsDateTimeValid() ) 
  {
      if (Rtc.LastError() != 0)
      {
          // we have a communications error
          // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
          // what the number means
          DEBUG_PRINT("RTC communications error = ");
          DEBUG_PRINT(Rtc.LastError());
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

          Rtc.SetDateTime(compiled);
      }
  }

  if ( !Rtc.GetIsRunning() )
  {
      DEBUG_PRINT("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  char datestring[20];

  snprintf_P(datestring, 
    UBOUND(datestring),
    PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
    now.Month(),
    now.Day(),
    now.Year(),
    now.Hour(),
    now.Minute(),
    now.Second() );
  DEBUG_PRINT(datestring);

  if (now < compiled) 
  {
      DEBUG_PRINT("RTC is older than compile time!  (Updating DateTime)");
      Rtc.SetDateTime(compiled);
  }
  else if (now > compiled) 
  {
      DEBUG_PRINT("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled) 
  {
      DEBUG_PRINT("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 

}

void ds3231_setDateTime(RtcDateTime now) {
    Rtc.SetDateTime( now );

    RtcDateTime dt = Rtc.GetDateTime();
    char datestring[20];

    snprintf_P(datestring, 
            UBOUND(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
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

void ds3231_setDateTime(uint16_t year, uint8_t month, uint8_t dom, uint8_t hour, uint8_t minute, uint8_t second) {
    RtcDateTime now = RtcDateTime(year, month, dom, hour, minute, second);
    ds3231_setDateTime( now );
}

bool ds3231_IsValid() {
    return Rtc.IsDateTimeValid();
}

uint64_t ds3231_getEpoch() {
    RtcDateTime now = Rtc.GetDateTime();
    return now.Epoch64Time();
}

uint8_t ds3231_Hour() {
    RtcDateTime now = Rtc.GetDateTime();
    return now.Hour();
}

uint8_t ds3231_Minute() {
    RtcDateTime now = Rtc.GetDateTime();
    return now.Minute();
}

uint8_t ds3231_DayOfMonth() {
    RtcDateTime now = Rtc.GetDateTime();
    return now.Day();
}

uint8_t ds3231_Month() {
    RtcDateTime now = Rtc.GetDateTime();
    return now.Month();
}