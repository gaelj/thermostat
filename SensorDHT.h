#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include <Arduino.h>
#include <ZUNO_DHT.h>

#include "pinout.h"

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

#endif // SENSOR_DHT_H
