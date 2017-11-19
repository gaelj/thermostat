#ifndef SENSOR_DS18B20_H
#define SENSOR_DS18B20_H

#include <Arduino.h>
#include <ZUNO_OneWire.h>
#include <ZUNO_DS18B20.h>

#include "pinout.h"

#define ROM_SIZE 8
#define MAX_SENSOR 1

class SensorClass {
public:
    SensorClass();
    float GetTemperature();
    float GetHumidity();
    void ReadSensor();

private:
    float realTemp;
    float realHum;
};

#endif // SENSOR_DS18B20_H
