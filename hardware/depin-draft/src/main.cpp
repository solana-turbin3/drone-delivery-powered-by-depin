#include "pins_arduino.h"
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include "IoTxChain-lib.h"
#include "SolanaUtils.h"
#include "PriceOracle.h"

const char* ssid    = "SSID";
const char* password = "PASSWORD";

// Solana RPC URL (Devnet)
const String solanaRpcUrl = "https://api.devnet.solana.com"; // or mainnet/testnet

// Your Solana wallet (Base58 format)
const String PRIVATE_KEY_BASE58 = "PRIVATE_KEY_BASE58";  // 64-byte base58
const String PUBLIC_KEY_BASE58 = "AHYic562KhgtAEkb1rSesqS87dFYRcfXb4WwWus3Zc9C";

const String PROGRAM_ID = "3iVjkQPbzHRfNGqzkrNBfE1m2TJYWQbycCMPukrdk6pP";
const String MINT = "8MaXvmTFewPTD2oQoxjiCYPDU3BmvhZSHo5RBAi41Fek";
const String VAULT = "7yjCjijhaK7pqC21rwuzmwKaG9B6horHa4qzALHjjGZz";
const String TOKEN_PROGRAM_ID = "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA";

// Initialize Solana Library
IoTxChain solana(solanaRpcUrl);

// Initialize Price Oracle (using mainnet prices)
PriceOracle priceOracle(true);

// Define the built-in LED pin for Arduino Nano ESP32
// The Arduino Nano ESP32 has the built-in LED on pin 13
#define LED_PIN LED_BUILTIN

#define DHT11PIN 6
#define DHTTYPE DHT11

DHT dht(DHT11PIN, DHTTYPE);

void connectToWiFi();
void fetchAndDisplayPrices();
void transferToVault();
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
  
  // Initialize DHT sensor
  dht.begin();

  getSolBalance();

  calculateDiscriminator("transfer_to_vault");
  
  // Fetch initial price data
  Serial.println("\n=== Fetching initial Solana price ===");
  fetchAndDisplayPrices();
  
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
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    fetchAndDisplayPrices();
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: ")); // Heat index is the "feels like" temperature. It is calculated based on the temperature and humidity.
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  if (hic > 30) {
    Serial.println(F("Heat index is above 30°C! :("));
    digitalWrite(LED_RED, LOW); // Turn on red LED
    transferToVault();
    setTemp(t);
  } else {
    Serial.println(F("Heat index is below 30°C :)"));
    digitalWrite(LED_RED, HIGH); // Turn off red LED
  }

  // Fetch and display prices every 5 loops (10 seconds)
  static int loopCounter = 0;
  loopCounter++;
  
  if (loopCounter % 5 == 0) {
    fetchAndDisplayPrices();
  }
} 

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void fetchAndDisplayPrices() {
  Serial.println("\n=== Fetching Solana Price ===");
  
  // Fetch Solana price
  PriceData solPrice = priceOracle.getSolanaPrice();
  
  if (solPrice.valid) {
    Serial.println("Solana Price Data:");
    Serial.print("Current Price: $");
    Serial.println(solPrice.price, 2);
    Serial.print("Confidence: ±$");
    Serial.println(solPrice.confidence, 4);
    
    // Formatted output
    Serial.print("  Formatted: ");
    Serial.println(priceOracle.formatPrice(solPrice));
    
    // You can use the price for logic, e.g.:
    if (solPrice.price > 100.0) {
      Serial.println("SOL is above $100!");
      digitalWrite(LED_BLUE, HIGH); // Blue LED indicator
    } else {
      digitalWrite(LED_BLUE, LOW);
    }
  } else {
    Serial.println("Failed to fetch Solana price");
  }
  
  Serial.println("=================================\n");
}

void transferToVault() {
  Serial.println("\n=== 🔹 transferToVault() ===");
  std::vector<std::vector<uint8_t>> seeds = {};

  std::vector<uint8_t> payload;  // no payload for initialize
  sendAnchorInstructionWithPDA(String("transfer_to_vault"), seeds, payload);
}

void setTemp(float temperature) {
  Serial.println("\n=== 🔹 setTemp() ===");
  std::vector<std::vector<uint8_t>> seeds = {
    {'t','e','m','p'},
    base58ToPubkey(PUBLIC_KEY_BASE58)
  };

  // Prepare payload (temperature as float32 little-endian)
  std::vector<uint8_t> payload(4);
  memcpy(payload.data(), &temperature, sizeof(float));

  sendAnchorInstructionWithPDA(String("set_temp"), seeds, payload);
}