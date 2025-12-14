#include "AnchorEvents.h"
#include "IoTxChain-lib.h"
#include "SolanaUtils.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SHA256.h>

// External reference to solana instance
extern IoTxChain solana;
extern const String solanaRpcUrl;

// Base64 decoding helper
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_decode(const String& encoded, uint8_t* output, int maxLen) {
    int i = 0, j = 0, k = 0;
    uint8_t temp[4];
    
    while (i < encoded.length() && j < maxLen) {
        // Skip non-base64 characters
        if (encoded[i] == '=' || encoded[i] == '\n' || encoded[i] == '\r') {
            i++;
            continue;
        }
        
        // Find character in base64 table
        const char* p = strchr(base64_chars, encoded[i]);
        if (p == nullptr) {
            i++;
            continue;
        }
        
        temp[k++] = p - base64_chars;
        i++;
        
        if (k == 4) {
            output[j++] = (temp[0] << 2) | (temp[1] >> 4);
            if (j < maxLen) output[j++] = (temp[1] << 4) | (temp[2] >> 2);
            if (j < maxLen) output[j++] = (temp[2] << 6) | temp[3];
            k = 0;
        }
    }
    
    return j;
}

// Constructor
AnchorEventListener::AnchorEventListener(const EventListenerConfig& config) 
    : config(config), lastProcessedSlot(0), isListening(false), lastPollTime(0), handlerCount(0) {
    
    // Set defaults if not provided
    if (config.pollIntervalMs == 0) {
        this->config.pollIntervalMs = 5000;  // 5 seconds default
    }
    if (config.maxEvents == 0) {
        this->config.maxEvents = 10;
    }
    
    // Initialize handlers array
    for (int i = 0; i < MAX_HANDLERS; i++) {
        handlers[i].active = false;
    }
}

// Register an event handler
bool AnchorEventListener::registerEventHandler(const String& eventName, EventCallback callback) {
    if (handlerCount >= MAX_HANDLERS) {
        Serial.println("Max event handlers reached");
        return false;
    }
    
    String discriminator = calculateDiscriminatorHex(eventName);
    return registerEventHandlerWithDiscriminator(discriminator, eventName, callback);
}

// Register handler with custom discriminator
bool AnchorEventListener::registerEventHandlerWithDiscriminator(
    const String& discriminatorHex, 
    const String& eventName, 
    EventCallback callback) {
    
    if (handlerCount >= MAX_HANDLERS) {
        Serial.println("Max event handlers reached");
        return false;
    }
    
    // Find empty slot
    for (int i = 0; i < MAX_HANDLERS; i++) {
        if (!handlers[i].active) {
            handlers[i].eventName = eventName;
            handlers[i].discriminatorHex = discriminatorHex;
            handlers[i].callback = callback;
            handlers[i].active = true;
            handlerCount++;
            
            Serial.print("Registered handler for event: ");
            Serial.print(eventName);
            Serial.print(" with discriminator: ");
            Serial.println(discriminatorHex);
            return true;
        }
    }
    
    return false;
}

// Calculate discriminator from event name
String AnchorEventListener::calculateDiscriminatorHex(const String& eventName) {
    // Use SHA256 to calculate discriminator (first 8 bytes)
    // Anchor format: sha256("event:<EventStructName>")
    String input = "event:" + eventName;
    
    Serial.print("📝 Calculating discriminator for: ");
    Serial.println(input);
    
    SHA256 sha256;
    sha256.reset();
    sha256.update((const uint8_t*)input.c_str(), input.length());
    uint8_t hash[32];
    sha256.finalize(hash, 32);
    
    // Convert first 8 bytes to hex
    String discriminator = "";
    for (int i = 0; i < 8; i++) {
        if (hash[i] < 16) discriminator += "0";
        discriminator += String(hash[i], HEX);
    }
    
    Serial.print("Discriminator: ");
    Serial.println(discriminator);
    
    return discriminator;
}

// Start listening
void AnchorEventListener::startListening() {
    isListening = true;
    lastPollTime = 0;  // Force immediate poll
    Serial.println("Event listener started for program: " + config.programId);
}

// Stop listening
void AnchorEventListener::stopListening() {
    isListening = false;
    Serial.println("Event listener stopped");
}

