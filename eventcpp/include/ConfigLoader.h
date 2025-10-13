#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

struct DroneConfig {
    String operatorPubkey;
    String droneStatePda;
    uint8_t canonicalBump;
    String programId;
    String network;
    bool valid;
};

struct WiFiConfig {
    String ssid;
    String password;
    bool valid;
};

struct WalletConfig {
    uint8_t privateKey[64];
    bool valid;
};

class ConfigLoader {
public:
    static bool begin() {
        if (!SPIFFS.begin(true)) {
            Serial.println("ERROR: Failed to mount SPIFFS");
            return false;
        }
        return true;
    }

    static void end() {
        SPIFFS.end();
    }

    static DroneConfig loadDroneConfig() {
        DroneConfig config;
        config.valid = false;

        File file = SPIFFS.open("/drone-config.json", "r");
        if (!file) {
            Serial.println("ERROR: Failed to open /drone-config.json");
            Serial.println("   1. Make sure data/drone-config.json exists");
            Serial.println("   2. Run: pio run --target uploadfs");
            return config;
        }

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) {
            Serial.println("ERROR: Failed to parse drone-config.json");
            Serial.print("   Parse error: ");
            Serial.println(error.c_str());
            return config;
        }

        config.operatorPubkey = doc["operatorPubkey"].as<String>();
        config.droneStatePda = doc["droneStatePda"].as<String>();
        config.canonicalBump = doc["canonicalBump"].as<uint8_t>();
        config.programId = doc["programId"].as<String>();
        config.network = doc["network"].as<String>();
        config.valid = true;

        Serial.println("Drone configuration loaded successfully");
        Serial.print("   Operator: ");
        Serial.println(config.operatorPubkey);
        Serial.print("   PDA:      ");
        Serial.println(config.droneStatePda);
        Serial.print("   Bump:     ");
        Serial.println(config.canonicalBump);
        Serial.print("   Network:  ");
        Serial.println(config.network);

        return config;
    }

    static WiFiConfig loadWiFiConfig() {
        WiFiConfig config;
        config.valid = false;

        File file = SPIFFS.open("/wifi-config.json", "r");
        if (!file) {
            Serial.println("ERROR: Failed to open /wifi-config.json");
            Serial.println("   Create data/wifi-config.json with:");
            Serial.println("   {\"ssid\": \"YOUR_SSID\", \"password\": \"YOUR_PASSWORD\"}");
            return config;
        }

        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) {
            Serial.println("ERROR: Failed to parse wifi-config.json");
            return config;
        }

        config.ssid = doc["ssid"].as<String>();
        config.password = doc["password"].as<String>();
        config.valid = true;

        Serial.println("WiFi configuration loaded successfully");

        return config;
    }

    static WalletConfig loadWalletConfig() {
        WalletConfig config;
        config.valid = false;

        File file = SPIFFS.open("/wallet.json", "r");
        if (!file) {
            Serial.println("ERROR: Failed to open /wallet.json");
            Serial.println("   Copy your Solana keypair to data/wallet.json");
            return config;
        }

        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) {
            Serial.println("ERROR: Failed to parse wallet.json");
            return config;
        }

        JsonArray array = doc.as<JsonArray>();
        if (array.size() != 64) {
            Serial.println("ERROR: wallet.json must contain exactly 64 bytes");
            return config;
        }

        for (size_t i = 0; i < 64; i++) {
            config.privateKey[i] = array[i].as<uint8_t>();
        }

        config.valid = true;
        Serial.println("Wallet configuration loaded successfully");

        return config;
    }
};

#endif