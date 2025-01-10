```ascii
  _______  _        _____   _______ ______ _____ _    _ 
 |__   __|| |      / ____| |__   __|  ____/ ____| |  | |
    | |   | |     | |         | |  | |__ | |    | |__| |
    | |   | |     | |         | |  |  __|| |    |  __  |
    | |   | |____ | |____     | |  | |___| |____| |  | |
    |_|   |______| \_____|    |_|  |______\_____|_|  |_|
                                                        
         Technologies Egypt - Power of Innovation
```

# Rotary Encoder Module v1.5
*Copyright © 2024 TLC-Technologies Egypt*
*All rights reserved. Protected by IP laws in Egypt*

*Released: January 2024*

```ascii
 +-----------------+
 |  Rotary Module  |
 +-----------------+
        |
   +----+----+
   |         |
+--+--+  +--+--+
| Enc |  | Btn |
+--+--+  +--+--+
   |         |
   +----+----+
        |
    +---+---+
    | Queue |
    +---+---+
        |
  +-----+-----+
  |   Tasks   |
  +-----------+
```

## Quick Start
1. Include the module:
   ```c
   #include "rotary_encoder_module.h"
   ```
2. Initialize:
   ```c
   rotary_encoder_init();
   ```

## Hardware Configuration
- Pin A: GPIO 18
- Pin B: GPIO 19
- Button: GPIO 15 (Active Low, External Pull-up)

## Specifications
- Steps per rotation: 20
- Degrees per step: 18°
- Full rotation: 360°
- Counter direction: CW (clockwise) / CCW (counter-clockwise)

## Features
- [x] Position tracking
- [x] Rotation counting
- [x] Angle calculation
- [x] Direction detection
- [x] Button debouncing
- [x] Menu position tracking
- [x] Step counting

## API Functions
```c
void rotary_encoder_init(void);              // Initialize module
int rotary_encoder_get_angle(void);          // Get current angle (0-359°)
int rotary_encoder_get_rotations(void);      // Get complete rotations
int rotary_encoder_get_menu_position(void);  // Get menu position
int rotary_encoder_get_step_counter(void);   // Get step count
int rotary_encoder_read_button(void);        // Read button state
```

## Event Structure
```c
typedef struct {
    int counter;              // Current position
    int steps;               // Total steps taken
    encoder_direction_t dir; // CW or CCW
    int angle;              // Current angle
    int rotations;         // Complete rotations
    int menu_position;     // Menu selection
} encoder_event_t;
```

## Implementation Details
- Uses FreeRTOS tasks
- Hardware timer for debouncing
- Queue-based event system
- Thread-safe implementation

## Example Output
```
Direction: CW, Counter: 1, Total Steps: 1
Angle: 18°, Rotations: 0, Menu Position: 1
```

## Task Priority
- Encoder Task: 5
- Printer Task: 1

## Dependencies
- ESP-IDF
- FreeRTOS
- GPIO Driver
- Timer Driver

## Future Improvements
- [ ] Add acceleration support
- [ ] Add position limits
- [ ] Add event callbacks
- [ ] Add configuration struct

## Maintenance Notes
1. Timer frequency: 1MHz
2. Debounce delay: 5ms
3. Queue size: 10 events
4. Button debounce: 50ms

## Version History
- v1.0: Initial release
- v1.1: Added debouncing
- v1.2: Fixed duplicate readings
- v1.3: Added menu position

## Legal Notice
This code is protected under Egyptian intellectual property laws. 
Unauthorized copying, modification, or distribution is strictly prohibited.
© 2024 TLC-Technologies Egypt. All rights reserved.

---
Version: 1.5
Last Updated: January 2024
Contact: TLC-Technologies Egypt
