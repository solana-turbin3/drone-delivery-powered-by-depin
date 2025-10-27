#include "pins_arduino.h"
#include <Arduino.h>
#include <WiFi.h>
#include "IoTxChain-lib.h"
#include "SolanaUtils.h"
#include "ConfigLoader.h"

// Solana Configuration (Devnet)
const String solanaRpcUrl = "https://api.devnet.solana.com";
String PUBLIC_KEY_BASE58;
String PROGRAM_ID;
uint8_t PRIVATE_KEY[64];

// Initialize Solana Library
IoTxChain solana(solanaRpcUrl);

// Hardware Pin Configuration
#define BUTTON_PIN 4
#define LED_GREEN 2
#define LED_RED 15
#define BUZZER_PIN 5

// Global configurations
DroneConfig droneConfig;
WiFiConfig wifiConfig;
WalletConfig walletConfig;

// System state
bool transactionInProgress = false;

// Function declarations
void initializeHardware();
void connectToWiFi();
void readyToFly();
void handleSuccess();
void handleError();
void playSuccessTone();
void playErrorTone();
bool checkButton();
void loadAllConfigurations();

// External functions (defined in SolanaUtils.cpp)
extern bool sendAnchorInstruction(
    const String& functionName,
    const String& pdaAddress,
    const std::vector<uint8_t>& payload,
    bool isInit
);

void loadAllConfigurations() {
    Serial.println("\nLoading configurations from filesystem...");
    
    if (!ConfigLoader::begin()) {
        Serial.println("FATAL ERROR: Could not mount filesystem");
        while(1) {
            handleError();
            delay(5000);
        }
    }
    
    // Load drone config
    droneConfig = ConfigLoader::loadDroneConfig();
    if (!droneConfig.valid) {
        Serial.println("FATAL ERROR: Could not load drone configuration");
        ConfigLoader::end();
        while(1) {
            handleError();
            delay(5000);
        }
    }
    
    // Load WiFi config
    wifiConfig = ConfigLoader::loadWiFiConfig();
    if (!wifiConfig.valid) {
        Serial.println("FATAL ERROR: Could not load WiFi configuration");
        ConfigLoader::end();
        while(1) {
            handleError();
            delay(5000);
        }
    }
    
    // Load wallet config
    walletConfig = ConfigLoader::loadWalletConfig();
    if (!walletConfig.valid) {
        Serial.println("FATAL ERROR: Could not load wallet configuration");
        ConfigLoader::end();
        while(1) {
            handleError();
            delay(5000);
        }
    }
    
    ConfigLoader::end();
    
    // Set global variables
    PUBLIC_KEY_BASE58 = droneConfig.operatorPubkey;
    PROGRAM_ID = droneConfig.programId;
    memcpy(PRIVATE_KEY, walletConfig.privateKey, 64);
    
    Serial.println("\nAll configurations loaded successfully!");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("========================================");
    Serial.println("ESP32 Drone Delivery - Solana Events");
    Serial.println("========================================");
    
    initializeHardware();
    loadAllConfigurations();
    connectToWiFi();
    
    Serial.println("\nDrone ready. Press button to emit ready-to-fly event.");
}

void loop() {
    if (checkButton() && !transactionInProgress) { 
        transactionInProgress = true;
        
        Serial.println("\nButton pressed. Attempting transaction...");
        readyToFly();
    }
    
    delay(100); 
}

void readyToFly() {
    Serial.println("\n=== Ready to Fly ===");
    
    Serial.print("Using operator: ");
    Serial.println(droneConfig.operatorPubkey);
    Serial.print("Using PDA: ");
    Serial.println(droneConfig.droneStatePda);
    
    std::vector<uint8_t> payload;
    
    if (sendAnchorInstruction("ready_to_fly", droneConfig.droneStatePda, payload, false)) {
        Serial.println("Ready-to-fly event emitted successfully!");
        handleSuccess();
    } else {
        Serial.println("Ready-to-fly transaction failed!");
        handleError();
    }
}

void initializeHardware() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    noTone(BUZZER_PIN);
    
    Serial.println("Hardware initialized");
}

void connectToWiFi() {
    Serial.print("Attempting to connect to WiFi...");
    
    WiFi.begin(wifiConfig.ssid.c_str(), wifiConfig.password.c_str());
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
    }
    
    digitalWrite(LED_GREEN, LOW);
    Serial.println("\nWiFi connection successful! Blockchain link established.");
    
    playSuccessTone();
}

bool checkButton() {
    int reading = digitalRead(BUTTON_PIN);
    
    if (reading == LOW) {
        delay(5); 
        if (digitalRead(BUTTON_PIN) == LOW) {
            return true;
        }
    }
    return false;
}

void handleSuccess() {
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_GREEN, HIGH);
        delay(200);
        digitalWrite(LED_GREEN, LOW);
        delay(200);
    }
    
    playSuccessTone();
    transactionInProgress = false; 
}

void handleError() {
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);
        delay(100);
    }
    
    playErrorTone();
    transactionInProgress = false; 
}

void playSuccessTone() {
    tone(BUZZER_PIN, 523, 150);
    delay(150);
    tone(BUZZER_PIN, 659, 150);
    delay(150);
    tone(BUZZER_PIN, 784, 300);
    delay(300);
    noTone(BUZZER_PIN);
}

void playErrorTone() {
    for (int i = 0; i < 3; i++) {
        tone(BUZZER_PIN, 200, 200);
        delay(250);
    }
    noTone(BUZZER_PIN);
}