// Poll for new events
void AnchorEventListener::poll() {
    if (!isListening) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastPollTime < config.pollIntervalMs) {
        return;  // Not time to poll yet
    }
    lastPollTime = currentTime;
    
    Serial.println("\nPolling for events...\n");
    
    // Fetch recent signatures
    String signatures[10];
    int count = 0;
    
    if (!fetchRecentSignatures(signatures, count)) {
        Serial.println("Failed to fetch signatures");
        return;
    }
    
    if (count == 0) {
        Serial.println("No new transactions");
        return;
    }
    
    Serial.print("Found ");
    Serial.print(count);
    Serial.println(" recent transactions");
    
    // Process each signature
    for (int i = 0; i < count; i++) {
        AnchorEvent event;
        if (fetchTransactionDetails(signatures[i], event)) {
            Serial.print("Looking for handler for discriminator: ");
            Serial.println(event.discriminatorHex);
            
            // Check if we have a handler for this event
            bool found = false;
            for (int j = 0; j < MAX_HANDLERS; j++) {
                if (handlers[j].active) {
                    Serial.print("   Comparing with ");
                    Serial.print(handlers[j].eventName);
                    Serial.print(": ");
                    Serial.println(handlers[j].discriminatorHex);
                    
                    if (handlers[j].discriminatorHex.equalsIgnoreCase(event.discriminatorHex)) {
                        Serial.print("Match! Triggering handler for event: ");
                        Serial.println(handlers[j].eventName);
                        
                        event.eventName = handlers[j].eventName;
                        
                        // Call the handler
                        handlers[j].callback(event);
                        found = true;
                        break;
                    }
                }
            }
            
            if (!found) {
                Serial.println("No matching handler found for this discriminator");
            }
            
            // Update last processed slot
            if (event.slot > lastProcessedSlot) {
                lastProcessedSlot = event.slot;
            }
        }
    }
}

// Fetch recent signatures for the program
bool AnchorEventListener::fetchRecentSignatures(String* signatures, int& count) {
    HTTPClient http;
    
    // Build JSON-RPC request
    DynamicJsonDocument request(1024);
    request["jsonrpc"] = "2.0";
    request["id"] = 1;
    request["method"] = "getSignaturesForAddress";
    
    JsonArray params = request.createNestedArray("params");
    params.add(config.programId);
    
    JsonObject options = params.createNestedObject();
    options["limit"] = config.maxEvents;
    
    // Don't use "before" parameter on first poll or when slot is 0
    // as it might cause issues
    if (lastProcessedSlot > 0) {
        // Note: "before" parameter might need adjustment based on your needs
        // For now, let's not use it to ensure we get transactions
        // options["before"] = String(lastProcessedSlot);
    }
    
    String requestBody;
    serializeJson(request, requestBody);
    
    http.begin(solanaRpcUrl);
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.POST(requestBody);
    count = 0;
    
    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, response);
        
        if (!error && doc.containsKey("result")) {
            JsonArray result = doc["result"].as<JsonArray>();
            
            for (JsonVariant v : result) {
                if (count >= 10) break;
                
                // Filter for successful transactions if configured
                if (config.onlyConfirmed && v["err"] != nullptr) {
                    continue;
                }
                
                signatures[count] = v["signature"].as<String>();
                count++;
            }
            
            http.end();
            return true;
        }
    }
    
    http.end();
    return false;
}

