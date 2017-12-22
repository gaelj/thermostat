--Script To Parse WeatherUnderground Multi-Value Sensor, Additionally using PWS: system from WU with a new output format
--This script assumes the output (which can be viewed in events show current state button) is like this 19.5;79;3;1019;3 (temp;humidity;null;pressure;null)
--more details at this wiki http://www.domoticz.com/wiki/Virtual_weather_devices
--
--The following need updated for your environment get the 'Idx' or 'Name' off the Device tab. By default only the Temp is 'uncommented or enabled' in this script.
local sensorwu = 'THB Potelles' --name of the sensor that gets created when you add the WU device (and that contains multiple values like temperature, humidity, barometer etc)
local idxt = 114 --idx of the virtual temperature sensor you need to change this to your own Device IDx
local idxh = 115 --idx of the virtual humidity sensor you need to change this to your own Device IDx
local idxp = 116 --idx of the virtual pressure sensor you need to change this to your own Device IDx
local idxx = 119 --idx of the thermostat's exterior temperature dimmer input (requires encoding)
--

commandArray = {}


sWeatherTemp, sWeatherHumidity, sHumFeelsLike, sWeatherPressure = otherdevices_svalues[sensorwu]:match("([^;]+);([^;]+);([^;]+);([^;]+);([^;]+)")
sWeatherTemp = tonumber(sWeatherTemp)
sWeatherHumidity = tonumber(sWeatherHumidity)
sWeatherPressure = tonumber(sWeatherPressure)
sEncodedTemp = math.floor(((sWeatherTemp + 25) * 2) + 0.5)
if sEncodedTemp < 0 then
    sEncodedTemp = 0
end
if sEncodedTemp > 99 then
     sEncodedTemp = 99
end

if devicechanged[sensorwu] then
        --parseDebug = ('WU Script Parsed Temp=' .. sWeatherTemp .. ' Humidity=' .. sWeatherHumidity .. ' Pressure=' .. sWeatherPressure)
       -- print(parseDebug)

        commandArray[1] = {['UpdateDevice'] = idxt .. '|0|' .. sWeatherTemp}
        commandArray[2] = {['UpdateDevice'] = idxh .. '|' .. tostring(sWeatherHumidity) .. '|' .. tostring(sHumFeelsLike)}
        --commandArray[3] = {['UpdateDevice'] = idxp .. '|0|' .. tostring(sWeatherPressure) .. ';' .. tostring(sWeatherPressForcast)}
end
oldLevelStr = otherdevices_svalues['Thermostat Exterior Temperature']
oldLevel = tonumber(oldLevelStr)
if oldLevel ~= sEncodedTemp then
    print('Old level: ' .. oldLevel .. ' - New level: ' .. sEncodedTemp)
    commandArray['Thermostat Exterior Temperature'] = "Set Level " .. sEncodedTemp
end
return commandArray
