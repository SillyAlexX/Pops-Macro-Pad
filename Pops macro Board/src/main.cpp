#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keyboard.h>

// --- Configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  
#define SCREEN_ADDRESS 0x3C 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Pin Definitions ---
const int PIN_SW1 = D3; 
const int PIN_SW2 = D7;
const int PIN_SW3 = D8;
const int PIN_SW4 = D9;
const int PIN_SW5 = D10;

// --- Macro Pad State Variables ---
// Array to hold all button pins for easier looping
const int buttonPins[] = {PIN_SW1, PIN_SW2, PIN_SW3, PIN_SW4, PIN_SW5};
const int NUM_BUTTONS = sizeof(buttonPins) / sizeof(buttonPins[0]);

// Array to track the previous state of each button (LOW=pressed, HIGH=released)
int lastButtonState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH, HIGH};

// Array to hold the display name for each macro (Not displayed, but kept for future reference)
const char* macroNames[] = {"UNDO", "COPY", "PASTE", "ENTER", "REFRESH"};


// --- Helper Function to Draw Bongo Cat ---
void drawBongoCat(bool handsDown) {
  // 1. Draw the Table (Bottom line)
  display.drawLine(0, 63, 128, 63, SSD1306_WHITE);

  // 2. Draw the Head (A flattened circle/ellipse)
  // Center X=64, Y=64 (peeking from bottom)
  display.fillCircle(64, 64, 25, SSD1306_WHITE); 
  
  // Head Outline
  display.fillCircle(64, 70, 30, SSD1306_WHITE); 
  
  // Eyes (Black pixels on the white face)
  display.fillCircle(54, 55, 2, SSD1306_BLACK); // Left Eye
  display.fillCircle(74, 55, 2, SSD1306_BLACK); // Right Eye
  
  // Mouth (Simple 'w' shape using lines)
  display.drawLine(64, 58, 62, 60, SSD1306_BLACK);
  display.drawLine(64, 58, 66, 60, SSD1306_BLACK);

  // Ears (Triangles on top of head)
  display.fillTriangle(38, 50, 45, 30, 52, 48, SSD1306_WHITE); // Left Ear
  display.fillTriangle(76, 48, 83, 30, 90, 50, SSD1306_WHITE); // Right Ear

  // 3. Draw Paws (The Animation)
  if (handsDown) {
    // Paws hitting the table (Low Y value)
    display.fillEllipse(40, 60, 8, 5, SSD1306_WHITE); // Left Paw
    display.fillEllipse(88, 60, 8, 5, SSD1306_WHITE); // Right Paw
    
    // Little "impact" lines
    display.drawLine(30, 55, 35, 60, SSD1306_WHITE);
    display.drawLine(98, 55, 93, 60, SSD1306_WHITE);
  } else {
    // Paws raised in the air (High Y value)
    display.fillEllipse(35, 40, 6, 6, SSD1306_WHITE); // Left Paw
    display.fillEllipse(93, 40, 6, 6, SSD1306_WHITE); // Right Paw
  }
}

// --- NEW: Function to handle the actual macro sending ---
void handleMacro(int index, int currentState) {
  // Check if the button state has changed (debouncing logic)
  if (currentState != lastButtonState[index]) {
    // Small delay for physical debouncing
    delay(5); 
    currentState = digitalRead(buttonPins[index]);

    if (currentState == LOW) { 
      // Button was JUST PRESSED (HIGH to LOW transition)
      
      // Execute the Macro
      switch (index) {
        case 0: // SW1 -> Ctrl + Z (UNDO)
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press('z');
          break;
        case 1: // SW2 -> Ctrl + C (COPY)
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press('c');
          break;
        case 2: // SW3 -> Ctrl + V (PASTE)
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press('v');
          break;
        case 3: // SW4 -> Enter
          Keyboard.press(KEY_RETURN);
          break;
        case 4: // SW5 -> F5 (REFRESH)
          Keyboard.press(KEY_F5);
          break;
      }
      
    } else { 
      // Button was JUST RELEASED (LOW to HIGH transition)
      // Always release ALL keys to ensure modifiers (Ctrl/Alt/Shift) are reset
      Keyboard.releaseAll();
    }
    
    // Update the last state
    lastButtonState[index] = currentState;
  }
}


void setup() {
  Serial.begin(115200);
  
  // --- Button Setup ---
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // --- Display Setup (I2C) ---
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }

  Keyboard.begin();
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Initial draw of the idle Bongo Cat
  drawBongoCat(false);
  display.display(); 
}

void loop() {
  bool anyPressed = false;

  // 1. Handle all button states and macros
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int currentState = digitalRead(buttonPins[i]);
    handleMacro(i, currentState);
    
    if (currentState == LOW) {
      anyPressed = true;
    }
  }
  
  static bool lastAnyPressed = false;

  if (anyPressed != lastAnyPressed) {
    // Redraw the entire screen (clears everything, including old cat/paws)
    display.clearDisplay(); 
    
    // Draw the Bongo Cat in the new state (handsDown or handsUp)
    drawBongoCat(anyPressed);

    display.display();
    
    // Update the state tracker
    lastAnyPressed = anyPressed;
  }
  
  // Run fast enough to catch key presses and animation state changes
  delay(10); 
}