⚠️ DISCLAIMER

This firmware is provided "as is" without any warranties. The user assumes all responsibility for installation, operation, and any consequences. This is not a commercial product and has not been tested for all possible scenarios. Use at your own risk. Don't leave the printer unnatended

---

This is an enhanced firmware implementation for the Semi Smart PCB addon designed by Voxel3D. The Semi Smart is a heated addon for the CoreBoxx enclosure system. This firmware provides enhanced features and improved functionality while maintaining compatibility with the original hardware design.

🎯 Overview

The 3DMonkey Enhanced Semi Smart firmware is an environmental controller firmware that maintains optimal conditions inside the enclosure. It features dual operation modes, PID temperature control, intelligent fan management, thermal safety systems, and a three-state system (OFF, STANDBY, ON) for complete control.


🏗️ System Architecture

Core Components

The firmware is built around a modular architecture with the following key components:

**State Manager**: Central state management and system coordination
**Sensor Manager**: Handles SHT4x temperature/humidity sensor operations
**PID Manager**: Advanced temperature control using GyverPID library
**Control System**: Heater and fan control with intelligent state management
**Thermal Security**: Multi-layer safety system for thermal protection
**View Manager**: OLED display interface with multiple views including splash screen
**Storage Manager**: EEPROM-based configuration persistence
**GPIO Interface**: Optional printer integration for automatic control

Hardware Requirements

Semi Smart PCB addon
CoreBoxx enclosure system


🔧 System States

The system operates in three distinct states:

**SYSTEM_OFF**: Complete shutdown
- No fan operation
- No heater operation
- No air cycling

**SYSTEM_STANDBY**: Low-power standby mode
- Periodic air cycling (15s on, 5min off)
- No heating
- Maintains air circulation
- Display shows "Standby" status
- Control mode selection available (USER/AUTO)

**SYSTEM_ON**: Full operation
- Active temperature control
- Proportional fan operation
- Full PID control
- Display shows operational data

State Transitions

**Button Logic**:
- **Short press ACTION**: OFF ↔ STANDBY (in any control mode)
- **Long press ACTION (2s)**: STANDBY → ON (USER mode only)
- **Long press ACTION (2s)**: ON → OFF (USER mode only)

**Automatic Transitions** (AUTO mode, controlled by printer GPIO):
- Print start: OFF/STANDBY → ON
- Print end: ON → OFF


🔧 Operation Control Modes

AUTO Mode

**Purpose**: Automatic control based on printer status
**Control**: Temperature-based PID control with humidity monitoring
**Logic**: Automatically starts/stops based on printer print status
**Use Case**: Integrated with 3D printer via GPIO interface
**State Control**: Automatic transitions based on print status

USER Mode

**Purpose**: Manual control by user
**Control**: Temperature-based PID control with manual start/stop
**Logic**: User controls system states via buttons
**Use Case**: Standalone operation without printer integration
**State Control**: Full manual control of all three states

🔧 Dry Modes

**DRY_MODE_BY_HUM**: Humidity-based control (stops when target humidity reached)
**DRY_MODE_BY_TIME**: Time-based drying operations (runs for specified duration)



🌡️ Temperature Control System

Control Features

PID temperature control using GyverPID library
Smooth power transitions
Configurable setpoints via UI (15°C - 45°C)
Support for both Celsius and Fahrenheit units
Proportional fan speed based on heater power

Temperature Ranges

**Operating Range**: 15°C - 45°C (59°F - 113°F)
**Default Temperature**: 35°C (95°F)
**Safety Limits**: 0°C - 55°C (32°F - 131°F)



💨 Fan Management System

Fan States

**FAN_OFF**: Complete shutdown
**FAN_COOLDOWN**: High-speed cooling after heater shutdown (100% PWM)
**FAN_STANDBY_OFF**: No fan operation (SYSTEM_OFF state)
**FAN_STANDBY_CYCLING_ON**: Low-speed periodic operation in standby (40% PWM)
**FAN_STANDBY_CYCLING_OFF**: Standby cycle off period
**FAN_ON**: Proportional speed based on heater power (35% minimum when heating)

Intelligent Features

Proportional fan speed based on heater power
Smooth speed transitions (2-second transition time)
Automatic cooldown cycles (30 seconds after heater off)
Standby cycling for air circulation (15s on, 5min off)
Complete silence option in OFF state


🛡️ Thermal Security System

Multi-Layer Protection

**Temperature Validation**: Ensures sensor readings are within valid ranges
**Heater Temperature Monitoring**: Separate thermistor for heater safety
**Automatic Recovery**: System resumes operation after safety conditions are met
**Visual Alerts**: Warning indicators on display

Protection States

**THERMAL_NORMAL**: Standard operation
**THERMAL_PROTECTION**: Heater disabled, fan at maximum (55°C threshold)
**THERMAL_COOLDOWN**: Cooling period before recovery (40°C recovery threshold)

Safety Parameters

