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
commandArray = {}

print ("=== Start of switches script ===")

p="(%d+)-(%d+)-(%d+) (%d+):(%d+):(%d+)"
offset=os.time()-os.time(os.date("!*t"))

newLevel = 0
oldCommandId = 0
oldValue = 0
oldSetpoint = 0
extTemp1 = 0
extTemp2 = 0

t1 = 'Thermostat Hors Gel'
t2 = 'Thermostat Parti'
t3 = 'Thermostat Nuit'
t4 = 'Thermostat Jour'
t5 = 'Thermostat Confort'

-- Radiator setpoints
sp1Name    = 'Setpoint bureau etage'
sp2Name    = 'Setpoint palier'
sp3Name    = 'Setpoint chambre'
spR1       = 96
spR2       = 23
spR3       = 20
Tb         = nil
Tp         = nil
Tc         = nil

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
            valueStr = string.gsub(string.sub(valueRaw, 0, -2), "%s", "")
            valueStr = string.gsub(string.sub(valueRaw, 0, -2), "Set Level:", "")
            value = tonumber(valueStr)
        end
    end
    return value
end

function GetSetPointValue(valueRaw)
    local value = 0
    if valueRaw == "Off" then
        value = 0
    elseif valueRaw == "Frost" or valueRaw == t1 then
        value = 10
    elseif valueRaw == "Absent" or valueRaw == t2 then
        value = 20
    elseif valueRaw == "Night" or valueRaw == t3 then
        value = 30
    elseif valueRaw == "Day" or valueRaw == t4 then
        value = 40
    elseif valueRaw == "Comfort" or valueRaw == t5 then
        value = 50
    end
    return value
end

function GetThermostatSetPoint(valueRaw)
    local value = "Off"
    if valueRaw < 10 then
        value = "Off"
    elseif valueRaw < 20 then
        value = "Frost"
    elseif valueRaw < 30 then
        value = "Absent"
    elseif valueRaw < 40 then
        value = "Night" 
    elseif valueRaw < 50 then
        value = "Day"
    elseif valueRaw < 60 then
        value = "Comfort"
    end
    return value
end

function DeviceIsAThermostatSwitch(deviceName)
    return (deviceName == t1 or deviceName == t2 or deviceName == t3 or deviceName == t4 or deviceName == t5)
end

function TurnOffOtherSwitches(keepName)
    print("TurnOffOtherSwitches: " .. keepName)
    if keepName ~= t1 then
       commandArray[t1] = "Off"
    end
    if keepName ~= t2 then
       commandArray[t2] = "Off"
    end
    if keepName ~= t3 then
       commandArray[t3] = "Off"
    end
    if keepName ~= t4 then
       commandArray[t4] = "Off"
    end
    if keepName ~= t5 then
       commandArray[t5] = "Off"
    end
end

function TurnOnSwitch(switchName)
    print("Turn on switch: " .. switchName)
    commandArray[switchName] = "On"
end

