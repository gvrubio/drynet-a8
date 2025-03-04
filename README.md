# DryNet-A8

DryNet-A8 is a firmware project for controlling Anet A8 3D printers using a serial-based JSON interface. This project allows users to send JSON commands over a serial connection to control various aspects of the printer, such as temperature, fans, and stepper motors.

## Features

- Serial-based control using JSON commands
- Custom configuration via `AnetConfig.h`
- Utility functions for enhanced printer operation (`AnetFunctions.h`)
- Main firmware logic in `drynet-a8.ino`
- Compatible with Arduino-based controllers

## Installation

### Requirements

- Anet A8 3D Printer
- Arduino IDE
- Compatible microcontroller
- USB Serial Connection

### Steps

1. Clone this repository:
   ```sh
   git clone https://github.com/gvrubio/drynet-a8.git
   ```
2. Open `drynet-a8.ino` in Arduino IDE.
3. Configure your settings in `AnetConfig.h`.
4. Compile and upload the firmware to your printer's controller.

## File Overview

- **`AnetConfig.h`** - Configuration settings for printer and control.
- **`AnetFunctions.h`** - Utility functions for handling printer operations.
- **`drynet-a8.ino`** - Main firmware file containing core logic and JSON parsing.

## JSON Serial Commands

The firmware listens for JSON commands over the serial interface. The following JSON keys can be used to control the printer:

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

1. Connect to the printer via a serial terminal (e.g., Arduino Serial Monitor, PuTTY).
2. Send JSON commands to control the printer.
3. The firmware will parse and apply the settings accordingly.

## Contributing

Feel free to submit pull requests or open issues for bug reports and feature requests.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Author

[GitHub - gvrubio](https://github.com/gvrubio)