**Protection Threshold**: 55°C (heater temperature)
**Recovery Threshold**: 40°C
**Minimum Protection Time**: 5 seconds
**Cooldown Time**: 30 seconds



📱 User Interface

Splash Screen

**Duration**: 2.5 seconds on startup
**Content**: VOXEL3D logo and "Code by 3DMonkey"
**Priority**: Blocks all other state changes during display

Display Views

**SPLASH**: Startup logo screen (2.5s duration)
**STANDBY**: System status, shows current conditions and control mode
**INFO**: Main operational view with real-time data and status indicators
**TEMP**: Temperature configuration (15-45°C)
**HUM**: Humidity configuration (5-90% RH, AUTO mode only)
**MODE**: Operation mode selection (BY HUM / BY TIME)
**DRY_TIME**: Drying duration configuration (10min-12h, BY TIME mode only)

Button Functions

**Action Button**:

Short press: 
- OFF → STANDBY (any mode)
- STANDBY → OFF (any mode)
- Enter/exit edit mode (when ON in USER mode)

Long press (2s): 
- STANDBY → ON (USER mode only)
- ON → OFF (USER mode only)

**Plus Button**: 
- Navigate views or increment values
- Change control mode (AUTO/USER) in STANDBY state only

**Minus Button**: Navigate views or decrement values

Visual Indicators

Status indicators (300ms intervals)
Power percentage display for heater
Countdown timers for DRY mode
Warning indicators for safety conditions
Blinking elements for active states
State-specific display messages (OFF, STBY, etc.)



🔄 Task Scheduling

The system uses a non-blocking task scheduler to ensure responsive operation:

**Sensor readings**: Every 2 seconds
**Display updates**: Every 300ms
**PID calculations**: Every 100ms
**Thermal safety**: Every 100ms
**Main loop**: Every 50ms
**Button polling**: Every 200ms
**Splash screen**: 2.5 seconds duration



💾 Configuration Storage

Persistent Settings
Target temperature and humidity
Operation mode preference (BY HUM / BY TIME)
Drying duration for BY TIME mode
Control mode (USER / AUTO)
Automatic validation and recovery of corrupted settings

EEPROM Management

Version control for configuration compatibility
Automatic fallback to safe defaults if corruption is detected
Range validation for all stored values



📊 Sensor Integration

SHT4x Sensor
Temperature and humidity readings
I2C communication
Automatic error detection and recovery
2-second measurement intervals

Sensor Validation

Range checking for temperature (-50°C to 150°C) and humidity (5-90%)
Rate-of-change monitoring (max 15°C change per reading)
Automatic fallback to safe values
Error recovery mechanisms


🔌 GPIO Interface (Optional)

Printer Integration

**Enabled by default**: Can be disabled for standalone operation
**Print Status Detection**: Monitors printer print status via GPIO
**Automatic Control**: Starts system when printing begins, stops when printing ends
**Compatibility**: Designed for printers with GPIO interface (e.g., Original Prusa MK4/S, MK3.9/S, or MK3.5/S series or CoreOne with xBuddy boards).
For XL and mk3 series and lower, you can use this as standalone by setting GPIO_INTERFACE_ENABLED 0

Control Logic

**Print Start**: Automatically turns system ON, sets mode to BY HUM
**Print End**: Automatically turns system OFF, enters OFF state
**Manual Override**: User can still control system manually



🚀 Getting Started

Prerequisites

**Hardware**: Semi Smart PCB addon by Voxel3D
**Enclosure**: CoreBoxx enclosure system
**Components**: Follow the original BOM from Voxel3D's design
**Assembly**: Complete the hardware assembly as per original instructions

Firmware Installation

Download this firmware
Upload the Arduino sketch to your Arduino Nano
Power on the system
Splash screen will display for 2.5 seconds
System starts in OFF state

Initial Configuration

Set target temperature (15-45°C)
Set target humidity (5-90%)
Select operation mode (BY HUM / BY TIME)
Configure drying duration if using BY TIME mode
Select control mode (USER / AUTO)

Operation

**Starting the System**:
1. Short press ACTION button: OFF → STANDBY
2. Long press ACTION button: STANDBY → ON (USER mode only)

**Stopping the System**:
1. Long press ACTION button: ON → OFF (USER mode only)
2. Short press ACTION button: STANDBY → OFF

**AUTO Mode Operation**:
- System automatically starts when printing begins
- System automatically stops when printing ends
- Manual control still available


⚙️ Configuration

Hardware Configuration

**For printers without xBuddy (e.g., MK3 series)**: Set `GPIO_INTERFACE_ENABLED 0` in config.h
**For printers with xBuddy**: Set `GPIO_INTERFACE_ENABLED 1` in config.h (default)

Temperature Units

**For Celsius**: Set `USE_FAHRENHEIT 0` in config.h (default)
**For Fahrenheit**: Set `USE_FAHRENHEIT 1` in config.h

Slicer Configuration

Add the following G-code commands in your slicer's "Start G-code" section:

