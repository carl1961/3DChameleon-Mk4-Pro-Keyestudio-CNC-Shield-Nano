#include <EEPROM.h>
#include <Wire.h>
#include "src/SSD1306Ascii/src/SSD1306Ascii.h"            // Library for text-only OLED display
#include "src/SSD1306Ascii/src/SSD1306AsciiWire.h"
#include "src/AccelStepper/src/AccelStepper.h"            // Library for stepper motor control with acceleration
#include "src/DigitalSignal/src/DigitalSignals.h"         // Library for debouncing digital inputs

// I2C display configuration
#define I2C_ADDRESS 0x3C                                  // I2C address of the OLED display
SSD1306AsciiWire oled;                                    // OLED display object
char infoBuf[64];                                         // Buffer for formatting display text

// Stepper position definitions in steps
#define STEP_POS_A 0                                      // Position of extruder A
#define STEP_POS_B 400                                    // Position of extruder B
#define STEP_POS_C 800                                    // Position of extruder C
#define STEP_POS_D 1200                                   // Position of extruder D

#define HOME_POSITION 1150                                // Home position for the selector (steps)
#define MAX_STEPS 1240                                    // Maximum steps for the selector

// EEPROM address definitions
#define PREV_EXTR_EE 0                                    // EEPROM address for storing previous extruder
#define FEED_LENTH_EE 4                                   // EEPROM address for storing feed length

const char* extruderName[] = {"A", "B", "C", "D"};        // Names for the extruders

// Structure to define extruder positions and behaviors
typedef struct {
  int position;                                           // Position in steps for selection
  int release;                                            // Position in steps after selection (release position)
  int direction;                                          // Direction for feeding filament (0=reverse, 1=forward)
} Select_t;

// Configuration for each extruder position
Select_t positions[] = {
  { STEP_POS_A, 900, 1 },                                 // Extruder A: select at 0, release at 900, feed direction forward
  { STEP_POS_B, 900, 1 },                                 // Extruder B: select at 400, release at 900, feed direction forward
  { STEP_POS_C, 300, 0 },                                 // Extruder C: select at 800, release at 300, feed direction reverse
  { STEP_POS_D, 300, 0 }                                  // Extruder D: select at 1200, release at 300, feed direction reverse
};

// Pin definitions
const byte inputSelection = 12;                           // Pin for input selection 
const byte endstopSelector = 13;                          // Pin for selector endstop 
const byte enableSteppers = 8;                            // Pin for enabling/disabling stepper drivers
const byte stepSelector = 5;                              // Step pin for selector stepper
const byte dirSelector = 2;                               // Direction pin for selector stepper
const byte stepFeeder = 6;                                // Step pin for feeder stepper
const byte dirFeeder = 3;                                 // Direction pin for feeder stepper

const byte buzzer = A0;                                   // Pin for buzzer
DigitalIn buttonSelector(11, INPUT_PULLUP, 200);          // Debounced input for manual selector button

// Selector stepper variables
long selectorTarget = 0;                                  // Target position for selector
bool selecting = false;                                   // Flag to indicate selection in progress

// Extruder state variables
int8_t previousExtruder = -1;                             // Currently active extruder
int8_t nextExtruder = -1;                                 // Next extruder to select

// Feeder stepper variables
long feederTarget = 6400;                                 // Default feed length (can be changed via serial)
bool feederDirection = false;                             // Current feed direction
bool toolSelected = false;                                // Flag indicating if a tool is selected

// Timing variables for input handling
uint32_t startSelectionTime;                              // Time when selection started
uint32_t selectionTime;                                   // Duration of selection button press
uint32_t beepTime;                                        // Time for next beep

// State machine for input handling
enum InputStates { IDLE, START, STOP, WAIT, SELECT};      // Possible states for input handling
volatile bool isNewInput;                                 // Flag for new input

uint8_t oldState = 255;                                   // Previous input state (for debugging)
uint8_t inputState = InputStates::IDLE;                   // Current input state

// Define stepper objects
AccelStepper selector(AccelStepper::DRIVER, stepSelector, dirSelector);  // Selector stepper
AccelStepper feeder(AccelStepper::DRIVER, stepFeeder, dirFeeder);        // Feeder stepper

