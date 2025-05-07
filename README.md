![path83](https://github.com/user-attachments/assets/97c6c39d-f574-43be-8745-9232d19bd2b8)
# DryNet-A8

DryNet-A8 is a project that upcycles the prone-to-burn Anet A8 3D printer components into a functional filament dryer. By repurposing the printer's heated bed, fans, and electronics, this project provides an efficient way to dry filament using a networked or serial-controlled interface.

**DISCLAIMER**: 
You still have to do all the safety fixes recommended for the Anet for this project, I would not like to see your house burn, fellas. 
This firmware has much less safety features than standard Marlin or Klipper. 
This does not have thermal runaway protection for now.
Keep your dryer under watch, use smoke sensors, etc. This is an ongoing development project, it works, but it is early. 
Help would be much appreciated.

BASIC SAFETY: Solder your bed wires, add a fan to the PSU, get mosfets.

## Features

- Converts Anet A8 components into a filament drying system
- Serial-based control using JSON commands
- Utilizes Anet A8 mainboard for control
- Utility functions for enhanced drying operation (`AnetFunctions.h`)
- Main firmware logic in `drynet-a8.ino`
- Compatible with Arduino-based controllers

## Installation

### Requirements

- Anet A8 3D Printer components (bed, fans, controller, etc.)
- Enclosure or box for the filament dryer
- Arduino IDE
- USB Cable
  
- EXTRAS:
- 2x **12V Mosfet modules**, one for the bed, another for the computer fan
- A **12v computer fan** to move the bed heat all around the enclosure
- i2C Oled display, for when the local UI is ready
- 1 shtc3 Humidity and Temperature sensor
- A TXS0108E ttl logic converter board, for the extra modules that run at 3.3v.
- 2MM Wire for anything power (PSU, Bed, Mosfets)

### Steps

1. Build the Drynet A8 hardware, ensuring all components are correctly wired and functional. GUIDE SOON.
2. Clone this repository:
   ```sh
   git clone https://github.com/gvrubio/drynet-a8.git
   ```
3. Open `drynet-a8.ino` in Arduino IDE.
4. Upload the firmware to the Anet A8 mainboard.

## File Overview

- **`AnetConfig.h`** - Defines the pin mappings for the Anet A8 mainboard.
- **`AnetFunctions.h`** - Utility functions for handling drying operations.
- **`drynet-a8.ino`** - Main firmware file containing core logic and JSON parsing.

## JSON Serial Commands

The firmware listens for JSON commands over the serial interface. The following JSON keys can be used to control the dryer:

| JSON Key         | Type    | Description |
|-----------------|---------|-------------|
| `preset`        | String  | Sets temperature and fan settings based on predefined modes (`"OFF"`, `"PLA"`, `"PETG"`, `"TPU"`) |
| `dryTimer`      | Integer | Sets the duration (in seconds) for drying mode |
| `isStepperOn`   | Boolean | Turns the stepper motor on (`true`) or off (`false`) |
| `isFan1On`      | Boolean | Turns Fan 1 on (`true`) or off (`false`) |
| `isFan2On`      | Boolean | Turns Fan 2 on (`true`) or off (`false`) |
| `targetAirTemp` | Float   | Sets the target ambient temperature (°C) |
| `targetBedTemp` | Float   | Sets the target bed temperature (°C) |
| `debug`         | Boolean | Enables (`true`) or disables (`false`) debug mode |

### Example JSON Command

To set a preset for PLA mode and enable the stepper motor, send the following JSON over serial:
```json
{
  "preset": "PLA",
  "isStepperOn": true
}
```

## Usage

1. Assemble the filament dryer using Anet A8 components.
2. Connect to the dryer via a serial terminal (e.g., Arduino Serial Monitor, PuTTY).
3. Send JSON commands to control temperature, fans, and drying time.
4. The firmware will parse and apply the settings accordingly.

## Contributing

Feel free to submit pull requests or open issues for bug reports and feature requests.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Author

[GitHub - gvrubio](https://github.com/gvrubio)
