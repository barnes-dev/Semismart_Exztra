#include "../../include/utils/tasks.h"
#include "../../include/sensors/sensor_manager.h"
#include "../../include/core/state_manager.h"
#include "../../include/control/control.h"
#include "../../include/safety/thermal_security.h"
#include "../../include/core/system_config.h"
#include "../../include/interface/gpio_interface.h"
#if DISPLAY_TYPE_SSD1306
#include "../../include/display/view_manager_sh1306.h"

#elif DISPLAY_TYPE_SH110X
#include "../../include/display/view_manager_sh1107.h"

#elif DISPLAY_TYPE_SSD1351
#include "../../include/display/view_manager_ssd1351.h"

#elif DISPLAY_TYPE_ST7796S
#include "../../include/display/view_manager_st7796s.h"
#endif
extern ViewManager viewManager;
extern SensorManager sensorManager;
extern StateManager stateManager;
extern ControlSystem controlSystem;

void displayTask() {
  viewManager.draw();
}

void sensorTask() {
  sensorManager.update();
}

void fanTask() {
    controlSystem.updateFanState();
    controlSystem.updateFanTransition();
}

void thermalTask() {
    updateThermalSecurity();
}

void updateStateTask() {
    stateManager.setHeaterOn(controlSystem.isHeaterOn());
    stateManager.setFanOn(controlSystem.isFanOn());
    #if GPIO_INTERFACE_ENABLED
    pollGPIOInterface();
    #endif
} 