// Helper functions for display output
void infoPrint(const char* str) {
  oled.print(str);                                        // Display on OLED
}

void infoPrintln(const char* str) {
  oled.println(str);                                      // Display on OLED with line break
}

// Update the OLED display with current extruder information
void updateDisplay(const char* str, const char* actualExtruder) {
  oled.clear();                                           // Clear display
  infoPrint("  Previous   |      Next\n");                // Header row
  oled.set2X();                                           // Double size text
  snprintf(infoBuf, sizeof(infoBuf), "    %s        %s", extruderName[previousExtruder], actualExtruder);
  infoPrintln(infoBuf);                                   // Show previous and next extruder
  oled.set1X();                                           // Back to normal text size
  infoPrint(str);                                         // Show status message
}

// Home the selector to find its zero position
void selectorHoming() {
  uint32_t timeout = millis();                            // Start timeout timer

  infoPrintln("Homing selector");
  selectorTarget = MAX_STEPS;                             // Move to max position
  selector.setMaxSpeed(300);                              // Set slower speed for homing
  selector.setAcceleration(500);                          // Set acceleration for homing

  // Enable stepper drivers
  digitalWrite(enableSteppers, LOW);                      // LOW enables the drivers
  
  // If already at endstop, move away first
  if (digitalRead(endstopSelector) == LOW) {              // LOW means endstop is triggered
    selector.moveTo(200);                                 // Move away from endstop
    while (selector.distanceToGo() != 0) {                // Wait until motion complete
      selector.run();
      if (millis() - timeout > 5000) {                    // 5 second timeout
        infoPrintln("Homing timeout");
        return;
      }
    }
  }
  
  // Now move toward endstop
  timeout = millis();                                     // Reset timeout
  selector.moveTo(selectorTarget);                        // Move to max position
  while (selector.distanceToGo() != 0) {                  // Continue moving until done or endstop hit
    selector.run();

    if (digitalRead(endstopSelector) == LOW) {            // If endstop hit
      selector.stop();                                    // Stop motion
      infoPrintln("Endstop reached");
      selector.setCurrentPosition(HOME_POSITION);         // Set current position as home
      break;
    }

    if (millis() - timeout > 5000) {                      // 5 second timeout
      infoPrintln("Homing timeout");
      break;    
    }
  }

  // Reset for normal operation and release
  selector.setMaxSpeed(1000);                             // Return to normal speed
  selector.setAcceleration(5000);                         // Return to normal acceleration
  releaseExtruder(previousExtruder);                      // Move to release position

  digitalWrite(enableSteppers, HIGH);                     // Disable steppers to save power
  delay(500);
}

// Unload filament from current extruder
void unloadFilament(int32_t numSteps){
  feeder.move(feederDirection ? numSteps : -numSteps);    // Move in appropriate direction based on extruder config
  feeder.runToPosition();                                 // Execute move and wait for completion
}

// Load filament into next extruder
void loadFilament(int32_t numSteps) {
  feeder.move(feederDirection ? -numSteps  : numSteps);   // Move in opposite direction of unload
  feeder.runToPosition();                                 // Execute move and wait for completion
}

// Move selector to the specified extruder position
void doSelection(uint8_t index) {
  digitalWrite(enableSteppers, LOW);                      // Enable stepper drivers
  selectorTarget = positions[index].position;             // Get position for this extruder
  feederDirection = positions[index].direction;           // Set feed direction for this extruder
  selector.moveTo(selectorTarget);                        // Start motion
  selector.runToPosition();                               // Wait for completion
  selecting = true;                                       // Set flag indicating selection in progress
}

// Move selector to release position after selection
void releaseExtruder(uint8_t index) {
  selectorTarget = positions[index].release;              // Get release position for this extruder
  digitalWrite(buzzer, HIGH);                             // Short beep
  delay(20);
  digitalWrite(buzzer, LOW);

  updateDisplay("     Ready to work", " ");               // Update display status
  selector.moveTo(selectorTarget);                        // Move to release position
  selector.runToPosition();                               // Wait for completion
}

