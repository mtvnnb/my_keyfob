/*
 * Car Key Fob Trigger - BLE Control
 * 
 * WIRING:
 * LOCK BUTTON (Optocoupler 1):
 *   PIN 020 ──[220Ω]── PC817C Pin 1 (Anode, dot side)
 *   GND ────────────── PC817C Pin 2 (Cathode)
 *   PC817C Pin 3&4 ─── Key fob LOCK button
 * 
 * UNLOCK BUTTON (Optocoupler 2):
 *   PIN 022 ──[220Ω]── PC817C Pin 1 (Anode, dot side)
 *   GND ────────────── PC817C Pin 2 (Cathode)
 *   PC817C Pin 3&4 ─── Key fob UNLOCK button
 * 
 * PHONE APP: "Bluefruit Connect"
 * - Button 1 = LOCK, Button 2 = UNLOCK
 * - No password required
 */

#include <Arduino.h>
#include <bluefruit.h>

#define LOCK_PIN 20     // P0.20 - controls LOCK optocoupler
#define UNLOCK_PIN 22   // P0.22 - controls UNLOCK optocoupler
#define STATUS_LED 15   // P0.15 - red LED

// BLE UART Service
BLEUart bleuart;

// Forward declarations
bool pairing_passkey_callback(uint16_t conn_handle, uint8_t const passkey[6], bool match_request);
void secured_callback(uint16_t conn_handle);

void pressLock() {
  Serial.println(">>> LOCK");
  bleuart.println("Locking...");
  digitalWrite(STATUS_LED, HIGH);
  
  digitalWrite(LOCK_PIN, HIGH);
  delay(300);
  digitalWrite(LOCK_PIN, LOW);
  
  digitalWrite(STATUS_LED, LOW);
  Serial.println(">>> LOCK COMPLETE");
  bleuart.println("Locked!");
}

void pressUnlock() {
  Serial.println(">>> UNLOCK");
  bleuart.println("Unlocking...");
  digitalWrite(STATUS_LED, HIGH);
  
  digitalWrite(UNLOCK_PIN, HIGH);
  delay(300);
  digitalWrite(UNLOCK_PIN, LOW);
  
  digitalWrite(STATUS_LED, LOW);
  Serial.println(">>> UNLOCK COMPLETE");
  bleuart.println("Unlocked!");
}

// BLE connect callback
void connect_callback(uint16_t conn_handle) {
  Serial.println("BLE Connected!");
  bleuart.println("===== KEYFOB READY =====");
  bleuart.println("Button 1 = LOCK");
  bleuart.println("Button 2 = UNLOCK");
}

// BLE disconnect callback
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.println("BLE Disconnected");
}

void setupBLE() {
  Bluefruit.begin();
  Bluefruit.setTxPower(4);  // Max power for range
  Bluefruit.setName("KeyFob");
  
  // Enable BLE Security (Bonding/Pairing) - BEFORE starting services
  Bluefruit.Security.setIOCaps(true, false, false);  // Display only (shows PIN on serial)
  Bluefruit.Security.setMITM(true);                   // Man-in-the-middle protection
  Bluefruit.Security.setPairPasskeyCallback(pairing_passkey_callback);
  Bluefruit.Security.setSecuredCallback(secured_callback);
  
  // Disable blue LED for BLE status
  Bluefruit.autoConnLed(false);
  
  // Callbacks
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  
  // CRITICAL: Set UART permissions BEFORE begin() to REQUIRE pairing
  bleuart.setPermission(SECMODE_ENC_WITH_MITM, SECMODE_ENC_WITH_MITM);  // Require pairing with MITM
  
  // Start UART service (encryption required)
  bleuart.begin();
  
  // Start advertising
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.Advertising.addName();
  
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);  // Fast advertising
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);  // Advertise forever
  
  Serial.println("BLE advertising as 'KeyFob' - SECURED");
  Serial.println("Pairing required - encryption enforced on UART");
}

// Pairing passkey callback - displays PIN to user
bool pairing_passkey_callback(uint16_t conn_handle, uint8_t const passkey[6], bool match_request) {
  Serial.println("===========================================");
  Serial.println("  PAIRING REQUEST");
  Serial.println("===========================================");
  Serial.print("Enter this PIN on your phone: ");
  for(int i=0; i<6; i++) {
    Serial.print((char)passkey[i]);
  }
  Serial.println();
  Serial.println("===========================================");
  
  // Also send to BLE UART
  bleuart.print("Pairing PIN: ");
  for(int i=0; i<6; i++) {
    bleuart.print((char)passkey[i]);
  }
  bleuart.println();
  
  return true;  // Accept pairing
}

// Secured connection callback
void secured_callback(uint16_t conn_handle) {
  Serial.println("Connection secured (encrypted & authenticated)");
  bleuart.println(">>> DEVICE PAIRED <<<");
  bleuart.println("Connection secured!");
}

void setup() {
  // CRITICAL: Enable DC/DC converter for battery operation
  // This MUST be done before Bluefruit.begin()
  #ifdef NRF_POWER_DCDC_ENABLED
    NRF_POWER->DCDCEN = 1;
  #endif
  
  // Disable all LEDs first
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);
  
  // Turn off blue LED
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  
  Serial.begin(115200);
  delay(500);
  
  Serial.println("===========================================");
  Serial.println("  KEY FOB TRIGGER - BLE (Battery Mode)");
  Serial.println("===========================================");
  
  // Setup trigger pins
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(UNLOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW);
  digitalWrite(UNLOCK_PIN, LOW);
  
  // Setup BLE
  setupBLE();
  
  // Startup blinks (red LED only)
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH);
    delay(200);
    digitalWrite(STATUS_LED, LOW);
    delay(200);
  }
  
  Serial.println("Ready! Waiting for BLE connection...");
  Serial.println("Battery power mode enabled");
}

void loop() {
  // Check for BLE commands
  if (bleuart.available()) {
    String cmd = bleuart.readString();
    cmd.trim();
    
    Serial.print("Received: ");
    Serial.println(cmd);
    
    // Button 1 = Lock
    if (cmd.indexOf("!B11") >= 0 || cmd == "lock" || cmd == "1") {
      pressLock();
    }
    // Button 2 = Unlock
    else if (cmd.indexOf("!B21") >= 0 || cmd == "unlock" || cmd == "2") {
      pressUnlock();
    }
    // Buttons 3 & 4 do nothing (ignored)
    else if (cmd.indexOf("!B31") >= 0 || cmd.indexOf("!B41") >= 0) {
      Serial.println("Button not assigned");
    }
    else if (cmd.length() > 0 && cmd[0] != '!') {
      bleuart.println("Commands: lock, unlock, 1, 2");
      bleuart.println("Or use Controller buttons 1-2");
    }
  }
}
