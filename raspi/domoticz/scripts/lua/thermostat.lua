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

print ("=== Start of thermostat script ===")

p="(%d+)-(%d+)-(%d+) (%d+):(%d+):(%d+)"
offset=os.time()-os.time(os.date("!*t"))

level = 0
oldLevel = 0

-- get previous Thermostat setting
for deviceName, deviceValue in pairs(otherdevices) do
    if (deviceName == 'Thermostat Setpoint') then
        levelStr = string.gsub(string.sub(deviceValue, -4, -3), "%s", "")
        oldLevel = tonumber(levelStr)
    end
end

-- Prepare radiator setpoints
spR1       = 96  -- Setpoint bureau etage
spR2       = 23  -- Setpoint palier
spR3       = 20  -- Setpoint chambre
Tb         = nil
Tp         = nil
Tc         = nil


-- Loop through all the changed devices sent by the trigger
if devicechanged ~= nil then
    for deviceName, deviceValue in pairs(devicechanged) do
        print("Devices changed: " .. deviceName .. " - value: " .. deviceValue)
        if (deviceName == 'Thermostat Setpoint') then
            levelStr = string.gsub(string.sub(deviceValue, -4, -3), "%s", "")
            level = tonumber(levelStr)
            print("Thermostat Setpoint - oldLevel:" .. oldLevel .. " - level:" .. level)
            
            if (level >= 10 and level < 20) then
               commandArray['Thermostat Hors Gel'] = "On"
               Tb = 5.0
               Tp = 5.0
               Tc = 5.0
               print("Set thermostat & radiators to Hors Gel !")
            elseif (level >= 20 and level < 30) then
               commandArray['Thermostat Parti'] = "On"
               Tb = 15.0
               Tp = 15.0
               Tc = 15.0
               print("Set thermostat & radiators to Parti !")
            elseif (level >= 30 and level < 40) then
               commandArray['Thermostat Nuit'] = "On"
               Tb = 15.0
               Tp = 15.0
               Tc = 16.0
               print("Set thermostat & radiators to Nuit !")
            elseif (level >= 40 and level < 50) then
               commandArray['Thermostat Jour'] = "On"
               Tb = 20.0
               Tp = 17.5
               Tc = 16.0
               print("Set thermostat & radiators to Jour !")
            elseif (level >= 50 and level < 60) then
               commandArray['Thermostat Confort'] = "On"
               Tb = 22.0
               Tp = 19.0
               Tc = 16.0
               print("Set thermostat & radiators to Confort !")
            else
               commandArray['Thermostat Hors Gel'] = "Off"
               commandArray['Thermostat Parti'] = "Off"
               commandArray['Thermostat Night'] = "Off"
               commandArray['Thermostat Jour'] = "Off"
               commandArray['Thermostat Confort'] = "Off"
               print("Set thermostat & radiators to undefined !")
            end
        else
            if (deviceValue == "On") then
                if (deviceName == 'Thermostat Hors Gel') then
                   commandArray['Thermostat Parti'] = "Off"
                   commandArray['Thermostat Nuit'] = "Off"
                   commandArray['Thermostat Jour'] = "Off"
                   commandArray['Thermostat Confort'] = "Off"
                   level = 10
                   
                elseif (deviceName == 'Thermostat Parti') then
                   commandArray['Thermostat Hors Gel'] = "Off"
                   commandArray['Thermostat Night'] = "Off"
                   commandArray['Thermostat Jour'] = "Off"
                   commandArray['Thermostat Confort'] = "Off"
                   level = 20
                   
                elseif (deviceName == 'Thermostat Nuit') then
                   commandArray['Thermostat Hors Gel'] = "Off"
                   commandArray['Thermostat Parti'] = "Off"
                   commandArray['Thermostat Jour'] = "Off"
                   commandArray['Thermostat Confort'] = "Off"
                   level = 30
                   
                elseif (deviceName == 'Thermostat Jour') then
                   commandArray['Thermostat Hors Gel'] = "Off"
                   commandArray['Thermostat Parti'] = "Off"
                   commandArray['Thermostat Nuit'] = "Off"
                   commandArray['Thermostat Confort'] = "Off"
                   level = 40
                   
                elseif (deviceName == 'Thermostat Confort') then
                   commandArray['Thermostat Hors Gel'] = "Off"
                   commandArray['Thermostat Parti'] = "Off"
                   commandArray['Thermostat Nuit'] = "Off"
                   commandArray['Thermostat Jour'] = "Off"
                   level = 50
                end
                if (level > 0 and level ~= oldLevel) then
                    commandArray['Thermostat Setpoint'] = "Set Level " .. level
                end
            end
        end
    end
end

if Tb ~= nil then
    commandArray[#commandArray + 1] = {['UpdateDevice']=spR1..'|0|'..tostring(Tb)}
    commandArray[#commandArray + 1] = {['UpdateDevice']=spR2..'|0|'..tostring(Tp)}
    commandArray[#commandArray + 1] = {['UpdateDevice']=spR3..'|0|'..tostring(Tc)}
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
print ("=== End of thermostat script ===")

return commandArray
