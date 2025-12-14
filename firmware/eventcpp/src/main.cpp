#include "pins_arduino.h"
#include <Arduino.h>
#include <WiFi.h>
#include <IoTxChain.h-lib.h>
#include "SolanaUtils.h"
#include "ConfigLoader.h"

// Solana Configuration (Devnet)
const String solanaRpcUrl = "https://api.devnet.solana.com";
String PUBLIC_KEY_BASE58;
String PROGRAM_ID;
uint8_t PRIVATE_KEY[64];

// Initialize Solana Library
IoTxChain solana(solanaRpcUrl)

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

// UART to Flight Controller (TELEM port on FC, GPIO16/17 on ESP32)
HardwareSerial FCSerial(2);  // use UART2 on ESP32

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

// ---------- MAVLink Minimal-Parser nur für GLOBAL_POSITION_INT (ID 33) ----------

// Liest einen little-endian int32 aus 4 Bytes
int32_t readInt32(const uint8_t *buf) {
    return (int32_t)(
        ((uint32_t)buf[0]) |
        ((uint32_t)buf[1] << 8) |
        ((uint32_t)buf[2] << 16) |
        ((uint32_t)buf[3] << 24)
    );
}

/**
 * Mini-MAVLink-Parser.
 * Frisst ein Byte nach dem anderen.
 * Wenn eine komplette GLOBAL_POSITION_INT-Nachricht (ID 33) angekommen ist,
 * gibt die Funktion true zurück und schreibt lat/lon/alt in die Referenzen.
 *
 * - latDeg: Breitengrad in Grad
 * - lonDeg: Längengrad in Grad
 * - altMeters: Höhe in Metern
 */
bool parseMavlinkByte(uint8_t b, float &latDeg, float &lonDeg, float &altMeters) {
    // Zustände des kleinen Automaten
    enum State {
        WAIT_STX,
        READ_LEN,
        READ_SEQ,
        READ_SYSID,
        READ_COMPID,
        READ_MSGID,
        READ_PAYLOAD,
        READ_CRC1,
        READ_CRC2
    };
    
    static State state = WAIT_STX;
    static uint8_t len = 0;
    static uint8_t msgid = 0;
    static uint8_t payload[64];   // reicht für GLOBAL_POSITION_INT (28 Bytes)
    static uint8_t payloadIndex = 0;

    switch (state) {
        case WAIT_STX:
            if (b == 0xFE) { // MAVLink v1 Startbyte
                state = READ_LEN;
            }
            break;

        case READ_LEN:
            len = b;
            // Wir erwarten für GLOBAL_POSITION_INT genau 28 Bytes
            if (len > sizeof(payload) || len == 0) {
                // irgendwas komisch -> Reset
                state = WAIT_STX;
            } else {
                payloadIndex = 0;
                state = READ_SEQ;
            }
            break;

        case READ_SEQ:
            // Sequenznummer ignorieren
            state = READ_SYSID;
            break;

        case READ_SYSID:
            // System-ID ignorieren
            state = READ_COMPID;
            break;

        case READ_COMPID:
            // Component-ID ignorieren
            state = READ_MSGID;
            break;

        case READ_MSGID:
            msgid = b;
            state = READ_PAYLOAD;
            break;

        case READ_PAYLOAD:
            payload[payloadIndex++] = b;
            if (payloadIndex >= len) {
                state = READ_CRC1;
            }
            break;

        case READ_CRC1:
            // Checksumme-Byte 1 (wir werten die CRC hier NICHT aus, nur überspringen)
            state = READ_CRC2;
            break;

        case READ_CRC2:
            // Checksumme-Byte 2 -> Paket ist vollständig
            state = WAIT_STX;

            // Nur wenn es GLOBAL_POSITION_INT (ID 33) ist und genug Bytes da sind:
            if (msgid == 33 && len >= 28) {
                // Layout von GLOBAL_POSITION_INT (MAVLink v1):
                // 0  - 3: time_boot_ms (uint32)
                // 4  - 7: lat (int32, 1e-7 deg)
                // 8  -11: lon (int32, 1e-7 deg)
                // 12 -15: alt (int32, mm)
                // 16 -19: relative_alt (int32, mm)
                // 20 -21: vx (int16, cm/s)
                // 22 -23: vy (int16, cm/s)
                // 24 -25: vz (int16, cm/s)
                // 26 -27: hdg (uint16, cdeg)

                int32_t lat_i = readInt32(&payload[4]);
                int32_t lon_i = readInt32(&payload[8]);
                int32_t alt_i = readInt32(&payload[12]);

                latDeg = (float)lat_i / 1e7f;
                lonDeg = (float)lon_i / 1e7f;
                altMeters = (float)alt_i / 1000.0f; // mm -> m

                return true; // Wir haben eine komplette GPS-Position
            }
            break;
    }

    return false; // Noch keine komplette GPS-Nachricht
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    // Start UART connection to Flight Controller on GPIO16 (RX) and GPIO17 (TX)
    // Make sure TELEM port on FC is set to MAVLink at 57600 baud
    FCSerial.begin(57600, SERIAL_8N1, 16, 17);
    Serial.println("UART to Flight Controller started on pins RX=16, TX=17");

    Serial.println("========================================");
    Serial.println("ESP32 Drone Delivery - Solana Events");
    Serial.println("========================================");
    
    initializeHardware();
    loadAllConfigurations();
    connectToWiFi();
    
    Serial.println("\nDrone ready. Press button to emit ready-to-fly event.");
}

void loop() {
    // 1) Neue MAVLink-GPS-Auswertung
    while (FCSerial.available() > 0) {
        uint8_t b = FCSerial.read();

        float latDeg, lonDeg, altMeters;
        // Byte an Parser füttern
        if (parseMavlinkByte(b, latDeg, lonDeg, altMeters)) {
            // Wenn wir hier sind, haben wir eine komplette GPS-Position bekommen
            Serial.println("\n--- GPS POSITION ---");
            Serial.print("Latitude : ");
            Serial.println(latDeg, 7);
            Serial.print("Longitude: ");
            Serial.println(lonDeg, 7);
            Serial.print("Altitude : ");
            Serial.print(altMeters, 2);
            Serial.println(" m");
            Serial.println("--------------------");
        }
    }

    // 2) Button -> Solana-Transaktion (wie vorher)
    if (checkButton() && !transactionInProgress) { 
        transactionInProgress = true;
        
        Serial.println("\nButton pressed. Attempting transaction...");
        readyToFly();
    }
    
    delay(10);  // small delay to avoid busy loop
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

