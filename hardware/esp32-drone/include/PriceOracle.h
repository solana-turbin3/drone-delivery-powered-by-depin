#ifndef PRICE_ORACLE_H
#define PRICE_ORACLE_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Pyth Network Price Feed IDs (Mainnet)
// You can find more at: https://pyth.network/developers/price-feed-ids
#define PYTH_SOL_USD_FEED_ID "0xef0d8b6fda2ceba41da15d4095d1da392a0d2f8ed0c6c7bc0f4cfac8c280b56d"  // SOL/USD
#define PYTH_ETH_USD_FEED_ID "0xff61491a931112ddf1bd8147cd1b641375f79f5825126d665480874634fd0ace"  // ETH/USD
#define PYTH_BTC_USD_FEED_ID "0xe62df6c8b4a85fe1a67db44dc12de5db330f7ac66b72dc658afedf0f4a415b43"  // BTC/USD

// Hermes API endpoints
#define HERMES_MAINNET_URL "https://hermes.pyth.network"
#define HERMES_TESTNET_URL "https://hermes-beta.pyth.network"

struct PriceData {
    double price;           // Current price
    double confidence;      // Confidence interval
    int32_t expo;          // Price exponent (actual_price = price * 10^expo)
    uint64_t publishTime;   // Timestamp in seconds
    double priceEMA;       // Exponential moving average price
    bool valid;            // Whether the price data is valid
    String symbol;         // Price feed symbol (e.g., "SOL/USD")
};

class PriceOracle {
private:
    String hermesUrl;
    HTTPClient http;
    
    // Parse the JSON response from Pyth
    PriceData parsePythResponse(String jsonResponse, String feedId);
    
public:
    // Constructor
    PriceOracle(bool useMainnet = true);
    
    // Destructor
    ~PriceOracle();
    
    // Fetch latest price for a given feed ID
    PriceData getLatestPrice(String feedId);
    
    // Methods for common price feeds
    PriceData getSolanaPrice();
    PriceData getEthereumPrice();
    PriceData getBitcoinPrice();
    
    // Calculate actual price from raw Pyth data
    double calculateActualPrice(double rawPrice, int32_t expo);
    
    // Format price for display
    String formatPrice(PriceData data, uint8_t decimals = 2);
};

#endif // PRICE_ORACLE_H 