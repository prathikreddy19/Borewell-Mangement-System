#define BLYNK_PRINT Serial  // Define BLYNK_PRINT to enable serial output for Blynk
#include <ESP8266WiFi.h>   // Include the library for ESP8266 Wi-Fi functionality
#include <BlynkSimpleEsp8266.h> // Include the Blynk library for ESP8266

// Wi-Fi credentials
char ssid[] = "Sarvavyapi"; // Wi-Fi SSID
char pass[] = "Sairam#009"; // Wi-Fi password

// Blynk authentication token
char auth[] = "cLjui-Mq7V4_EzqTGT-YyVV2QKanmajF";

// Define pin connections
#define RELAY_PIN D5           // Relay control pin
#define MANUAL_BUTTON_PIN D6   // Manual button pin
#define MODE_BUTTON_PIN D0     // Mode switch button pin

// Timing constants
const uint32_t INTERVAL = 5000; // Interval for auto mode relay toggling
const uint32_t DEBOUNCE_DELAY = 50; // Debounce delay for button presses

// State variables
bool relayState = false; // Current state of the relay (ON/OFF)
bool autoMode = false;   // Flag for automatic mode
uint32_t previousMillis = 0; // Stores last toggle time for auto mode
uint32_t lastDebounceTime = 0; // Last time the mode button state changed
int lastModeButtonState = HIGH; // Previous state of the mode button
int modeButtonState; // Current state of the mode button

// Virtual pin assignments for Blynk
#define VPIN_RELAY_CONTROL V1 // Virtual pin for relay control
#define VPIN_MODE_CONTROL V2   // Virtual pin for mode control
#define VPIN_RELAY_STATUS V3   // Virtual pin for relay status
#define VPIN_MODE_STATUS V4    // Virtual pin for mode status

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud
  
  // Configure pin modes
  pinMode(RELAY_PIN, OUTPUT); // Set relay pin as output
  pinMode(MANUAL_BUTTON_PIN, INPUT_PULLUP); // Set manual button pin as input with pull-up resistor
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP); // Set mode button pin as input with pull-up resistor
  
  digitalWrite(RELAY_PIN, HIGH); // Set relay to OFF state (active-low)
  
  Blynk.begin(auth, ssid, pass); // Initialize Blynk connection
  
  Serial.println("System initialized in Manual Mode"); // Log initialization message
  updateBlynkStatus(); // Update Blynk status on startup
}

void loop() {
  Blynk.run(); // Keep Blynk running
  checkModeButton(); // Check the mode button state
  
  // Handle the mode based on the current setting
  if (autoMode) {
    handleAutoMode(); // Handle automatic mode logic
  } else {
    handleManualMode(); // Handle manual mode logic
  }
}

// Function to check the state of the mode button
void checkModeButton() {
  int reading = digitalRead(MODE_BUTTON_PIN); // Read the current state of the mode button
  
  // Check if the button state has changed
  if (reading != lastModeButtonState) {
    lastDebounceTime = millis(); // Record the time of the change
  }
  
  // If enough time has passed since the last change
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // If the state has changed (debounced)
    if (reading != modeButtonState) {
      modeButtonState = reading; // Update the mode button state
      // If the button is pressed (LOW)
      if (modeButtonState == LOW) {
        toggleMode(); // Toggle the mode (auto/manual)
      }
    }
  }
  
  lastModeButtonState = reading; // Store the current reading for the next loop
}

// Function to toggle between manual and automatic modes
void toggleMode() {
  autoMode = !autoMode; // Switch the mode state
  if (autoMode) {
    previousMillis = millis(); // Reset the previousMillis for the auto mode
    Serial.println("Switched to Auto Mode"); // Log mode switch
  } else {
    Serial.println("Switched to Manual Mode"); // Log mode switch
  }
  updateBlynkStatus(); // Update Blynk with the new mode status
}

// Function to handle automatic mode
void handleAutoMode() {
  uint32_t currentMillis = millis(); // Get the current time
  // Check if the interval has passed
  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis; // Update the last toggle time
    toggleRelay(); // Toggle the relay state
  }
}

// Function to handle manual mode
void handleManualMode() {
  // Check if the manual button is pressed
  if (digitalRead(MANUAL_BUTTON_PIN) == LOW) {
    delay(DEBOUNCE_DELAY); // Delay for debouncing
    // Check again if the button is still pressed
    if (digitalRead(MANUAL_BUTTON_PIN) == LOW) {
      toggleRelay(); // Toggle the relay state
      // Wait until the button is released
      while (digitalRead(MANUAL_BUTTON_PIN) == LOW) {
      }
    }
  }
}

// Function to toggle the relay state
void toggleRelay() {
  relayState = !relayState; // Toggle the relay state
  digitalWrite(RELAY_PIN, relayState ? LOW : HIGH); // Set relay pin accordingly (active-low)
  Serial.println(relayState ? "Relay ON" : "Relay OFF"); // Log the relay state
  updateBlynkStatus(); // Update Blynk with the new relay status
}

// Function to update Blynk status for relay and mode
void updateBlynkStatus() {
  Blynk.virtualWrite(VPIN_RELAY_STATUS, relayState); // Send relay status to Blynk
  Blynk.virtualWrite(VPIN_MODE_STATUS, autoMode); // Send mode status to Blynk
}

// Function to handle virtual pin writes for relay control
BLYNK_WRITE(VPIN_RELAY_CONTROL) {
  if (!autoMode) { // Only allow control if in manual mode
    relayState = param.asInt(); // Get the relay state from Blynk
    digitalWrite(RELAY_PIN, relayState ? LOW : HIGH); // Set the relay pin
    updateBlynkStatus(); // Update Blynk with the new relay status
  }
}

// Function to handle virtual pin writes for mode control
BLYNK_WRITE(VPIN_MODE_CONTROL) {
  autoMode = param.asInt(); // Get the mode state from Blynk
  if (autoMode) {
    previousMillis = millis(); // Reset the time for auto mode
  }
  updateBlynkStatus(); // Update Blynk with the new mode status
}
