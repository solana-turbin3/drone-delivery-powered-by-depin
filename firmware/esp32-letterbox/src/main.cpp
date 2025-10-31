#include "pins_arduino.h"
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include "IoTxChain-lib.h"
#include "SolanaUtils.h"
#include "ssid.h"
#include "keypair.h"
#include "AnchorEvents.h"
#include <ESP32Servo.h>


Servo servoMotor;

int val;
float temperature;


// Solana RPC URL (Devnet)
const String solanaRpcUrl = "https://api.devnet.solana.com"; // or mainnet/testnet



const String PROGRAM_ID = "8NLjevMMfZDViPWAcLNJTQij4crwVddE1N8SRwwrUSsd";


// Initialize Solana Library
IoTxChain solana(solanaRpcUrl);

// Initialize Event Listener (global instance)
AnchorEventListener* eventListener = nullptr;

// Event handler callbacks
void onDroneArrivalEvent(const AnchorEvent& event);
void onLetterboxSensorEvent(const AnchorEvent& event);

// Define the built-in LED pin for Arduino Nano ESP32
// The Arduino Nano ESP32 has the built-in LED on pin 13
#define LED_PIN LED_BUILTIN
#define SERVO_PIN 26 // ESP32 pin GPIO26 connected to servo motor to open and close the letterbox



void connectToWiFi();

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Wait a moment for serial to initialize
  delay(1000);
  
  // Print startup message
  Serial.println("ESP32 ");
  Serial.println("Built-in LED will start blinking...");

  connectToWiFi();

  // Configure the LED pin as an output
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  // Start with LED off
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_BLUE, LOW);
  
  servoMotor.attach(SERVO_PIN);  // attaches the servo on ESP32 pin

 
  Serial.println("Setup complete.");
}

void loop() {
  // Turn the LED on
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  Serial.println("LED ON");
  
  // Wait for 1 second
  delay(1000);
  
  // Turn the LED off
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_BLUE, LOW);
  Serial.println("LED OFF");
  
  // Wait for 1 second
  delay(1000);

   // Poll for Anchor events
   if (eventListener != nullptr && eventListener->isActive()) {
    eventListener->poll();
  }

} 

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("!connected");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}







// Setup event listener
void setupEventListener() {
  Serial.println("\n=== 🎧 Setting up Anchor Event Listener ===");
  
  // Configure the event listener
  EventListenerConfig config;
  config.programId = PROGRAM_ID;  // Your Anchor program ID
  config.pollIntervalMs = 30000;  // Poll every 30 seconds (reduced frequency)
  config.maxEvents = 5;           // Check last 5 transactions
  config.onlyConfirmed = true;    // Only confirmed transactions
  config.startSlot = 0;           // Start from latest
  
  // Create event listener instance
  eventListener = new AnchorEventListener(config);
  
  // Register event handlers for your program's events
  // These must match your Anchor event struct names exactly!
  eventListener->registerEventHandler("DroneArrivedEvent", onDroneArrivalEvent);
  eventListener->registerEventHandler("LetterboxSensorEvent", onLetterboxSensorEvent);
  
  // You can also register with custom discriminators if needed
  // Example: eventListener->registerEventHandlerWithDiscriminator("a1b2c3d4e5f60708", "CustomEvent", onCustomEvent);
  
  // Start listening
  eventListener->startListening();
  
  Serial.println("✅ Event listener configured and started");
}

// Event handler implementations


void onDroneArrivalEvent(const AnchorEvent& event) {
  Serial.println("\nDRONE ARRIVAL EVENT DETECTED!");
  Serial.print("  Transaction: ");
  Serial.println(event.signature);
  servoMotor.write(90);//open letterbox for drone to deliver
}

void onLetterboxSensorEvent(const AnchorEvent& event) {
  Serial.println("\nLETTERBOX SENSOR EVENT DETECTED!");
  Serial.print("  Transaction: ");
  Serial.println(event.signature);
  servoMotor.write(0);//close letterbox
}

/*  void letterboxSensor() {
  //if letterbox sensor is triggered
  //close letterbox, servo = 0 degrees
  //    servoMotor.write(0);
}
 */