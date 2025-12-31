# NRF52840 KeyFob BLE Trigger (Dual Button + Authentication)

**⚠️ PROJECT STATUS: VIBE CODED ⚠️**

This project was developed iteratively through experimentation and troubleshooting. While functional, it may contain non-optimal patterns, security considerations, and edge cases that haven't been fully tested. Use at your own risk, especially for security-critical applications.

Control your car key fob (LOCK and UNLOCK buttons) remotely via Bluetooth with BLE pairing authentication!

## Table of Contents
- [Hardware](#hardware)
- [Security Model](#security-model)
- [Wiring](#wiring)
- [Features](#features)
- [Quick Start](#quick-start)
- [Code Architecture](#code-architecture)
- [BLE Security Implementation](#ble-security-implementation)
- [Power Management](#power-management)
- [Pin Definitions](#pin-definitions)
- [Configuration](#configuration)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Known Issues & Vulnerabilities](#known-issues--vulnerabilities)
- [Development Notes](#development-notes)
- [Future Improvements](#future-improvements)

## Hardware

- **Board**: NRF52840 SuperMini (NiceNano clone from AliExpress)
  - MCU: Nordic NRF52840 (ARM Cortex-M4F @ 64MHz)
  - RAM: 256KB
  - Flash: 1MB
  - BLE: Bluetooth 5.0, Bluetooth mesh
  - USB: Native USB 2.0
  - Pins: Exposed GPIO pins (not all are safe to use)
  
- **Optocouplers**: 2x PC817C (4-pin DIP)
  - Purpose: Electrical isolation between MCU and key fob circuits
  - Current Transfer Ratio: 50-600% (varies by unit)
  - Forward Voltage: ~1.2V (LED side)
  - Forward Current: 10-50mA (we use ~9.5mA)
  
- **Resistors**: 2x 220Ω (1/4W)
  - Purpose: Current limiting for optocoupler LEDs
  - Tolerance: ±5% acceptable
  - Alternative values: 180Ω - 330Ω work fine
  
- **Power Switch**: MSK-12C02 SPDT slide switch
  - Rating: 300mA @ 50V DC
  - Pins: 3 pins (Common, NO, NC)
  
- **Battery**: 3.7V LiPo (tested with 301230 ~130mAh)
  - Voltage range: 3.0V - 4.2V
  - Discharge rate: 1C typical
  - No built-in protection circuit on board - battery must have protection
  
- **Key Fob**: Any car key with physical button contacts

## Security Model

### BLE Pairing Authentication

**Design Decision**: Use BLE Security Manager Protocol (SMP) with MITM protection instead of application-layer passwords.

**Why BLE Pairing Instead of Password?**
1. **Built-in encryption**: All communication automatically encrypted after pairing
2. **One-time setup**: Pair once, auto-connect forever (until bond cleared)
3. **OS-level integration**: Uses phone's native Bluetooth pairing UI
4. **Key storage**: Pairing keys stored in flash, survives reboots
5. **Better UX**: No need to type password every connection

**Security Level**: `SECMODE_ENC_WITH_MITM`
- **ENC** = Encrypted connection required
- **MITM** = Man-In-The-Middle protection via 6-digit PIN
- This forces passkey authentication before allowing UART communication

**Pairing Flow**:
1. Phone connects → BLE stack requests security upgrade
2. Board generates random 6-digit PIN (000000-999999)
3. PIN displayed on Serial Monitor and sent via BLE UART
4. User enters PIN on phone
5. Cryptographic key exchange (ECDH P-256)
6. Bonding keys stored in both devices
7. Future connections: automatic authentication using stored keys

**Bond Storage**:
- Location: NRF52840 internal flash (non-volatile)
- Survives: Power cycles, battery changes, disconnects
- Cleared by: Re-flashing firmware, manual bond clear, "Forget Device" on phone

### Known Security Limitations

⚠️ **VULNERABILITIES & RISKS**:

1. **PIN Display Over BLE UART**
   - The 6-digit PIN is sent over unencrypted BLE UART initially
   - **Risk**: Attacker in range could intercept PIN during first pairing
   - **Mitigation**: Only display on Serial Monitor (USB), not over BLE
   - **Status**: Current code sends to both (vulnerable)
   
2. **No Bond Whitelist**
   - Any device can attempt pairing if bond is cleared
   - **Risk**: If you "forget" device, anyone nearby can pair
   - **Mitigation**: Implement bond whitelist, or don't forget device
   - **Status**: Not implemented
   
3. **Physical Access**
   - USB port allows firmware re-flash without authentication
   - **Risk**: Attacker with physical access can load malicious firmware
   - **Mitigation**: Secure bootloader, enclosure with tamper detection
   - **Status**: Not implemented
   
4. **Replay Attacks**
   - Commands sent over BLE could theoretically be recorded and replayed
   - **Risk**: Low (BLE encryption includes replay protection)
   - **Mitigation**: BLE 4.2+ has built-in anti-replay
   - **Status**: Relies on BLE stack implementation
   
5. **Optocoupler Failure**
   - PC817C can fail shorted (rare) instead of open
   - **Risk**: Stuck button press drains key fob battery
   - **Mitigation**: Watchdog timer, current monitoring
   - **Status**: No protection implemented
   
6. **Battery Over-discharge**
   - No low-battery warning or cutoff in code
   - **Risk**: LiPo damaged if discharged below 3.0V
   - **Mitigation**: Use battery with protection circuit, add voltage monitoring
   - **Status**: Battery must have protection circuit
   
7. **RF Jamming**
   - BLE 2.4GHz can be jammed (same as WiFi)
   - **Risk**: Device becomes non-functional if jammed
   - **Mitigation**: None possible at application layer
   - **Status**: Inherent to BLE technology

### Threat Model

**Assumptions**:
- Attacker does not have physical access to device
- Legitimate user pairs device in secure location (home)
- Key fob uses rolling codes (modern cars)
- Battery has protection circuit

**Out of Scope**:
- Car security vulnerabilities (relay attacks, rolling code replay)
- RF signal capture/replay from car itself
- Physical tampering with installed device
- Bluetooth chip-level vulnerabilities

**LOCK Button (Optocoupler 1):**
```
NRF52840 PIN 020 ──[220Ω]── PC817C Pin 1 (Anode, dot side)
NRF52840 GND ────────────── PC817C Pin 2 (Cathode)
PC817C Pin 3&4 ────────────── Key fob LOCK button
```

**UNLOCK Button (Optocoupler 2):**
```
NRF52840 PIN 022 ──[220Ω]── PC817C Pin 1 (Anode, dot side)
NRF52840 GND ────────────── PC817C Pin 2 (Cathode)
PC817C Pin 3&4 ────────────── Key fob UNLOCK button
```

**Active-HIGH Configuration:**
- PIN HIGH = optocoupler ON = button pressed
- PIN LOW = optocoupler OFF = button released

## Features

- ✅ **Dual button control** - Lock and Unlock
- ✅ **Password authentication** - Only authorized devices can connect (default: 1234)
- ✅ BLE UART communication
- ✅ Bluefruit Controller button support
- ✅ Optocoupler isolation for safety
- ✅ Visual LED feedback
- ✅ No blue LED flashing

### Connection Instructions

**First-time setup:**
1. Open Bluefruit Connect app
2. Connect to **"KeyFob"**
3. Go to **UART** mode
4. **Enter password: 1234**
5. You'll see "ACCESS GRANTED"
6. Switch to **Controller** mode
7. **Button 1 = LOCK**, **Button 2 = UNLOCK**

**Commands:**
- UART: `lock` or `1` = Lock
- UART: `unlock` or `2` = Unlock
- Controller: Button 1 = Lock, Button 2 = Unlock
## Quick Start

### 1. Install PlatformIO
- Install [VS Code](https://code.visualstudio.com/)
- Install PlatformIO IDE extension

### 2. Clone & Build
```bash
git clone https://github.com/mtvnnb/my_keyfob.git
cd my_keyfob
pio run
```

### 3. Upload
```bash
# Board detected on COM5
pio run -t upload --upload-port COM5
```
**Note**: Board switches to different COM port after upload (usually COM10).

### 4. Use Phone App
1. Download **"Bluefruit Connect"** (iOS/Android)
2. Connect to **"KeyFob"**
3. **Enter password: 1234** (via UART or Controller)
4. Tap **Controller** → **Control Pad**
5. **Button 1 = LOCK**, **Button 2 = UNLOCK**

## Usage

### Authentication

**First connect** - you must authenticate:
1. Connect to "KeyFob" in Bluefruit Connect
2. Go to **UART** mode
3. Send password: **1234**
4. You'll see "ACCESS GRANTED"

**Default password**: `1234` (change in `src/main.cpp`)

### Commands

**Via Bluefruit Controller (after auth):**
- **Button 1** = Lock car
- **Button 2** = Unlock car
- Buttons 3 & 4 = Not assigned

**Via UART/Text:**
- `1234` = Authenticate (first time)
- `lock` or `1` = Lock car
- `unlock` or `2` = Unlock car

## Configuration

**Change Password** (`src/main.cpp`):
```cpp
const String PASSWORD = "1234";  // Change to your password
```

**Change Button Hold Time** (`src/main.cpp`):
```cpp
delay(300);  // Change to desired milliseconds
```

**COM Ports** (`platformio.ini`):
- `upload_port = COM5` - Upload port (board detected)
- `monitor_port = COM5` - Serial monitor port

## Pin Definitions

| Pin | Function | Description |
|-----|----------|-------------|
| P0.20 (020) | LOCK_PIN | Controls lock optocoupler (via 220Ω resistor) |
| P0.22 (022) | UNLOCK_PIN | Controls unlock optocoupler (via 220Ω resistor) |
| P0.15 (015) | STATUS_LED | Red LED indicator |
| GND | Ground | Reference for both optocoupler cathodes |
- **Status LED**: Turns on during press, off after release
- **BLE Response**: Confirms action with "Pressing button..." and "Done!" messages
- **Serial Output**: Logs press and release events

## Power Configuration

- **BLE TX Power**: +4 dBm (maximum for extended range)
- **Advertising Interval**: 32-244 (units of 0.625ms)
## Troubleshooting

### Can't authenticate / Wrong password
- Make sure to type the exact password: `1234`
- Check serial monitor for authentication status
- Reconnect to BLE if stuck

### Only one button works
- Verify both optocouplers are wired correctly
- Check PIN 022 is connected to second optocoupler
- Test each pin individually with multimeter

### Blue LED flashing when connected to B+
- **Solution**: Use PIN_020/022 + GND (active-HIGH) instead of B+
- Current working setup doesn't need B+ connection

### PIN_017 (D2) doesn't output voltage
- Some pins have different mappings on this board
- Use raw GPIO numbers (020, 022) instead of D-numbers
- PIN_020 and PIN_022 confirmed working

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
