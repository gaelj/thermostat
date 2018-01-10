--
-- Domoticz passes information to scripts through a number of global tables
--
-- otherdevices, otherdevices_lastupdate and otherdevices_svalues are arrays for all devices: 
--   otherdevices['yourotherdevicename'] = "On"
--   otherdevices_lastupdate['yourotherdevicename'] = "2015-12-27 14:26:40"
--   otherdevices_svalues['yourotherthermometer'] = string of svalues
--
-- uservariables and uservariables_lastupdate are arrays for all user variables: 
--   uservariables['yourvariablename'] = 'Test Value'
--   uservariables_lastupdate['yourvariablename'] = '2015-12-27 11:19:22'
--
-- other useful details are contained in the timeofday table
--   timeofday['Nighttime'] = true or false
--   timeofday['SunriseInMinutes'] = number
--   timeofday['Daytime'] = true or false
--   timeofday['SunsetInMinutes'] = number
--   globalvariables['Security'] = 'Disarmed', 'Armed Home' or 'Armed Away'
--
-- To see examples of commands see: http://www.domoticz.com/wiki/LUA_commands#General
-- To get a list of available values see: http://www.domoticz.com/wiki/LUA_commands#Function_to_dump_all_variables_supplied_to_the_script
--
-- Based on your logic, fill the commandArray with device commands. Device name is case sensitive. 
--


-- Handles communication with the custom Zwave thermostat


commandArray = {}

local sensorwu = 'THB Potelles' 

currentMode = 0
boilerValue = nil
extTemp = nil
extHum = nil
extPres = nil


-- Radiator setpoints
RadiatorTempSensors = {['Temp vanne bureau'] = 0, ['Temp vanne palier'] = 0, ['Temp vanne chambre'] = 0, ['Temp salon canape'] = 0, ['Temp salon tele'] = 0, ['Temp salon rue'] = 0}

ModeValues = {["Off"] = 0, ["Frost"] = 1, ["Absent"] = 2, ["Night"] = 3, ["Day"] = 4, ["Comfort"] = 5, ["DayFloor1"] = 6, ["ComfortFloor1"] = 7, ["DayFloor2"] = 8, ["ComfortFloor2"] = 9 }
ModeStrings = {[0] = "Off", [1] = "Frost", [2] = "Absent", [3] = "Night", [4] = "Day", [5] = "Comfort", [6] = "DayFloor1", [7] = "ComfortFloor1", [8] = "DayFloor2", [9] = "ComfortFloor2" }

RadiatorSetPoints1 = { ["Name"] = 'Setpoint bureau etage', ["Sensor"] = 'Temp vanne bureau',  ["id"] = 96,  ["Temperatures"] = { ["Frost"] = 5,  ["Absent"] = 13,  ["Night"] = 15,  ["Day"] = 20,    ["Comfort"] = 22 }}
RadiatorSetPoints2 = { ["Name"] = 'Setpoint palier',       ["Sensor"] = 'Temp vanne palier',  ["id"] = 23,  ["Temperatures"] = { ["Frost"] = 5,  ["Absent"] = 13,  ["Night"] = 15,  ["Day"] = 17.5,  ["Comfort"] = 18 }}
RadiatorSetPoints3 = { ["Name"] = 'Setpoint chambre',      ["Sensor"] = 'Temp vanne chambre', ["id"] = 20,  ["Temperatures"] = { ["Frost"] = 5,  ["Absent"] = 13,  ["Night"] = 16,  ["Day"] = 16,    ["Comfort"] = 16 }}
RadiatorSetPoints4 = { ["Name"] = 'Setpoint salon canape', ["Sensor"] = 'Temp salon canape',  ["id"] = 276, ["Temperatures"] = { ["Frost"] = 5,  ["Absent"] = 13,  ["Night"] = 15,  ["Day"] = 19,    ["Comfort"] = 21 }}
RadiatorSetPoints5 = { ["Name"] = 'Setpoint salon tele',   ["Sensor"] = 'Temp salon tele',    ["id"] = 279, ["Temperatures"] = { ["Frost"] = 5,  ["Absent"] = 13,  ["Night"] = 15,  ["Day"] = 19,    ["Comfort"] = 21 }}
RadiatorSetPoints6 = { ["Name"] = 'Setpoint salon rue',    ["Sensor"] = 'Temp salon rue',     ["id"] = 282, ["Temperatures"] = { ["Frost"] = 5,  ["Absent"] = 13,  ["Night"] = 14,  ["Day"] = 17,    ["Comfort"] = 19 }}