// Perform complete extruder change sequence
bool selectExtruder(int extruder) {
  if (extruder < 0 || extruder == previousExtruder)       // Validate input
    return false;

  updateDisplay("Unload prev. extruder", extruderName[nextExtruder]);
  doSelection(previousExtruder);                          // Position selector at current extruder
  unloadFilament(feederTarget);                           // Unload filament from current extruder

  updateDisplay("Load next extruder", extruderName[nextExtruder]);
  doSelection(extruder);                                  // Position selector at new extruder
  loadFilament(feederTarget + 800);                       // Load filament with extra length for purging

  previousExtruder = extruder;                            // Update current extruder
  EEPROM.put(PREV_EXTR_EE, previousExtruder);            // Save to EEPROM
  releaseExtruder(extruder);                              // Move to release position
  return true;
}

// Handle hall sensor (or switch) input with state machine
void readInput() {
  // State machine for handling input selection
  switch (inputState) {
    case InputStates::WAIT:                               // Waiting for button release after selection
      if (digitalRead(inputSelection) == LOW) {           // Button pressed again
        inputState = InputStates::SELECT;                 // Move to action state
      }
      break;

    case InputStates::IDLE:                               // Waiting for button press
      if (digitalRead(inputSelection) == LOW) {           // Button pressed
        startSelectionTime = millis();                    // Record start time
        updateDisplay("Start selection input", " ");
        inputState = InputStates::START;                  // Move to start state
        beepTime = millis();                              // Initialize beep timer
      }
      break;

    case InputStates::START:                             // Button being held down
      if (digitalRead(inputSelection) == HIGH) {         // Button released
        selectionTime = millis() - startSelectionTime;   // Calculate hold duration
        inputState = InputStates::STOP;                  // Move to stop state
      }

      if (millis() - beepTime >= 500) {                  // Beep every 500ms while holding
        beepTime = millis();
        digitalWrite(buzzer, HIGH);
        delay(20);
        digitalWrite(buzzer, LOW);
      }
      break;

    case InputStates::STOP: {                            // Button released, determine action
      // Different hold durations select different extruders
      if (selectionTime < 500) {                         // Too short, ignore
        nextExtruder = -1;
        inputState = InputStates::IDLE;
      }
      else if (selectionTime >= 500 && selectionTime < 1000)
        nextExtruder = 0;                                // Select extruder A
      else if (selectionTime >= 1000 && selectionTime < 1500)
        nextExtruder = 1;                                // Select extruder B
      else if (selectionTime >= 1500 && selectionTime < 2000)
        nextExtruder = 2;                                // Select extruder C
      else if (selectionTime >= 2000 && selectionTime < 2500)
        nextExtruder = 3;                                // Select extruder D
      else if (selectionTime >= 3200) {                  // Very long press: home sequence
        nextExtruder = 0;
        previousExtruder = 0;
        EEPROM.put(PREV_EXTR_EE, previousExtruder);     // Save to EEPROM
        selectorHoming();                                // Run homing sequence
        delay(500);
        updateDisplay("Waiting for input", " ");
        inputState = InputStates::IDLE;
      }

      if (nextExtruder > -1) {                          // If extruder selected, wait for confirmation
        inputState = InputStates::WAIT;
        updateDisplay("Tool selected wait start", extruderName[nextExtruder]);
      }
      break;
    }
  }
}

// Process serial commands for tool selection and feed length
void getCommandFromSerial() {
  if (Serial.available()) {                            // Check for serial input
    String val = Serial.readStringUntil('\n');         // Read command line

    if (val.startsWith("T")) {                         // Tool selection command
      // Get the tool number 'T'
      nextExtruder = val.substring(1).toInt();         // Extract tool number
      if (nextExtruder >= 0 && nextExtruder < 4) {     // Validate tool number
        selectExtruder(nextExtruder);                  // Select the extruder
      }
    }
    else if (val.startsWith("S")) {                    // Set feed length command
      feederTarget = val.substring(1).toInt();         // Extract feed length
      oled.clear();                                    // Update display
      oled.print("Update feeder lenght");
      oled.print("Step: ");
      oled.println(feederTarget);
      EEPROM.put(FEED_LENTH_EE, feederTarget);         // Save to EEPROM
      delay(500);
      updateDisplay("Waiting for input", " ");
    }
    else if (val.startsWith("H")) {                    // Home command
      selectorHoming();                                // Run homing sequence
      delay(500);
      updateDisplay("Waiting for input", " ");
    }
  }
}

