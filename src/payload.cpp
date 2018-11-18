
#include "globals.h"
#include "payload.h"

PayloadConvert::PayloadConvert(uint8_t size) {
  buffer = (uint8_t *)malloc(size);
  cursor = 0;
}

PayloadConvert::~PayloadConvert(void) { free(buffer); }

void PayloadConvert::reset(void) { cursor = 0; }

uint8_t PayloadConvert::getSize(void) { return cursor; }

uint8_t *PayloadConvert::getBuffer(void) { return buffer; }

/* ---------------- plain format without special encoding ---------- */

#if PAYLOAD_ENCODER == 1

void PayloadConvert::addCount(uint16_t value1, uint16_t value2) {
  buffer[cursor++] = highByte(value1);
  buffer[cursor++] = lowByte(value1);
  buffer[cursor++] = highByte(value2);
  buffer[cursor++] = lowByte(value2);
}

void PayloadConvert::addAlarm(int8_t rssi, uint8_t msg) {
  buffer[cursor++] = rssi;
  buffer[cursor++] = msg;
}

void PayloadConvert::addConfig(configData_t value) {
  buffer[cursor++] = value.lorasf;
  buffer[cursor++] = value.txpower;
  buffer[cursor++] = value.adrmode;
  buffer[cursor++] = value.screensaver;
  buffer[cursor++] = value.screenon;
  buffer[cursor++] = value.countermode;
  buffer[cursor++] = highByte(value.rssilimit);
  buffer[cursor++] = lowByte(value.rssilimit);
  buffer[cursor++] = value.sendcycle;
  buffer[cursor++] = value.wifichancycle;
  buffer[cursor++] = value.blescantime;
  buffer[cursor++] = value.blescan;
  buffer[cursor++] = value.wifiant;
  buffer[cursor++] = value.vendorfilter;
  buffer[cursor++] = value.rgblum;
  buffer[cursor++] = value.gpsmode;
  buffer[cursor++] = value.monitormode;
  memcpy(buffer + cursor, value.version, 10);
  cursor += 10;
}

void PayloadConvert::addStatus(uint16_t voltage, uint64_t uptime, float cputemp,
                               uint32_t mem, uint8_t reset1, uint8_t reset2) {

  buffer[cursor++] = highByte(voltage);
  buffer[cursor++] = lowByte(voltage);
  buffer[cursor++] = (byte)((uptime & 0xFF00000000000000) >> 56);
  buffer[cursor++] = (byte)((uptime & 0x00FF000000000000) >> 48);
  buffer[cursor++] = (byte)((uptime & 0x0000FF0000000000) >> 40);
  buffer[cursor++] = (byte)((uptime & 0x000000FF00000000) >> 32);
  buffer[cursor++] = (byte)((uptime & 0x00000000FF000000) >> 24);
  buffer[cursor++] = (byte)((uptime & 0x0000000000FF0000) >> 16);
  buffer[cursor++] = (byte)((uptime & 0x000000000000FF00) >> 8);
  buffer[cursor++] = (byte)((uptime & 0x00000000000000FF));
  buffer[cursor++] = (byte)(cputemp);
  buffer[cursor++] = (byte)((mem & 0xFF000000) >> 24);
  buffer[cursor++] = (byte)((mem & 0x00FF0000) >> 16);
  buffer[cursor++] = (byte)((mem & 0x0000FF00) >> 8);
  buffer[cursor++] = (byte)((mem & 0x000000FF));
  buffer[cursor++] = (byte)(reset1);
  buffer[cursor++] = (byte)(reset2);
}