function SetRadiators(newSetpoint)
    -- Send new setpoints to radiators if mode has changed
    if (newSetpoint > 0 and oldSetpoint ~= newSetpoint) then
        if Tb ~= nil then
            print("Sending new setpoints to radiators")
            commandArray[#commandArray + 1] = {['UpdateDevice']=spR1..'|0|'..tostring(Tb)}
            commandArray[#commandArray + 1] = {['UpdateDevice']=spR2..'|0|'..tostring(Tp)}
            commandArray[#commandArray + 1] = {['UpdateDevice']=spR3..'|0|'..tostring(Tc)}
        end
    end
end

function SetThermostatSetpoint(newSetpoint)
    print("newSetpoint " .. newSetpoint)
    print("oldSetpoint " .. oldSetpoint)
    if (newSetpoint > 0 and oldSetpoint ~= newSetpoint) then
        print("The thermostat switch changed to " .. newLevel .. ": " .. GetThermostatSetPoint(newLevel))
        commandArray['Thermostat Setpoint'] = 'Set Level ' .. newSetpoint
    end
end



-- commands

No_command                 = 0
-- Set commands: PC sends value to Zuno / Zuno requests value from PC
Get_Mode                   = 1

Get_Radiator_Count         = 2
Get_ActiveRadiator         = 3

Get_RadiatorSetpoint1      = 4
Get_RadiatorSetpoint2      = 5

Get_RadiatorTemperature1   = 6
Get_RadiatorTemperature2   = 7

Get_ExteriorTemperature1   = 8
Get_ExteriorTemperature2   = 9

Get_ExteriorHumidity1      = 10
Get_ExteriorHumidity2      = 11

Get_ExteriorPressure1      = 12
Get_ExteriorPressure2      = 13

-- Set commands: Zuno sends value to PC / PC requests value from Zuno
Rep_Mode                    = 14

Rep_Radiator_Count         = 15
Rep_ActiveRadiator         = 16

Rep_RadiatorSetpoint1      = 17
Rep_RadiatorSetpoint2      = 18

Rep_RadiatorTemperature1   = 19
Rep_RadiatorTemperature2   = 20

Rep_ExteriorTemperature1   = 21
Rep_ExteriorTemperature2   = 22

Rep_ExteriorHumidity1      = 23
Rep_ExteriorHumidity2      = 24

Rep_ExteriorPressure1      = 25
Rep_ExteriorPressure2      = 26

Set_mode                   = 27

function SendCommand(commandId, value)
    print("SEND COMMAND: " .. commandId .. ": " .. value)
    commandArray['ZRX command'] = "Set Level " .. commandId
    commandArray['ZRX value'] = "Set Level " .. value
end


-- get current values
for deviceName, deviceValue in pairs(otherdevices) do
    if (deviceName == 'ZTX command') then
        oldCommandId = GetSliderValue(deviceValue)
        
    elseif (deviceName == 'ZTX value') then
        oldValue = GetSliderValue(deviceValue)
        
    elseif (deviceName == 'Thermostat Setpoint') then
        oldSetpoint = GetSetPointValue(deviceValue)
        
    elseif (deviceName == 'Temperature Potelles') then
        extTemp1 = math.floor(deviceValue)
        extTemp2 = math.floor((deviceValue - extTemp1) * 100)
    end
end

-- Loop through all the changed devices sent by the trigger
if devicechanged ~= nil then
    for deviceName, deviceValue in pairs(devicechanged) do
        
        print("Devices changed: " .. deviceName .. " - new value: " .. deviceValue)
        
        if (deviceName == 'Thermostat Setpoint') then
            newLevel = GetSetPointValue(deviceValue)
            print("Thermostat Setpoint newLevel:" .. newLevel)
            
            if (newLevel >= 10 and newLevel < 20) then
               TurnOnSwitch(t1)
               Tb = 5.0
               Tp = 5.0
               Tc = 5.0
            elseif (newLevel >= 20 and newLevel < 30) then
               TurnOnSwitch(t2)
               Tb = 15.0
               Tp = 15.0
               Tc = 15.0
            elseif (newLevel >= 30 and newLevel < 40) then
               TurnOnSwitch(t3)
               Tb = 15.0
               Tp = 15.0
               Tc = 16.0
            elseif (newLevel >= 40 and newLevel < 50) then
               TurnOnSwitch(t4)
               Tb = 20.0
               Tp = 17.5
               Tc = 16.0
            elseif (newLevel >= 50 and newLevel < 60) then
               TurnOnSwitch(t5)
               Tb = 22.0
               Tp = 19.0
               Tc = 16.0
            else
               print("Set thermostat & radiators to undefined !")
            end
            SetRadiators(newLevel)
            if (newLevel > 0 and oldSetpoint ~= newLevel) then
                SendCommand(Rep_Mode, newLevel)
            end

        elseif (deviceValue == "On" and DeviceIsAThermostatSwitch(deviceName)) then
            print("DeviceIsAThermostatSwitch: " .. deviceName)
            TurnOffOtherSwitches(deviceName)
            SetThermostatSetpoint(GetSetPointValue(deviceName))
            
        elseif deviceName == "ZTX command" then
            -- Zuno pull
            commandId = GetSliderValue(deviceValue)
            if commandId == Get_Mode then
                print("ZUNO Pull ! commandId " .. commandId)
                SendCommand(Rep_Mode, oldSetpoint)
            elseif commandId == Get_ExteriorTemperature1 then
                print("ZUNO Pull ! extTemp1 " .. commandId)
                SendCommand(Rep_ExteriorTemperature1, extTemp1 + 50)
            elseif commandId == Get_ExteriorTemperature2 then
                print("ZUNO Pull ! extTemp2 " .. commandId)
                SendCommand(Rep_ExteriorTemperature2, extTemp2)
            elseif commandId == No_command then
                print("No command !")
            end

        elseif oldCommandId > No_command and deviceName == "ZTX value" then
            -- Zuno push
            deviceValue = GetSliderValue(deviceValue)
            if oldCommandId == Set_Mode and deviceValue > 0 then
                print("ZUNO Push ! Set_Mode")
                print("deviceValue: " .. deviceValue)
                print("RECEIVE COMMAND: Get_Mode (" .. Get_Mode .. ") " .. deviceValue)
                
                SetThermostatSetpoint(deviceValue)
                commandArray['ZTX value'] = "Set Level 0"
            end

        --elseif deviceName == "Thermostat command" and deviceValue == 14 then
        --    --commandArray['Thermostat command'] = "Set Level 1"
        --    commandArray['Thermostat value'] = "Set Level " .. oldSetpoint
        --    print("Zuno set Thermostat Mode on Domoticz:" .. oldSetpoint)
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
print ("=== End of switches script ===")

return commandArray
