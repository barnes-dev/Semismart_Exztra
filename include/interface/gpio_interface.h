#ifndef GPIO_INTERFACE_H
#define GPIO_INTERFACE_H

#include <Arduino.h>
#include "../core/system_config.h"

#if GPIO_INTERFACE_ENABLED
// Initialize GPIO interface pins and states
void initGPIOInterface();
// Should be called periodically to read/signals states
void pollGPIOInterface();
#else
inline void initGPIOInterface() {}
inline void pollGPIOInterface() {}
#endif

#endif // GPIO_INTERFACE_H 