// Handle manual extruder selection using push button
void handleManualSelection() {
  buttonSelector.update();                             // Update debounced button state
  if (buttonSelector.click()) {                        // Short click detected
    digitalWrite(buzzer, HIGH);                        // Acknowledge click with beep
    delay(10);
    digitalWrite(buzzer, LOW);
    nextExtruder = ++nextExtruder % 4;                 // Cycle through extruders (0-3)
    Serial.print("Select T");
    Serial.println(nextExtruder);
    updateDisplay("Waiting for input", extruderName[nextExtruder]);
  }
  if (buttonSelector.longClick()) {                    // Long click detected
    Serial.println("Start selection");
    inputState = InputStates::SELECT;                  // Trigger selection state
  }
}

// Arduino setup function - runs once at startup
void setup() {
  // Load saved settings from EEPROM
  EEPROM.get(PREV_EXTR_EE, previousExtruder);          // Get last used extruder
  EEPROM.get(FEED_LENTH_EE, feederTarget);             // Get feed length
  if (feederTarget < 6400) {                           // Sanity check on feed length
    feederTarget = 6400;                               // Set default if invalid
    EEPROM.put(FEED_LENTH_EE, feederTarget);           // Save to EEPROM
  }

  // Configure pins
  pinMode(inputSelection, INPUT_PULLUP);               // Button with internal pullup
  pinMode(endstopSelector, INPUT_PULLUP);              // Endstop with internal pullup
  digitalWrite(enableSteppers, HIGH);                  // Disable steppers initially (HIGH = disabled)
  pinMode(enableSteppers, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Power-on beep sequence
  for (int i=0; i<3; i++) {
    digitalWrite(buzzer, HIGH);
    delay(20);
    digitalWrite(buzzer, LOW);
    delay(200);
  }

  // Setup display
  Wire.begin();
  Wire.setClock(400000L);                              // Fast I2C for display

  oled.begin(&SH1106_128x64, I2C_ADDRESS);             // Initialize display
  oled.setFont(Arial14);                               // Set font
  oled.clear();

  Serial.begin(115200);                                // Start serial communication
  infoPrintln("Starting");
  Serial.print("Current feederTarget: ");
  Serial.println(feederTarget);
  selectorHoming();                                    // Initialize by homing the selector mechanism

  // Configure feeder stepper parameters
  feeder.setMaxSpeed(2500);                            // Set maximum speed for feeder motor
  feeder.setAcceleration(5000);                        // Set acceleration for feeder motor
  updateDisplay("Waiting for input", " ");             // Show initial display message
}

// Main program loop - runs continuously
void loop() {
  selector.run();                                      // Non-blocking stepper motor control
  feeder.run();                                        // Non-blocking stepper motor control  

  // Handle input selection using hall effect sensor or switch
  readInput();                                         // Process input from selection sensor
  if (inputState == InputStates::SELECT) {             // Selection action requested
    uint32_t startMoveTime = millis();                 // Start timing the operation
    if (previousExtruder != nextExtruder) {            // Only change if different extruder
      digitalWrite(buzzer, HIGH);                      // Acknowledge with beep
      delay(10);
      digitalWrite(buzzer, LOW);
      selectExtruder(nextExtruder);                    // Perform complete tool change
    }
    inputState = InputStates::IDLE;                    // Return to idle state
    Serial.print("Selection elapsed time: ");          // Report time taken
    Serial.println((millis() - startMoveTime) / 1000.0, 2);
  }

  // Handle filament loading after selection
  if ((selector.distanceToGo() != 0) && selecting) {   // If selector is moving and selection in progress
    selecting = false;                                 // Clear selection flag
    delay(100);                                        // Brief delay
    loadFilament(feederTarget);                        // Load filament into the selected extruder
  }

  // Power saving - disable steppers when not moving
  if (feeder.distanceToGo() == 0 && selector.distanceToGo() == 0) {
    digitalWrite(enableSteppers, HIGH);                // Disable stepper drivers when idle
  }

  // Handle manual extruder selection using push button
  handleManualSelection();

  // Process serial commands for tool selection and feed length
  getCommandFromSerial();
}

