```ascii
  _______  _        _____   _______ ______ _____ _    _ 
 |__   __|| |      / ____| |__   __|  ____/ ____| |  | |
    | |   | |     | |         | |  | |__ | |    | |__| |
    | |   | |     | |         | |  |  __|| |    |  __  |
    | |   | |____ | |____     | |  | |___| |____| |  | |
    |_|   |______| \_____|    |_|  |______\_____|_|  |_|
                                                        
         Technologies Egypt - Power of Innovation
```

# Rotary Encoder Module v1.6
*Copyright © 2024 TLC-Technologies Egypt*
*All rights reserved. Protected by IP laws in Egypt*

*Released: January 2024*

```ascii
 +------------------+
 |  Rotary Module   |
 +------------------+
        |
   +----+-----+
   |          |
+--+--+   +---+---+
| Enc |   | Button|
+--+--+   +---+---+
   |          |
   +----+-----+
        |
    +---+----+
    | Queues |
    +---+----+
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
- Pin A: GPIO 18 (Pull-up enabled)
- Pin B: GPIO 19 (Pull-up enabled)
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
- [x] Button press duration measurement
- [x] Button press counting
- [x] Button debouncing
- [x] Menu position tracking
- [x] Multi-task support
- [x] Event queuing system

## API Functions
```c
// Initialization
void rotary_encoder_init(void);

// Encoder Functions
int rotary_encoder_get_angle(void);          // 0-359°
int rotary_encoder_get_rotations(void);      // Complete rotations
int rotary_encoder_get_menu_position(void);  // Menu position
int rotary_encoder_get_step_counter(void);   // Step count

// Button Functions
uint32_t rotary_encoder_get_button_press_count(void);    // Total presses
uint32_t rotary_encoder_get_last_press_duration(void);   // Last press duration
```

## Event Structures
```c
// Encoder Events
typedef struct {
    int counter;              // Current position
    int steps;               // Total steps taken
    encoder_direction_t dir; // CW or CCW
    int angle;              // Current angle
    int rotations;         // Complete rotations
    int menu_position;     // Menu selection
} encoder_event_t;

// Button Events
typedef struct {
    bool pressed;            // Button state
    uint32_t press_duration; // Duration in ms
    uint32_t press_count;    // Press counter
} button_event_t;
```

## Implementation Details
- Uses FreeRTOS tasks
- Hardware timer for debouncing
- Queue-based event system
- Thread-safe implementation

## Example Outputs
```
// Encoder Events
Direction: CW, Counter: 1, Total Steps: 1
Angle: 18°, Rotations: 0, Menu Position: 1

// Button Events
Button pressed! Count: 0
Button released! Duration: 320 ms, Total presses: 1
```

## Task Configuration
- Encoder Task Priority: 5
- Button Monitor Priority: 5
- Printer Task Priority: 1
- Button Poll Rate: 10ms
- Encoder Poll Rate: 1ms

## Queue Configuration
- Encoder Queue Size: 10 events
- Button Queue Size: 5 events

## Timing Parameters
- Button Debounce: 50ms
- Encoder Debounce: 5ms
- Timer Resolution: 1MHz

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
- v1.4: Improved stability
- v1.5: Added documentation
- v1.6: Added button timing and counting

## Legal Notice
This code is protected under Egyptian intellectual property laws. 
Unauthorized copying, modification, or distribution is strictly prohibited.
© 2024 TLC-Technologies Egypt. All rights reserved.

---
Last Updated: January 2024
Contact: TLC-Technologies Egypt
