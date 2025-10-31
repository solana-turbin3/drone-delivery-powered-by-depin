#ifndef ANCHOR_EVENTS_H
#define ANCHOR_EVENTS_H

#include <Arduino.h>
#include <WiFi.h>

// Forward declarations to avoid include issues
class IoTxChain;

// Event data structure
struct AnchorEvent {
    String signature;           // Transaction signature
    uint64_t slot;              // Slot number
    uint64_t blockTime;         // Unix timestamp
    String programId;           // Program that emitted the event
    String eventName;           // Event name (decoded from discriminator)
    String discriminatorHex;    // 8-byte event discriminator as hex string
    String rawData;             // Raw event data as base64 or hex
    bool hasError;              // Transaction error status
};

// Event listener configuration
struct EventListenerConfig {
    String programId;           // Program ID to monitor
    uint32_t pollIntervalMs;    // Polling interval in milliseconds (default: 5000)
    uint32_t maxEvents;         // Maximum events to fetch per poll (default: 10)
    bool onlyConfirmed;         // Only process confirmed transactions (default: true)
    uint64_t startSlot;         // Starting slot (0 for latest)
};

// Simple event callback type
typedef void (*EventCallback)(const AnchorEvent& event);

class AnchorEventListener {
private:
    EventListenerConfig config;
    uint64_t lastProcessedSlot;
    bool isListening;
    unsigned long lastPollTime;
    
    // Simple event registry
    static const int MAX_HANDLERS = 10;
    struct EventHandler {
        String eventName;
        String discriminatorHex;
        EventCallback callback;
        bool active;
    };
    EventHandler handlers[MAX_HANDLERS];
    int handlerCount;
    
    // Helper functions
    String calculateDiscriminatorHex(const String& eventName);
    bool fetchRecentSignatures(String* signatures, int& count);
    bool fetchTransactionDetails(const String& signature, AnchorEvent& event);
    
public:
    // Constructor
    AnchorEventListener(const EventListenerConfig& config);
    
    // Register an event handler for a specific event
    bool registerEventHandler(const String& eventName, EventCallback callback);
    
    // Register handler with custom discriminator
    bool registerEventHandlerWithDiscriminator(const String& discriminatorHex, 
                                              const String& eventName, 
                                              EventCallback callback);
    
    // Start listening for events (non-blocking)
    void startListening();
    
    // Stop listening for events
    void stopListening();
    
    // Poll once for new events (should be called in loop())
    void poll();
    
    // Check if listener is active
    bool isActive() const { return isListening; }
    
    // Get last processed slot
    uint64_t getLastProcessedSlot() const { return lastProcessedSlot; }
    
    // Set RPC URL
    void setRpcUrl(const String& url);
};

// Helper class for parsing common event types
class EventDataParser {
public:
    // Parse amount from event data
    static uint64_t parseAmount(const String& data, int offset = 0);
    
    // Parse public key from event data
    static String parsePublicKey(const String& data, int offset = 0);
    
    // Parse float value from event data
    static float parseFloat(const String& data, int offset = 0);
    
    // Parse string from event data
    static String parseString(const String& data, int offset = 0);
    
    // Convert hex string to bytes
    static void hexToBytes(const String& hex, uint8_t* bytes, size_t length);
    
    // Convert bytes to hex string
    static String bytesToHex(const uint8_t* bytes, size_t length);
};

#endif // ANCHOR_EVENTS_H 