RadiatorSetPoints = { RadiatorSetPoints1, RadiatorSetPoints2, RadiatorSetPoints3, RadiatorSetPoints4, RadiatorSetPoints5, RadiatorSetPoints6 }

for i, r in ipairs(RadiatorSetPoints) do
    down = i > 3
    if down then
        r["Temperatures"]["DayFloor1"] = r["Temperatures"]["Day"]
        r["Temperatures"]["ComfortFloor1"] = r["Temperatures"]["Comfort"]
        r["Temperatures"]["DayFloor2"] = r["Temperatures"]["Night"]
        r["Temperatures"]["ComfortFloor2"] = r["Temperatures"]["Night"]
    else
        r["Temperatures"]["DayFloor1"] = r["Temperatures"]["Night"]
        r["Temperatures"]["ComfortFloor1"] = r["Temperatures"]["Night"]
        r["Temperatures"]["DayFloor2"] = r["Temperatures"]["Day"]
        r["Temperatures"]["ComfortFloor2"] = r["Temperatures"]["Comfort"]
    end
end

function GetSliderValue(valueRaw)
    local valueStr = ''
    local value = 0
    if valueRaw == "On" then
        value = 100
    elseif valueRaw == "Off" then
        value = 0
    else
        value = tonumber(valueStr)
        if value == nil then
            valueStr = string.gsub(string.sub(valueRaw, 0, -2), "Set Level:", "")
            valueStr = string.gsub(valueStr, "%s", "")
            value = tonumber(valueStr)
        end
    end
    return value
end

function SetThermostatMode(newMode)
    print("newMode " .. newMode)
    print("currentMode " .. currentMode)
    if (newMode > 0 and currentMode ~= newMode) then
        print("The thermostat mode changed to " .. newMode .. ": " .. ModeStrings[newMode])
        commandArray['Thermostat Setpoint'] = 'Set Level ' .. (newMode * 10)
    end
end

