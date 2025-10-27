#include "pins_arduino.h"
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include "IoTxChain-lib.h"
#include "SolanaUtils.h"
#include "ssid.h"
#include "keypair.h"
#include "AnchorEvents.h"


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
void onTransferEvent(const AnchorEvent& event);
void onTempSetEvent(const AnchorEvent& event);
void onDroneArrivalEvent(const AnchorEvent& event);
void onLetterboxSensorEvent(const AnchorEvent& event);

// Define the built-in LED pin for Arduino Nano ESP32
// The Arduino Nano ESP32 has the built-in LED on pin 13
#define LED_PIN LED_BUILTIN


void connectToWiFi();
void setTemp(float temperature);

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Wait a moment for serial to initialize
  delay(1000);
  
  // Print startup message
  Serial.println("Arduino Nano ESP32 Example");
  Serial.println("Built-in LED will start blinking...");

  connectToWiFi();

  // Configure the LED pin as an output
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  // Start with LED off
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_BLUE, LOW);
  
  getSolBalance();  
 
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
  
  val = Serial.read();
  if (val == 'T')
  {
    delay(500);
    Serial.println("Temperature is 101");
    temperature = 101;
    
    if (temperature > 0) {
      Serial.println("Setting temperature");
    setTemp(temperature);
    }
  }

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


void setTemp(float temperature) {
  Serial.println("\n=== 🔹 setTemp() ===");
  std::vector<std::vector<uint8_t>> seeds = {
    {'t','e','m','p'},
    base58ToPubkey(PUBLIC_KEY_BASE58)
  };

  // Prepare payload (temperature as u32 little-endian)
  uint32_t u32temperature = (uint32_t)temperature;

  std::vector<uint8_t> payload(4);
  memcpy(payload.data(), &u32temperature, sizeof(float));

  sendAnchorInstructionWithPDA(String("set_temp"), seeds, payload);
}

void listenForDroneArrival() {
  //if dronestatus= arrived
  //open letterbox, servo = 90 degrees
}

void letterboxSensor() {
  //if letterbox sensor is triggered
  //close letterbox, servo = 0 degrees
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
  eventListener->registerEventHandler("TempSetEvent", onTempSetEvent);
  eventListener->registerEventHandler("TransferEvent", onTransferEvent);
  
  // You can also register with custom discriminators if needed
  // Example: eventListener->registerEventHandlerWithDiscriminator("a1b2c3d4e5f60708", "CustomEvent", onCustomEvent);
  
  // Start listening
  eventListener->startListening();
  
  Serial.println("✅ Event listener configured and started");
}

// Event handler implementations
void onTransferEvent(const AnchorEvent& event) {
  Serial.println("\nTRANSFER EVENT DETECTED!");
  Serial.print("  Transaction: ");
  Serial.println(event.signature);
  Serial.print("  Slot: ");
  Serial.println(event.slot);
  
  // Parse TransferEvent data structure:
  // - amount: u64 (8 bytes)
  // - vault_balance: u64 (8 bytes)
  
  // Decode base64 to get actual bytes
  uint8_t decodedData[64];
  extern int base64_decode(const String& encoded, uint8_t* output, int maxLen);
  int decodedLen = base64_decode(event.rawData, decodedData, 64);
  
  if (decodedLen >= 24) {  // 8 bytes discriminator + 8 bytes amount + 8 bytes balance
    // Skip discriminator (8 bytes) and parse values
    uint64_t amount = 0;
    uint64_t vaultBalance = 0;
    
    // Parse amount (little-endian u64 at offset 8)
    for (int i = 0; i < 8; i++) {
      amount |= ((uint64_t)decodedData[8 + i]) << (i * 8);
    }
    
    // Parse vault_balance (little-endian u64 at offset 16)
    for (int i = 0; i < 8; i++) {
      vaultBalance |= ((uint64_t)decodedData[16 + i]) << (i * 8);
    }
    
    Serial.print("  Amount: ");
    Serial.print(amount);
    Serial.println(" lamports");
    Serial.print("  Vault Balance: ");
    Serial.print(vaultBalance);
    Serial.println(" lamports");
    
    // Flash Built-In LED pattern for transfer
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, LOW);
      delay(50);
      digitalWrite(LED_PIN, HIGH);
      delay(50);
    }
  }
}

void onTempSetEvent(const AnchorEvent& event) {
  Serial.println("\nTEMPERATURE SET EVENT!");
  Serial.print("  Transaction: ");
  Serial.println(event.signature);
  
  // Parse TempSetEvent data structure:
  // - old_value: u32 (4 bytes)
  // - new_value: u32 (4 bytes)
  
  // Decode base64 to get actual bytes
  uint8_t decodedData[64];
  extern int base64_decode(const String& encoded, uint8_t* output, int maxLen);
  int decodedLen = base64_decode(event.rawData, decodedData, 64);
  
  if (decodedLen >= 16) {  // 8 bytes discriminator + 4 bytes old + 4 bytes new
    // Skip discriminator (8 bytes) and parse values
    uint32_t oldValue = 0;
    uint32_t newValue = 0;
    
    // Parse old_value (little-endian u32 at offset 8)
    for (int i = 0; i < 4; i++) {
      oldValue |= ((uint32_t)decodedData[8 + i]) << (i * 8);
    }
    
    // Parse new_value (little-endian u32 at offset 12)
    for (int i = 0; i < 4; i++) {
      newValue |= ((uint32_t)decodedData[12 + i]) << (i * 8);
    }
    
    Serial.print("  Old Temperature: ");
    Serial.println(oldValue);
    Serial.print("  New Temperature: ");
    Serial.println(newValue);
  }
  
  // Flash Blue LED to indicate event
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BLUE, LOW);
    delay(100);
    digitalWrite(LED_BLUE, HIGH);
    delay(100);
  }
}

void onDroneArrivalEvent(const AnchorEvent& event) {
  Serial.println("\nDRONE ARRIVAL EVENT DETECTED!");
  Serial.print("  Transaction: ");
  Serial.println(event.signature);
}

void onLetterboxSensorEvent(const AnchorEvent& event) {
  Serial.println("\nLETTERBOX SENSOR EVENT DETECTED!");
  Serial.print("  Transaction: ");
  Serial.println(event.signature);
}