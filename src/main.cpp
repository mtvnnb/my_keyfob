/*
 * Car Key Fob Trigger - BLE Control
 * 
 * WIRING:
 * PIN 020 ──[220Ω]── PC817C Pin 1 (Anode, dot side)
 * GND ────────────── PC817C Pin 2 (Cathode)
 * PC817C Pin 3 ───── Key fob button wire A
 * PC817C Pin 4 ───── Key fob button wire B
 * 
 * PHONE APP: Use "nRF Connect" or "Bluefruit Connect"
 * - Connect to "KeyFob"
 * - Send "p" or "press" to trigger button
 */

#include <Arduino.h>
#include <bluefruit.h>

#define TRIGGER_PIN 20  // P0.20 - controls optocoupler (with 220Ω resistor)
#define STATUS_LED 15   // P0.15 - red LED

// BLE UART Service
BLEUart bleuart;

void pressButton() {
  // HIGH = current flows through optocoupler = button pressed
  digitalWrite(TRIGGER_PIN, HIGH);
}

void releaseButton() {
  // LOW = no current = button released  
  digitalWrite(TRIGGER_PIN, LOW);
}

void triggerKeyFob() {
  Serial.println(">>> BUTTON PRESS");
  bleuart.println("Pressing button...");
  digitalWrite(STATUS_LED, HIGH);
  
  pressButton();
  delay(300);  // Hold button for 300ms
  releaseButton();
  
  digitalWrite(STATUS_LED, LOW);
  Serial.println(">>> BUTTON RELEASED");
  bleuart.println("Done!");
}

// BLE connect callback
void connect_callback(uint16_t conn_handle) {
  Serial.println("BLE Connected!");
  bleuart.println("KeyFob Ready! Send 'p' to press button.");
}

// BLE disconnect callback
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.println("BLE Disconnected");
}

void setupBLE() {
  Bluefruit.begin();
  Bluefruit.setTxPower(4);  // Max power for range
  Bluefruit.setName("KeyFob");
  
  // Disable blue LED for BLE status
  Bluefruit.autoConnLed(false);
  
  // Callbacks
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  
  // Start UART service
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
  
  Serial.println("BLE advertising as 'KeyFob'");
}

void setup() {
  // Disable all LEDs first
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);
  
  // Turn off blue LED
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  
  Serial.begin(115200);
  delay(500);
  
  Serial.println("===========================================");
  Serial.println("  KEY FOB TRIGGER - BLE");
  Serial.println("===========================================");
  
  // Setup trigger pin
  pinMode(TRIGGER_PIN, OUTPUT);
  releaseButton();  // Start released (LOW)
  
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
}

void loop() {
  // Check for BLE commands
  if (bleuart.available()) {
    String cmd = bleuart.readString();
    cmd.trim();
    
    Serial.print("Received: ");
    Serial.println(cmd);
    
    // Bluefruit Connect Controller buttons (Button 1-4)
    // Format: !B11 = Button 1 pressed, !B10 = Button 1 released
    if (cmd.indexOf("!B11") >= 0 || cmd.indexOf("!B21") >= 0 || 
        cmd.indexOf("!B31") >= 0 || cmd.indexOf("!B41") >= 0) {
      triggerKeyFob();
    }
    // Also support text commands
    else if (cmd == "p" || cmd == "press" || cmd == "1") {
      triggerKeyFob();
    } 
    else if (cmd.length() > 0 && cmd[0] != '!') {
      bleuart.println("Use Controller buttons or send: p");
    }
  }
}