// Fetch transaction details
bool AnchorEventListener::fetchTransactionDetails(const String& signature, AnchorEvent& event) {
    HTTPClient http;
    
    // Build JSON-RPC request for transaction details
    DynamicJsonDocument request(512);
    request["jsonrpc"] = "2.0";
    request["id"] = 1;
    request["method"] = "getTransaction";
    
    JsonArray params = request.createNestedArray("params");
    params.add(signature);
    
    JsonObject options = params.createNestedObject();
    options["encoding"] = "base64";
    options["commitment"] = "confirmed";
    
    String requestBody;
    serializeJson(request, requestBody);
    
    http.begin(solanaRpcUrl);
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.POST(requestBody);
    
    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        DynamicJsonDocument doc(8192);
        DeserializationError error = deserializeJson(doc, response);
        
        if (!error && doc.containsKey("result")) {
            JsonObject result = doc["result"];
            
            // Parse transaction data
            event.signature = signature;
            event.slot = result["slot"];
            event.blockTime = result["blockTime"];
            event.hasError = result["meta"]["err"] != nullptr;
            
            // Look for program logs to extract event data
            JsonArray logs = result["meta"]["logMessages"];
            
            Serial.println("\nChecking logs for events...");
            for (JsonVariant log : logs) {
                String logStr = log.as<String>();
                
                // Anchor events are typically emitted as "Program data: <base64>"
                if (logStr.startsWith("Program data: ")) {
                    String base64Data = logStr.substring(14);  // Remove "Program data: "
                    Serial.print("Found Program data: ");
                    Serial.println(base64Data.substring(0, 32) + "...");
                    
                    // Decode base64 to extract discriminator
                    if (base64Data.length() >= 12) {  // At least 8 bytes encoded
                        // Decode the base64 data
                        uint8_t decodedBytes[256];
                        int decodedLen = base64_decode(base64Data, decodedBytes, 256);
                        
                        if (decodedLen >= 8) {
                            // Extract discriminator (first 8 bytes)
                            event.discriminatorHex = "";
                            for (int i = 0; i < 8; i++) {
                                if (decodedBytes[i] < 16) event.discriminatorHex += "0";
                                event.discriminatorHex += String(decodedBytes[i], HEX);
                            }
                            
                            event.rawData = base64Data;
                            event.programId = config.programId;
                            
                            Serial.print("\nEvent discriminator: ");
                            Serial.println(event.discriminatorHex);
                            Serial.print("Decoded length: ");
                            Serial.print(decodedLen);
                            Serial.println("bytes\n");
                            
                            http.end();
                            return true;
                        }
                    }
                }
            }
            Serial.println("\n!!! No Program data found in logs !!!\n");
        }
    }
    
    http.end();
    return false;
}

// EventDataParser implementation
uint64_t EventDataParser::parseAmount(const String& data, int offset) {
    // Parse 8 bytes as uint64_t from hex or base64 data
    if (data.length() < offset + 16) return 0;
    
    String amountHex = data.substring(offset, offset + 16);
    uint64_t amount = 0;
    
    // Convert hex to uint64_t
    for (int i = 0; i < 16; i++) {
        char c = amountHex[i];
        uint8_t digit = 0;
        
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        }
        
        amount = (amount << 4) | digit;
    }
    
    return amount;
}

String EventDataParser::parsePublicKey(const String& data, int offset) {
    // Parse 32 bytes as public key from hex data
    if (data.length() < offset + 64) return "";
    
    return data.substring(offset, offset + 64);
}

float EventDataParser::parseFloat(const String& data, int offset) {
    // Parse 4 bytes as float from hex data
    if (data.length() < offset + 8) return 0.0;
    
    uint8_t bytes[4];
    hexToBytes(data.substring(offset, offset + 8), bytes, 4);
    
    float value;
    memcpy(&value, bytes, sizeof(float));
    return value;
}

String EventDataParser::parseString(const String& data, int offset) {
    // Parse string with length prefix from hex data
    if (data.length() < offset + 8) return "";
    
    // First 4 bytes are length
    uint32_t length = parseAmount(data, offset) & 0xFFFFFFFF;
    
    if (data.length() < offset + 8 + (length * 2)) return "";
    
    String result = "";
    for (uint32_t i = 0; i < length; i++) {
        String hexByte = data.substring(offset + 8 + (i * 2), offset + 8 + (i * 2) + 2);
        char c = (char)strtol(hexByte.c_str(), nullptr, 16);
        result += c;
    }
    
    return result;
}

void EventDataParser::hexToBytes(const String& hex, uint8_t* bytes, size_t length) {
    for (size_t i = 0; i < length && i * 2 < hex.length(); i++) {
        String byteStr = hex.substring(i * 2, i * 2 + 2);
        bytes[i] = (uint8_t)strtol(byteStr.c_str(), nullptr, 16);
    }
}

String EventDataParser::bytesToHex(const uint8_t* bytes, size_t length) {
    String hex = "";
    for (size_t i = 0; i < length; i++) {
        if (bytes[i] < 16) hex += "0";
        hex += String(bytes[i], HEX);
    }
    return hex;
} 