```
M262 P0 B0
M264 P0 B1
```

Add the following G-code command in your slicer's "End G-code" section:

```
M264 P0 B0
```

These commands enable automatic control of the enclosure system based on print status.


🛠️ Troubleshooting

Common Issues


**System won't start (red led blinking on arduino)**

Check SHT sensor connection
Verify power supply voltage

**Temperature not reaching target**

Verify heater wiring (Pin 10)
Check PID parameters
Ensure adequate power supply

**Fan not operating**

Check fan wiring (Pin 3)
Verify PWM pin configuration
Check thermal safety system status

**Display issues**

Verify I2C connections
Check display address (0x3C)
Ensure proper power supply

**Splash screen not showing**

Check display connections
Verify timer initialization
Ensure no state conflicts

### Error Indicators

**Warning display**: Thermal protection active
**No display**: Check I2C connections
**Sensor errors**: Automatic fallback to safe values



📈 Performance Characteristics

Response Times

Temperature control: <30 seconds to target
Fan response: <1 second
UI updates: 300ms refresh rate
Safety response: <100ms
Splash screen: 2.5 seconds

Accuracy

Temperature: ±0.3°C (SHT4x specification)
Humidity: ±2% RH (SHT4x specification)
PID control: ±0.5°C typical

Power Consumption

**Power Supply**: 300W rated power supply
**OFF state**: <10mA (minimal power consumption)
**Standby**: <50mA
**Normal operation**: ~40% of rated power (~120W) during temperature maintenance
**Peak consumption**: Up to 300W during initial heating phase
**Typical maintenance**: 100-150W for temperature control

Addition from NiccZee:
v 1.0.0:

Added functionality: It is now selectable to run the heater (CYCLING) and get it controlled either by Temperature or by Humidity.
When temperature has reached the target setting it turns off the heater, but keeps the fans on until the temperature starts to descend and then turns off the fans. This way, the residual heat is taken care of. When the temperature decreseas to 3 degrees under the set target temp, the heater and fans turns on again, and cycles like this indefinitely until you turn off the heater (SYS OFF). The humidity option works the same way.
This is selectable with the MODE setting. (USER TEMP, USER HUM, AUTO TEMP, AUTO HUM). The AUTO options are only there for when you have the GPIO option on, to steer the Semismart when you print.

The Timer option has an option to set the time for how long it will stay on and heat to the set target temp, and then it will turn off.
There is also a setting to set for how long it will stay off until it starts to heat again. (Set to 0 minutes to turn off repetitions).

Added some animation on some icons for fun!
Added the Voxel logo at the startup screen as well.

The UI is based on the original UI from the Semismart, which means that the MODE is selected when "CYCLING" is on, and the 4 different modes (2 if you don't have th GPIO setting on) are selected with the upper button.

The lower button in the same "CYCLING" mode changes the LED-strip if the feature is turned on. I can see that the LED-strip used the D12 pin on the NANO. It is normally used with for receiving data with SPI communication, but it is not used with screens, as we only send to screens and never receive.

To turn on the modes "DRYING" or "PRINTING" manually, you have to have set the Mode at "CYCLING" to "USER TEMP" or "USER HUM". Then press the middle button for more than 2 seconds.

Once in "DRYING" or "PRINTING" mode, you can select these 2 options with the lower button, getting into "Operation Mode". To select mode, press the middle button to open selection, then press either upper button for "By Humid" = "Drying", or lower button for "Dry Timer" = "Printing". Press the middle button again to save your selection. And press the lower button to get to the selected operational mode.

In "Drying" mode the target temperature, a run-timer and a standby-timer is set. The target temperature sets when the heater should turn off. Fans will still be running. The Run-timer sets (in minutes) for how long you want the Heater and fans to be on, and the Standby-timer sets for how long the Heater and fans are off, until they turn on again. The timers repeat untill the Semismart is turned to a different mode or OFF. If the Standby timer is set to 0, there is no repetition. The the Run-timer has reached "ETA 0:00", it turns off the Heater and fans permanently "SYS OFF".
Temperature and timers are set by pressing the upper button, rolling through Temperature, Run-Time, and Standby-timer. Press the middle button to open the settting-mode and select up/down with the upper and lower buttons. Press middle button again to save your setting.

In "Printing" mode the setting are Temperature and Humidity, again rolling through with the upper button. This mode will run for as long you have it in this mode. This is also the mode that is turned on automatically (at "AUTO TEMP" and "AUTO HUM") when the GPIO feature is on. The fans and heater is controlled by the set Temp and Humidity.

To turn these modes off, you need to long press (2 seconds) the middle button to turn off the Semismart. "SYS OFF". The fans will stay on for 45 seconds, in COOLING mode, and then turn off.

🤝 Contributing

Contributions are welcome! Please feel free to open issues for bugs and feature requests.


3DMonkey Enhanced Semi Smart** - Enhanced firmware for Voxel3D's Semi Smart addon
