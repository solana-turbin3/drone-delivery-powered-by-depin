#include "pins_arduino.h"
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include "IoTxChain-lib.h"
#include "SolanaUtils.h"
#include "ssid.h"
#include "keypair.h"

int val;
float temperature;


// Solana RPC URL (Devnet)
const String solanaRpcUrl = "https://api.devnet.solana.com"; // or mainnet/testnet



const String PROGRAM_ID = "8NLjevMMfZDViPWAcLNJTQij4crwVddE1N8SRwwrUSsd";
const String MINT = "8MaXvmTFewPTD2oQoxjiCYPDU3BmvhZSHo5RBAi41Fek";
const String VAULT = "7yjCjijhaK7pqC21rwuzmwKaG9B6horHa4qzALHjjGZz";
const String TOKEN_PROGRAM_ID = "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA";

// Initialize Solana Library
IoTxChain solana(solanaRpcUrl);

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