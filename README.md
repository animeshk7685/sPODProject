# spod

## Arduino libraries used and their version numbers.
- ArduinoRS485
- GD23Z
- I2C_device_Arduino
- LEDDrivers_NXP_Arduino
- LP5024
- NimBLE-Arduino
- PCA95x5
- SerialFlash
- spod_library
- TCA6408
- TCA9554

## Touchscreen:
- lychee_library v1.0.1 in repos\spod\libraries\spod_library 
- Wire v3.2.0 in esp32\hardware\esp32\3.2.0\libraries\Wire 
- LittleFS v3.2.0 in esp32\hardware\esp32\3.2.0\libraries\LittleFS 
- FS v3.2.0 in esp32\hardware\esp32\3.2.0\libraries\FS 
- GD23Z in repos\spod\libraries\GD23Z (legacy)
- SPI v3.2.0 in esp32\hardware\esp32\3.2.0\libraries\SPI 
- EEPROM v3.2.0 in esp32\hardware\esp32\3.2.0\libraries\EEPROM 
- LEDDrivers_NXP_Arduino v1.0.2 in repos\spod\libraries\LEDDrivers_NXP_Arduino 
- I2C_device_Arduino v1.1.0 in repos\spod\libraries\I2C_device_Arduino 
- TCA9554 v0.1.1 in repos\spod\libraries\TCA9554 
- PCA95x5 v0.1.3 in repos\spod\libraries\PCA95x5 
- ArduinoRS485 v1.1.0 in repos\spod\libraries\ArduinoRS485

## Switch:
- lychee_library v1.0.1 in repos\spod\libraries\spod_library 
- Wire v3.2.0 in esp32\hardware\esp32\3.2.0\libraries\Wire 
- LittleFS v3.2.0 in esp32\hardware\esp32\3.2.0\libraries\LittleFS 
- FS v3.2.0 in esp32\hardware\esp32\3.2.0\libraries\FS 
- TCA6408 v1.0.0 in repos\spod\libraries\TCA6408 
- LP50XX v1.0.0 in repos\spod\libraries\LP5024 
- PCA95x5 v0.1.3 in repos\spod\libraries\PCA95x5 
- ArduinoRS485 v1.1.0 in repos\spod\libraries\ArduinoRS485

## Switch magic two-button sequences:
- BUTTONS 2&3 -- TESTING:  for Baja testing...
- BUTTONS 4&8 -- NEXTADDR: cycle through source address
- BUTTONS 6&7 -- SLEEP:    sleep
- BUTTONS 7&8 -- LOCKOUT:  lockout keypad (press again to unlock)
- BUTTONS 1&8 -- RESET:    request reset to default configuration
- BUTTONS 4&5 -- CONFIRM:  confirm reset to default configuration

## PCM:


## Touchscreen screen flow:
> ---------------------------------------------
1_0_home_page.cpp:
  1. "Config Setup"  -- 2_0_0_config_setup_page.cpp
  2. "Settings Help" -- 2_0_1_settings_page.cpp

2_0_0_config_setup_page.cpp
  1. "Automatic Triggers" -- 3_0_0_0_auto_trigger_select_page.cpp
  2. "Config Triggers"    -- 3_0_0_1_config_triggers_page.cpp
  3. "Config Circuit"     -- 3_0_0_2_config_circuit_page.cpp
  4. "Change Icon/Name"   -- 3_0_0_3_switch_icon_name_page.cpp (does not exist yet)
  5. "HOME"               -- 1_0_hope_page.cpp
  6. Select SWITCH-#      -- 3_0_0_4_switch_setup_page.cpp

3_0_0_0_auto_trigger_select_page.cpp
  1. "Back"/"Abort"       -- 2_0_0_config_setup_page.cpp
  2. "Home"               -- 1_0_home_page.cpp
  3. "Clear"              -- TBD
  4. "Test"               -- TBD (gets a message from PCM1 when auto_trigger detected)
  5. "Apply"              -- stays on this page with updated set to true
  6. Select TRIGGER-#     -- 4_0_0_0_0_auto_trigger_output_page.cpp

3_0_0_1_config_triggers_page.cpp
  1. "Back"/"Abort"       -- 2_0_0_config_setup_page.cpp
  2. "Home"               -- 1_0_home_page.cpp
  3. "Apply"              -- stays on this page with updated set to true

3_0_0_2_switch_icon_name_page.cpp -- TBD

3_0_0_3_config_output_page.cpp
  1. "Back"/"Abort"       -- 2_0_0_config_setup_page.cpp
  2. "Home"               -- 1_0_home_page.cpp
  3. "Apply"              -- stays on this page with updated set to true

3_0_0_4_switch_setup_page.cpp
  1. "Back"/"Abort"       -- 2_0_0_config_setup_page.cpp
  2. "Home"               -- 1_0_home_page.cpp
  3. "Output Style"       -- 4_0_0_4_0_output_setup_page.cpp
  4. "Clear"              -- stays on this page after clearing test_switch and circuit_activation
  5. "Test"               -- 4_0_0_4_1_test_page.cpp
  6. "Apply"              -- stays on this page with updated set to true

4_0_0_4_0_output_setup_page.cpp
  1. "Back"               -- 3_0_0_4_switch_setup_page.cpp
  2. "Home"               -- 1_0_home_page.cpp
  3. "Clear"              -- stays on this page after clearing the local outputs
  4. "Test"               -- TBD, test the local outputs
  5. "Apply"              -- TBD, apply changes to the local outputs

4_0_0_4_1_test_page.cpp
  1. "Back"               -- 3_0_0_4_switch_setup_page.cpp
  2. "Home"               -- 1_0_home_page.cpp
  3. "Apply"              -- stays on this page with updated set to true


2_0_1_settings_page.cpp
  1. "BACK"               -- 1_0_home_page.cpp
  2. "HOME"               -- 1_0_home_page.cpp
  3. "Factory Reset"      -- 3_0_1_0_factory_reset_page.cpp
  4. "More Info"          -- 3_0_1_1_more_info_page.cpp
  5. "Help"               -- 3_0_1_2_help_page.cpp


3_0_1_0_factory_reset_page.cpp
  1. "Abort"              -- 2_0_1_settings_page.cpp (without setting to factory defaults)
  2. "Confirm"            -- 2_0_1_settings_page.cpp (after setting to factory defaults)
