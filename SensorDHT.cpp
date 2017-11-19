/**
 * @brief Main Thermostat class
 *          Gives access to boiler state, desired temperature, real temperature & humidity,
 *          and implements ZWave getter & setter functions
 * 
 */

#include "SensorDHT.h"

DHT DhtSensor(PIN_TEMP_SENSOR, DHT22);

/**
 * @brief Constructor. Does required initialisations and turns the boiler off
 * 
 */
SensorClass::SensorClass() {
    DhtSensor.begin();
}

/**
 * @brief Read the sensor and store the result to buffer
 * 
 */
void SensorClass::ReadSensor() {
    byte forceRead = 0;
    realTemp = DhtSensor.readTemperature(forceRead);
    realHum = DhtSensor.readHumidity(forceRead);
    Serial.println(realTemp);
    Serial.println(realHum);
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
    return realHum;
}
