/**
 * @brief Main Thermostat class
 *          Gives access to boiler state, desired temperature, real temperature & humidity,
 *          and implements ZWave getter & setter functions
 * 
 */

#include "sensor.h"

#ifdef TEMP_DHT

DHT DhtSensor(PIN_TEMP_SENSOR, DHT22);

/**
 * @brief Constructor. Does required initialisations and turns the boiler off
 * 
 */
SensorClass::SensorClass()
{
    DhtSensor.begin();
    Temperature = -0.2;
    Humidity = -0.2;
}

/**
 * @brief Read the sensor and store the result to buffer
 * 
 */
void SensorClass::ReadSensor()
{
    const byte force_read = 1;
    do {
        Temperature = DhtSensor.readTemperature(force_read);
    } while (Temperature == -0.1);
    do {
        Humidity = DhtSensor.readHumidity(force_read);
    } while (Humidity == -0.1);
}
#endif




#ifdef TEMP_DS18B20

OneWire ow(PIN_TEMP_SENSOR);
DS18B20Sensor ds18b20(&ow);

byte sensor_roms[DS18B20_ROM_SIZE * DS18B20_MAX_SENSOR];
#define DS18B20_ROM_DATA(index) (&sensor_roms[index * DS18B20_ROM_SIZE])

/**
 * @brief Constructor. Does required initialisations and turns the boiler off
 * 
 */
SensorClass::SensorClass()
{
    ds18b20.findAllSensors(sensor_roms);
}

/**
 * @brief Read the sensor and store the result to buffer
 * 
 */
void SensorClass::ReadSensor()
{
    Temperature = ds18b20.getTemperature(DS18B20_ROM_DATA(0));
}
#endif
