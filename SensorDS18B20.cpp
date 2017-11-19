/**
 * @brief Main Thermostat class
 *          Gives access to boiler state, desired temperature, real temperature & humidity,
 *          and implements ZWave getter & setter functions
 * 
 */

#include "SensorDS18B20.h"

OneWire ow(PIN_TEMP_SENSOR);
DS18B20Sensor ds18b20(&ow);

byte sensor_roms[ROM_SIZE*MAX_SENSOR];
#define ROM_DATA(index) (&sensor_roms[index*ROM_SIZE])

/**
 * @brief Constructor. Does required initialisations and turns the boiler off
 * 
 */
SensorClass::SensorClass() {
    ds18b20.findAllSensors(sensor_roms);
}

/**
 * @brief Read the sensor and store the result to buffer
 * 
 */
void SensorClass::ReadSensor() {
    byte forceRead = 0;
    realTemp = ds18b20.getTemperature(ROM_DATA(0));
    Serial.println(realTemp);
}
/**
 * @brief Get the current room temperature
 * 
 * @return float   the room temperature
 */
float SensorClass::GetTemperature() {
    return realTemp;
}

/**
 * @brief Get the current room humidity
 * 
 * @return float   the room humidity
 */
float SensorClass::GetHumidity() {
    return 0;
}
