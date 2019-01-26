#ifdef HAS_RTC

#include "rtctime.h"

// Local logging tag
static const char TAG[] = "main";

RtcDS3231<TwoWire> Rtc(Wire);

// initialize RTC
int rtc_init(void) {

  // return = 0 -> error / return = 1 -> success

  // block i2c bus access
  if (I2C_MUTEX_LOCK()) {

    Wire.begin(HAS_RTC);
    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

    if (!Rtc.IsDateTimeValid()) {
      ESP_LOGW(TAG,
               "RTC has no valid RTC date/time, setting to compilation date");
      Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning()) {
      ESP_LOGI(TAG, "RTC not running, starting now");
      Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();

    if (now < compiled) {
      ESP_LOGI(TAG, "RTC date/time is older than compilation date, updating)");
      Rtc.SetDateTime(compiled);
    }

    // configure RTC chip
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  } else {
    ESP_LOGE(TAG, "I2c bus busy - RTC initialization error");
    goto error;
  }

  I2C_MUTEX_UNLOCK(); // release i2c bus access
  ESP_LOGI(TAG, "RTC initialized");
  return 1;

error:
  I2C_MUTEX_UNLOCK(); // release i2c bus access
  return 0;

} // rtc_init()

int set_rtctime(uint32_t UTCTime) {
  // return = 0 -> error / return = 1 -> success
  // block i2c bus access
  if (I2C_MUTEX_LOCK()) {
    Rtc.SetDateTime(RtcDateTime(UTCTime));
    I2C_MUTEX_UNLOCK(); // release i2c bus access
    return 1;
  }
  return 0;
} // set_rtctime()

int set_rtctime(RtcDateTime t) {
  // return = 0 -> error / return = 1 -> success
  // block i2c bus access
  if (I2C_MUTEX_LOCK()) {
    Rtc.SetDateTime(t);
    I2C_MUTEX_UNLOCK(); // release i2c bus access
    return 1;
  }
  return 0;
} // set_rtctime()

time_t get_rtctime(void) {
  // never call now() in this function, would cause recursion!
  time_t tt = 0;
  // block i2c bus access
  if (I2C_MUTEX_LOCK()) {
    if (!Rtc.IsDateTimeValid()) {
      ESP_LOGW(TAG, "RTC lost confidence in the DateTime");
    } else {
      RtcDateTime t = Rtc.GetDateTime();
      tt = t.Epoch32Time();
    }
    I2C_MUTEX_UNLOCK(); // release i2c bus access
    return tt;
  }
  return tt;
} // get_rtctime()

void sync_rtctime(void) {
  if (timeStatus() != timeSet) { // do we need time sync?
    time_t t = get_rtctime();
    if (t) { // do we have valid time by RTC?
      setTime(t);
      ESP_LOGI(TAG, "RTC has set system time to %02d/%02d/%d %02d:%02d:%02d",
               month(t), day(t), year(t), hour(t), minute(t), second(t));
    } else
      ESP_LOGE(TAG, "RTC has no confident time, not synced");
  }

#ifdef TIME_SYNC_INTERVAL_RTC
  setSyncProvider(&get_rtctime);
  setSyncInterval(TIME_SYNC_INTERVAL_RTC);
#endif

} // sync_rtctime;

float get_rtctemp(void) {
  // block i2c bus access
  if (I2C_MUTEX_LOCK()) {
    RtcTemperature temp = Rtc.GetTemperature();
    I2C_MUTEX_UNLOCK(); // release i2c bus access
    return temp.AsFloatDegC();
  } // while
  return 0;
} // get_rtc()

time_t gpsTimeSync(void) {
#ifdef HAS_GPS
  tmElements_t tm;
  tm.Second = gps.time.second();
  tm.Minute = gps.time.minute();
  tm.Hour = gps.time.hour();
  tm.Day = gps.date.day();
  tm.Month = gps.date.month();
  tm.Year = CalendarYrToTm(gps.date.year());
  return makeTime(tm);
#endif // HAS_GPS
}

#endif // HAS_RTC