void PayloadConvert::addGPS(gpsStatus_t value) {
#ifdef HAS_GPS
  buffer[cursor++] = (byte)((value.latitude & 0xFF000000) >> 24);
  buffer[cursor++] = (byte)((value.latitude & 0x00FF0000) >> 16);
  buffer[cursor++] = (byte)((value.latitude & 0x0000FF00) >> 8);
  buffer[cursor++] = (byte)((value.latitude & 0x000000FF));
  buffer[cursor++] = (byte)((value.longitude & 0xFF000000) >> 24);
  buffer[cursor++] = (byte)((value.longitude & 0x00FF0000) >> 16);
  buffer[cursor++] = (byte)((value.longitude & 0x0000FF00) >> 8);
  buffer[cursor++] = (byte)((value.longitude & 0x000000FF));
  buffer[cursor++] = value.satellites;
  buffer[cursor++] = highByte(value.hdop);
  buffer[cursor++] = lowByte(value.hdop);
  buffer[cursor++] = highByte(value.altitude);
  buffer[cursor++] = lowByte(value.altitude);
#endif
}

void PayloadConvert::addBME(bmeStatus_t value) {
#ifdef HAS_BME
  int16_t temperature = (int16_t)(value.temperature); // float -> int
  uint16_t humidity = (uint16_t)(value.humidity);     // float -> int
  int16_t altitude = (int16_t)(value.altitude);      // float -> int
  buffer[cursor++] = highByte(temperature);
  buffer[cursor++] = lowByte(temperature);
  buffer[cursor++] = highByte(value.pressure);
  buffer[cursor++] = lowByte(value.pressure);
  buffer[cursor++] = highByte(humidity);
  buffer[cursor++] = lowByte(humidity);
  buffer[cursor++] = highByte(value.gas_resistance);
  buffer[cursor++] = lowByte(value.gas_resistance);
  buffer[cursor++] = highByte(altitude);
  buffer[cursor++] = lowByte(altitude);
#endif
}

void PayloadConvert::addButton(uint8_t value) {
#ifdef HAS_BUTTON
  buffer[cursor++] = value;
#endif
}

/* ---------------- packed format with LoRa serialization Encoder ----------
 */
// derived from
// https://github.com/thesolarnomad/lora-serialization/blob/master/src/LoraEncoder.cpp

#elif PAYLOAD_ENCODER == 2

void PayloadConvert::addCount(uint16_t value1, uint16_t value2) {
  writeUint16(value1);
  writeUint16(value2);
}

void PayloadConvert::addAlarm(int8_t rssi, uint8_t msg) {
  writeUint8(rssi);
  writeUint8(msg);
}

void PayloadConvert::addConfig(configData_t value) {
  writeUint8(value.lorasf);
  writeUint8(value.txpower);
  writeUint16(value.rssilimit);
  writeUint8(value.sendcycle);
  writeUint8(value.wifichancycle);
  writeUint8(value.blescantime);
  writeUint8(value.rgblum);
  writeBitmap(value.adrmode ? true : false, value.screensaver ? true : false,
              value.screenon ? true : false, value.countermode ? true : false,
              value.blescan ? true : false, value.wifiant ? true : false,
              value.vendorfilter ? true : false, value.gpsmode ? true : false);
  writeVersion(value.version);
}

void PayloadConvert::addStatus(uint16_t voltage, uint64_t uptime, float cputemp,
                               uint32_t mem, uint8_t reset1, uint8_t reset2) {
  writeUint16(voltage);
  writeUptime(uptime);
  writeUint8((byte)cputemp);
  writeUint32(mem);
  writeUint8(reset1);
  writeUint8(reset2);
}

void PayloadConvert::addGPS(gpsStatus_t value) {
#ifdef HAS_GPS
  writeLatLng(value.latitude, value.longitude);
  writeUint8(value.satellites);
  writeUint16(value.hdop);
  writeUint16(value.altitude);
#endif
}

void PayloadConvert::addBME(bmeStatus_t value) {
#ifdef HAS_BME
  writeTemperature(value.temperature);
  writeUint16(value.pressure);
  writeHumidity(value.humidity);
  writeUint16(value.gas_resistance);
  writeTemperature(value.altitude);
#endif
}

void PayloadConvert::addButton(uint8_t value) {
#ifdef HAS_BUTTON
  writeUint8(value);
#endif
}

