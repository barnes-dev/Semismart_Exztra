# CHANGELOG - 3D Monkey SemiSmart Firmware

## 📋 Version History

### v1.0.0-RC - Release Candidate

#### Critical Bug Fixes
- Fixed cooldown logic bug (heaterPrevState detection)
- Fan enters cooldown when heater was active
- Navigation bug fixed (switch statement restored)

#### Dead Code Elimination
- Removed Timer splashTimer and redundant logic
- Removed duplicate pin defines
- Removed unused fan states
- Removed unused time and conversion constants

#### Performance Optimizations
- PID Manager: cached frequently accessed values
- Thermistor: optimized with pre-calculated constants and ternaries
- Thermal Security: simplified validation
- GPIO: control logic only on state changes
- Animation: cached millis()

#### Memory Optimizations
- Display: removed redundant temporary variables
- Strings: shortened PROGMEM strings
- Font: removed redundant setFont() calls
- TextColor: simplified in configuration screens
- Universal constants hardcoded

#### Structure Improvements
- PlatformIO: integration and advanced flags
- Includes: logical grouping and better dependency management

#### Code Simplifications
- ON/OFF: ternaries for state transitions
- Power: constrain() for limits
- Temperature validation: single boolean expression
- Steinhart: optimized with constants
- Percentages: integer division

---

Previous versions

### beta2.0.0 - Memory & Code Optimizations
- Memory flash optimizations
- Code logic optimizations
- Animation and display improvements
- Mathematical constant optimizations

### beta1.2.0 - Performance & Structure
- Performance optimizations (caching, calculations)
- Structure optimizations (PlatformIO, includes)
- Documentation improvements

### beta1.1.0 - Critical Optimizations
- Dead code elimination
- Redundancy simplification
- Memory optimization
- Complex logic simplification

### beta1.0.0 - Initial Release1
- Basic functionality implemented
- Core system working