function SetRadiators(modeId)
    if modeId > 0 then
        print("Sending new setpoints to radiators")
        for i, r in ipairs(RadiatorSetPoints) do
            commandArray[#commandArray + 1] = {['UpdateDevice']=r["id"] .. '|0|' .. tostring(r["Temperatures"][ModeStrings[modeId]])}
        end
    end
end

function EnforceMinMax(val)
    if val < 0 then
        val = 0
    elseif val > 99 then
        val = 99
    end
    return val
end

-- Get current values
for deviceName, deviceValue in pairs(otherdevices) do
    if (deviceName == 'Thermostat Setpoint') then
        currentMode = ModeValues[deviceValue]
        
    elseif (deviceName == sensorwu) then
        sWeatherTemp, sWeatherHumidity, sHumFeelsLike, sWeatherPressure = deviceValue:match("([^;]+);([^;]+);([^;]+);([^;]+);([^;]+)")
        extTemp = EnforceMinMax(math.floor((sWeatherTemp * 2) + 50))
        extHum = EnforceMinMax(math.floor(sWeatherHumidity))
        extPres = EnforceMinMax(math.floor(sWeatherPressure - 950))
    
    elseif RadiatorTempSensors[deviceName] ~= nil then
        RadiatorTempSensors[deviceName] = deviceValue
    elseif deviceName == "Z_Boiler" then
        boilerValue = GetSliderValue(deviceValue)
    end
end

-- Hysteresis
boilerNeeded = 0
hysteresis = 0.5
for i, r in ipairs(RadiatorSetPoints) do
    setPoint = tonumber(r["Temperatures"][ModeStrings[currentMode]])
    temperature = tonumber(RadiatorTempSensors[r["Sensor"]])
    if uservariables['IsInHeatingCycle'] == 0 and (temperature < setPoint - hysteresis) then
        boilerNeeded = 1
    elseif uservariables['IsInHeatingCycle'] == 1 and (temperature < setPoint) then
        boilerNeeded = 1
    end
end
--print('boilerNeeded: ' .. tostring(boilerNeeded) .. ' boilerValue: ' .. boilerValue .. ' IsInHeatingCycle: ' .. tostring(uservariables['IsInHeatingCycle']))
if boilerNeeded == 1 and boilerValue < 99 then
    print("$$$$$$$$$$$$ BOILER ON REQ")
    commandArray["Z_Boiler"] = "Set Level On"
    uservariables['IsInHeatingCycle'] = 1
elseif boilerNeeded == 0 and boilerValue > 0 then
    print("$$$$$$$$$$$$ BOILER OFF REQ")
    commandArray["Z_Boiler"] = "Set Level Off"
    uservariables['IsInHeatingCycle'] = 0
end

-- Apply values to sliders if needed
if extTemp ~= nil then
    for deviceName, deviceValue in pairs(otherdevices) do
        if deviceName == "Z_ExteriorTemperature" then
            if GetSliderValue(deviceValue) ~= extTemp then
                commandArray[deviceName] = "Set Level " .. extTemp
            end
            
        elseif deviceName == "Z_ExteriorHumidity" then
            if GetSliderValue(deviceValue) ~= extHum then
                commandArray[deviceName] = "Set Level " .. extHum
            end
            
        elseif deviceName == "Z_ExteriorPressure" then
            if GetSliderValue(deviceValue) ~= extPres then
                commandArray[deviceName] = "Set Level " .. extPres
            end
        end
    end
end

-- Loop through all the changed devices sent by the trigger
if devicechanged ~= nil then
    for deviceName, deviceValue in pairs(devicechanged) do
        --print("Devices changed: " .. deviceName .. " - new value: " .. deviceValue)

        if (deviceName == 'Thermostat Setpoint') then
            print("Thermostat deviceValue:" .. deviceValue)
            newLevel = ModeValues[deviceValue]
            print("Thermostat Setpoint newLevel:" .. newLevel)
            commandArray['Z_Mode'] = 'Set Level ' .. newLevel
            SetRadiators(newLevel)
            
        elseif deviceName == "sensorwu" then
            commandArray['Z_ExteriorTemperature'] = "Set Level " .. extTemp
            commandArray['Z_ExteriorHumidity'] = "Set Level " .. extHum
            commandArray['Z_ExteriorPressure'] = "Set Level " .. extPres
    
        elseif deviceName == "Z_Mode" then
            deviceValue = GetSliderValue(deviceValue)
            if (deviceValue == 0 or deviceValue >= 99) and currentMode ~= 0 then
                commandArray[deviceName] = "Set Level " .. currentMode
            elseif deviceValue > 0 then
                SetThermostatMode(deviceValue);
            end
                
        elseif deviceName == "Z_ExteriorTemperature" then
            if deviceValue == 0 and extTemp ~= nil then
                commandArray[deviceName] = "Set Level " .. extTemp
            end
            
        elseif deviceName == "Z_ExteriorHumidity" then
            if deviceValue == 0 and extHum ~= nil then
                commandArray[deviceName] = "Set Level " .. extHum
            end
            
        elseif deviceName == "Z_ExteriorPressure" then
            if deviceValue == 0 and extPres ~= nil then
                commandArray[deviceName] = "Set Level " .. extPres
            end

        --elseif deviceName == "Thermostat command" and deviceValue == 14 then
        --    --commandArray['Thermostat command'] = "Set Level 1"
        --    commandArray['Thermostat value'] = "Set Level " .. currentMode
        --    print("Zuno set Thermostat Mode on Domoticz:" .. currentMode)
        end
    end
end


-- loop through all the variables
-- for variableName, variableValue in pairs(uservariables) do
--    if (variableName=='myVariable') then
--        if variableValue == 1 then
--            commandArray['a device name'] = "On"
--            commandArray['Group:My Group'] = "Off AFTER 30"
--        end
--    end
-- end

commandArray['Variable:IsInHeatingCycle'] = tostring(uservariables['IsInHeatingCycle'])
return commandArray