void PayloadConvert::intToBytes(uint8_t pos, int32_t i, uint8_t byteSize) {
  for (uint8_t x = 0; x < byteSize; x++) {
    buffer[x + pos] = (byte)(i >> (x * 8));
  }
  cursor += byteSize;
}

void PayloadConvert::writeUptime(uint64_t uptime) {
  intToBytes(cursor, uptime, 8);
}

void PayloadConvert::writeVersion(char *version) {
  memcpy(buffer + cursor, version, 10);
  cursor += 10;
}

void PayloadConvert::writeLatLng(double latitude, double longitude) {
  intToBytes(cursor, latitude, 4);
  intToBytes(cursor, longitude, 4);
}

void PayloadConvert::writeUint32(uint32_t i) { intToBytes(cursor, i, 4); }

void PayloadConvert::writeUint16(uint16_t i) { intToBytes(cursor, i, 2); }

void PayloadConvert::writeUint8(uint8_t i) { intToBytes(cursor, i, 1); }

void PayloadConvert::writeHumidity(float humidity) {
  int16_t h = (int16_t)(humidity * 100);
  intToBytes(cursor, h, 2);
}

/**
 * Uses a 16bit two's complement with two decimals, so the range is
 * -327.68 to +327.67 degrees
 */
void PayloadConvert::writeTemperature(float temperature) {
  int16_t t = (int16_t)(temperature * 100);
  if (temperature < 0) {
    t = ~-t;
    t = t + 1;
  }
  buffer[cursor++] = (byte)((t >> 8) & 0xFF);
  buffer[cursor++] = (byte)t & 0xFF;
}

void PayloadConvert::writeBitmap(bool a, bool b, bool c, bool d, bool e, bool f,
                                 bool g, bool h) {
  uint8_t bitmap = 0;
  // LSB first
  bitmap |= (a & 1) << 7;
  bitmap |= (b & 1) << 6;
  bitmap |= (c & 1) << 5;
  bitmap |= (d & 1) << 4;
  bitmap |= (e & 1) << 3;
  bitmap |= (f & 1) << 2;
  bitmap |= (g & 1) << 1;
  bitmap |= (h & 1) << 0;
  writeUint8(bitmap);
}

/* ---------------- Cayenne LPP 2.0 format ---------- */
// see specs http://community.mydevices.com/t/cayenne-lpp-2-0/7510
// PAYLOAD_ENCODER == 3 -> Dynamic Sensor Payload, using channels -> FPort 1
// PAYLOAD_ENCODER == 4 -> Packed Sensor Payload, not using channels -> FPort 2

#elif (PAYLOAD_ENCODER == 3 || PAYLOAD_ENCODER == 4)

void PayloadConvert::addCount(uint16_t value1, uint16_t value2) {
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_COUNT_WIFI_CHANNEL;
#endif
  buffer[cursor++] =
      LPP_LUMINOSITY; // workaround since cayenne has no data type meter
  buffer[cursor++] = highByte(value1);
  buffer[cursor++] = lowByte(value1);
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_COUNT_BLE_CHANNEL;
#endif
  buffer[cursor++] =
      LPP_LUMINOSITY; // workaround since cayenne has no data type meter
  buffer[cursor++] = highByte(value2);
  buffer[cursor++] = lowByte(value2);
}

void PayloadConvert::addAlarm(int8_t rssi, uint8_t msg) {
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_ALARM_CHANNEL;
#endif
  buffer[cursor++] = LPP_PRESENCE;
  buffer[cursor++] = msg;
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_MSG_CHANNEL;
#endif
  buffer[cursor++] = LPP_ANALOG_INPUT;
  buffer[cursor++] = rssi;
}

void PayloadConvert::addConfig(configData_t value) {
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_ADR_CHANNEL;
#endif
  buffer[cursor++] = LPP_DIGITAL_INPUT;
  buffer[cursor++] = value.adrmode;
}

