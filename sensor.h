#ifndef SENSOR_H
#define SENSOR_H

#define TEMP_DS18B20
#undef  TEMP_DHT

#include <Arduino.h>
#include "pinout.h"

#ifdef TEMP_DHT
#include <ZUNO_DHT.h>
#endif
#ifdef TEMP_DS18B20
#include <ZUNO_OneWire.h>
#include <ZUNO_DS18B20.h>
#define ROM_SIZE 8
#define MAX_SENSOR 1
#endif

class SensorClass {
public:
    SensorClass();
    float GetTemperature();
    float GetHumidity();
    void ReadSensor();

protected:
    float realTemp;
    float realHum;
};

#endif // SENSOR_H
