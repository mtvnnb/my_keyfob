# NRF52840 KeyFob BLE Trigger

Control your car key fob button remotely via Bluetooth using your phone!

## Hardware

- **Board**: NRF52840 SuperMini (NiceNano clone)
- **Optocoupler**: PC817C (for electrical isolation)
- **Resistor**: 220Ω
- **Key Fob**: Any car key with physical button

## Wiring Diagram

```
PIN 020 ──[220Ω]── PC817C Pin 1 (Anode, dot side)
GND ────────────── PC817C Pin 2 (Cathode)
PC817C Pin 3 ───── Key fob button wire A
PC817C Pin 4 ───── Key fob button wire B
```

### Pin Configuration

- **P0.20 (PIN 020)**: Trigger pin - controls the optocoupler
- **P0.15 (PIN 015)**: Status LED (red) - indicates button press activity
- **GND**: Ground reference

## Features

- **Bluetooth Low Energy (BLE)** communication
- **UART Service** for receiving commands
- **Optocoupler isolation** to safely trigger the key fob without interfacing directly
- **Visual feedback** via status LED during button press
- **Multiple command formats** supported
- **Auto-reconnect** on disconnect

## How It Works

1. The nice!nano advertises as "KeyFob" over BLE
2. When connected via phone app, you can send commands
3. On command received, the microcontroller sends current through PIN 020
4. The optocoupler (PC817C) closes the circuit, simulating a button press on the key fob
5. Button is held for 300ms, then released
6. Status LED blinks to confirm activation

## Mobile App Setup

### Recommended Apps

- **nRF Connect** (Nordic Semiconductor) - Available on iOS/Android
- **Bluefruit Connect** (Adafruit) - Available on iOS/Android

### Connection Instructions

1. Open your BLE app
2. Scan for devices and connect to **"KeyFob"**
3. Navigate to UART service
4. Send one of the following commands:
   - `p` - Press button
   - `press` - Press button
## Quick Start

### 1. Install PlatformIO
- Install [VS Code](https://code.visualstudio.com/)
- Install PlatformIO IDE extension

### 2. Clone & Build
```bash
git clone <your-repo-url>
cd keyfob
pio run
```

### 3. Upload
```bash
# Find your COM port first (Windows Device Manager)
pio run -t upload --upload-port COM5
```
**Note**: Board switches to different COM port after upload (usually COM10).

### 4. Use Phone App
1. Download **"Bluefruit Connect"** (iOS/Android)
2. Connect to **"KeyFob"**
3. Tap **Controller** → **Control Pad**
4. Press any button (1-4) to trigger!

## Commands

### Via nRF Connect
- `p` or `press` or `1` - Trigger button

### Via Bluefruit Connect
- Controller buttons 1-4 - Trigger button

## Configuration

**platformio.ini**:
- `upload_port = COM5` - Change to your COM port
- `monitor_port = COM10` - Serial monitor port (after upload)

**main.cpp**:
- `delay(300)` - Button hold time in milliseconds
- **Status LED**: Turns on during press, off after release
- **BLE Response**: Confirms action with "Pressing button..." and "Done!" messages
- **Serial Output**: Logs press and release events

## Power Configuration

- **BLE TX Power**: +4 dBm (maximum for extended range)
- **Advertising Interval**: 32-244 (units of 0.625ms)
## Troubleshooting

### Blue LED flashing when connected to B+
- **Solution**: Use PIN_020 + GND (active-HIGH configuration) instead of B+ for power
- Current working setup doesn't need B+ connection

### PIN_017 (D2) doesn't output voltage
- Some pins have different mappings on this board
- Use raw GPIO numbers (020, 024) instead of D-numbers
- PIN_020 confirmed working

### Optocoupler not triggering
- Check polarity: Anode (Pin 1, dot side) connects to 220Ω resistor
- Verify resistor value (220Ω)
- Test with multimeter: Should see ~10-15mA when triggered
- Ensure PC817C pins 3-4 connect to key fob button contacts

### COM port changes after upload
- **Normal behavior** - bootloader uses one port, app uses another
- Check Device Manager after upload completes
- Usually changes from COM5 → COM10

### Can't find board in Device Manager
- Try unplugging and replugging USB
- Board may need to enter bootloader mode (double-tap reset if available)

## Project Structure

```
keyfob/
├── src/
│   └── main.cpp          # Main firmware (BLE control)
├── platformio.ini        # PlatformIO configuration
└── README.md            # This file
```

## Credits

Built with [PlatformIO](https://platformio.org/) and [Adafruit nRF52 Arduino Core](https://github.com/adafruit/Adafruit_nRF52_Arduino)

December 2025