void PayloadConvert::addStatus(uint16_t voltage, uint64_t uptime, float celsius,
                               uint32_t mem, uint8_t reset1, uint8_t reset2) {
  uint16_t temp = celsius * 10;
  uint16_t volt = voltage / 10;
#ifdef HAS_BATTERY_PROBE
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_BATT_CHANNEL;
#endif
  buffer[cursor++] = LPP_ANALOG_INPUT;
  buffer[cursor++] = highByte(volt);
  buffer[cursor++] = lowByte(volt);
#endif // HAS_BATTERY_PROBE

#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_TEMPERATURE_CHANNEL;
#endif
  buffer[cursor++] = LPP_TEMPERATURE;
  buffer[cursor++] = highByte(temp);
  buffer[cursor++] = lowByte(temp);
}

void PayloadConvert::addGPS(gpsStatus_t value) {
#ifdef HAS_GPS
  int32_t lat = value.latitude / 100;
  int32_t lon = value.longitude / 100;
  int32_t alt = value.altitude * 100;
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_GPS_CHANNEL;
#endif
  buffer[cursor++] = LPP_GPS;
  buffer[cursor++] = (byte)((lat & 0xFF0000) >> 16);
  buffer[cursor++] = (byte)((lat & 0x00FF00) >> 8);
  buffer[cursor++] = (byte)((lat & 0x0000FF));
  buffer[cursor++] = (byte)((lon & 0xFF0000) >> 16);
  buffer[cursor++] = (byte)((lon & 0x00FF00) >> 8);
  buffer[cursor++] = (byte)(lon & 0x0000FF);
  buffer[cursor++] = (byte)((alt & 0xFF0000) >> 16);
  buffer[cursor++] = (byte)((alt & 0x00FF00) >> 8);
  buffer[cursor++] = (byte)(alt & 0x0000FF);
#endif // HAS_GPS
}

void PayloadConvert::addBME(bmeStatus_t value) {
#ifdef HAS_BME

  // data value conversions to meet cayenne data type definition
  // 0.1°C per bit => -3276,7 .. +3276,7 °C
  int16_t temperature = (int16_t)(value.temperature * 10.0);
  // 0.1 hPa per bit => 0 .. 6553,6 hPa
  uint16_t pressure = value.pressure * 10;
  // 0.5% per bit => 0 .. 128 %C
  uint8_t humidity = (uint8_t)(value.humidity * 2.0);
  // 0.01 Ohm per bit => 0 .. 655,36 Ohm
  uint16_t gas = value.gas_resistance * 100;

#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_TEMPERATURE_CHANNEL;
#endif
  buffer[cursor++] = LPP_TEMPERATURE; // 2 bytes 0.1 °C Signed MSB
  buffer[cursor++] = highByte(temperature);
  buffer[cursor++] = lowByte(temperature);
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_BAROMETER_CHANNEL;
#endif
  buffer[cursor++] = LPP_BAROMETER; // 2 bytes 0.1 hPa Unsigned MSB
  buffer[cursor++] = highByte(pressure);
  buffer[cursor++] = lowByte(pressure);
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_HUMIDITY_CHANNEL;
#endif
  buffer[cursor++] = LPP_HUMIDITY; // 1 byte 0.5 % Unsigned
  buffer[cursor++] = humidity;
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_GAS_CHANNEL;
#endif
  buffer[cursor++] = LPP_ANALOG_INPUT; // 2 bytes 0.01 Signed
  buffer[cursor++] = highByte(gas);
  buffer[cursor++] = lowByte(gas);
#endif // HAS_BME
}

void PayloadConvert::addButton(uint8_t value) {
#ifdef HAS_BUTTON
#if (PAYLOAD_ENCODER == 3)
  buffer[cursor++] = LPP_BUTTON_CHANNEL;
#endif
  buffer[cursor++] = LPP_DIGITAL_INPUT;
  buffer[cursor++] = value;
#endif // HAS_BUTTON
}

#else
#error "No valid payload converter defined"
#endif