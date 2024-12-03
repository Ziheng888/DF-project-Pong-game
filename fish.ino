/*
 * DF Pong Controller - Lantern Angler(FishEYE)
 * OADU - Creation and Computation 2024 - Experiement 4
 * Ziheng Qu
 * Game Link : https://digitalfuturesocadu.github.io/df-pong/
 * 
 * Movement Values:
 * 0 = No movement / Neutral position
 * 1 = UP movement (paddle moves up)
 * 2 = DOWN movement (paddle moves down)
 * 
*/


#include <ArduinoBLE.h>
#include "ble_functions.h"
#include "buzzer_functions.h"


// Pins for light sensors
int lightPin1 = A7;
int lightPin2 = A6;

// Configuration for light smoothing and sensitivity
const int lightAverageWindow = 10; // Number of samples for rolling average
const int equalityThreshold = 150;  // Adjust this value for sensitivity

// Global variables
int lightValue1 = 0;         
int lightValue2 = 0;         
int smoothedLight1 = 0;      
int smoothedLight2 = 0;      
unsigned long lastLightReadTime = 0;
unsigned int lightReadInterval = 50; // Interval in milliseconds for reading sensors
 

// Rolling average variables
int lightReadings1[lightAverageWindow];
int lightReadings2[lightAverageWindow];
int lightReadIndex = 0;
long lightTotalValue1 = 0;
long lightTotalValue2 = 0;


// Bluetooth device and pin configurations

//Name your controller!
const char* deviceName = "FishEYES";

// Pin definitions buzzer/LED
const int BUZZER_PIN = 11;       // Pin for haptic feedback buzzer
const int LED_PIN = LED_BUILTIN; // Status LED pin

// Movement state tracking
int currentMovement = 0;         // Current movement value (0=none, 1=up, 2=down, 3=handshake)



// Setup function to initialize sensors, BLE, and peripherals
void setup() 
{
  
  Serial.begin(9600);
  pinMode(3, OUTPUT); // Output pin for indicating movement
  Serial.println("Dual Light Sensor Comparison");
  initializeLightAverage(); // Initialize rolling average arrays


  
  // Configure LED for connection status indication
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize Bluetooth Low Energy with device name and status LED
  setupBLE(deviceName, LED_PIN);
  
  // Initialize buzzer for feedback
  setupBuzzer(BUZZER_PIN);
}

// Main loop to handle sensor reading, movement detection, and BLE updates
void loop() 
{
  
  // Update BLE connection status and handle incoming data
  updateBLE();

  
  //read the inputs te determine the current state
  //results in changing the value of currentMovement
  
  //send the movement state to P5  
  sendMovement(currentMovement);

  //make the correct noise
  updateBuzzer(currentMovement); // Provide feedback using the buzzer
  readLightSensors();  // Read light sensor values and calculate averages
  handleInput(); // Process input and handle movement state
  
}

//Initialize Light Average arrays to zero
void initializeLightAverage() {
  for (int i = 0; i < lightAverageWindow; i++) {
    lightReadings1[i] = 0;
    lightReadings2[i] = 0;
  }
  lightTotalValue1 = 0;
  lightTotalValue2 = 0;
  lightReadIndex = 0;
}

// Calculate the average value of light in a range to get smoothed data
void updateLightAverage(int newValue1, int newValue2) {
  lightTotalValue1 = lightTotalValue1 - lightReadings1[lightReadIndex];
  lightReadings1[lightReadIndex] = newValue1;
  lightTotalValue1 = lightTotalValue1 + newValue1;
  
  lightTotalValue2 = lightTotalValue2 - lightReadings2[lightReadIndex];
  lightReadings2[lightReadIndex] = newValue2;
  lightTotalValue2 = lightTotalValue2 + newValue2;
  
  lightReadIndex = (lightReadIndex + 1) % lightAverageWindow;
  
  smoothedLight1 = lightTotalValue1 / lightAverageWindow;
  smoothedLight2 = lightTotalValue2 / lightAverageWindow;
}

// Read values from light sensors and calculate smoothed averages
void readLightSensors() {
  unsigned long currentTime = millis();
  if (currentTime - lastLightReadTime >= lightReadInterval) { / Ensure minimum interval
    lightValue1 = analogRead(lightPin1); // Read raw value from sensor 1
    lightValue2 = analogRead(lightPin2); // Read raw value from sensor 2
    
    updateLightAverage(lightValue1, lightValue2); // Update smoothed values
    printLightValues(); // Debugging output for testing
    
    lastLightReadTime = currentTime; // Update last read time
  }
}

// For Testing
void printLightValues() {
  Serial.print("Smoothed: ");
  Serial.print(smoothedLight1); // Output smoothed value for sensor 1
  Serial.print("\nSmoothed: ");
  Serial.print(smoothedLight2); // Output smoothed value for sensor 2
  Serial.print("\nDarker sensor: ");
  
  // Calculate difference of 2 sensor and change the movement of the paddle
  int difference = abs(smoothedLight1 - smoothedLight2); // Calculate absolute difference
  if (difference <= equalityThreshold) { // Check if within threshold
    Serial.println("Equal (within threshold)");
    currentMovement = 0; // Neutral position
    digitalWrite(3, LOW); // Turn off movement indicator
  } else if (smoothedLight1 < smoothedLight2) { // Sensor 1 detects darker light
    Serial.println("Sensor 1");
    currentMovement = 1; // Move paddle up
    digitalWrite(3, HIGH); // Turn on movement indicator
  } else { // Sensor 2 detects darker light
    Serial.println("Sensor 2");
    currentMovement = 2; //Move paddle down
    digitalWrite(3, HIGH); // Turn on movement indicator
  }
  Serial.print("Current Movement: ");
  Serial.print(currentMovement); // Output current movement state
}