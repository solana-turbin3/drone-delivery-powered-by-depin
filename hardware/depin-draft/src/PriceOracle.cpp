#include "PriceOracle.h"
#include <base64.h>

PriceOracle::PriceOracle(bool useMainnet) {
    hermesUrl = useMainnet ? HERMES_MAINNET_URL : HERMES_TESTNET_URL;
}

PriceOracle::~PriceOracle() {
    if (http.connected()) {
        http.end();
    }
}

PriceData PriceOracle::parsePythResponse(String jsonResponse, String feedId) {
    PriceData data;
    data.valid = false;
    
    // Parse JSON response
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return data;
    }
    
    // Check if we have parsed array
    if (doc.is<JsonArray>()) {
        JsonArray array = doc.as<JsonArray>();
        if (array.size() > 0) {
            JsonVariant priceInfoVar = array[0];
            if (priceInfoVar.is<JsonObject>()) {
                JsonObject priceInfo = priceInfoVar.as<JsonObject>();
                
                // Extract price data
                JsonVariant priceVar = priceInfo["price"];
                if (!priceVar.isNull() && priceVar.is<JsonObject>()) {
                    JsonObject price = priceVar.as<JsonObject>();
                    
                    // Get the raw price value
                    JsonVariant priceStrVar = price["price"];
                    if (!priceStrVar.isNull()) {
                        data.price = strtod(priceStrVar.as<const char*>(), NULL);
                    }
                    
                    // Get confidence interval
                    JsonVariant confVar = price["conf"];
                    if (!confVar.isNull()) {
                        data.confidence = strtod(confVar.as<const char*>(), NULL);
                    }
                    
                    // Get exponent
                    JsonVariant expoVar = price["expo"];
                    if (!expoVar.isNull()) {
                        data.expo = expoVar.as<int32_t>();
                    }
                    
                    // Get publish time
                    JsonVariant timeVar = price["publish_time"];
                    if (!timeVar.isNull()) {
                        data.publishTime = timeVar.as<uint64_t>();
                    }
                    
                    // Calculate actual price
                    data.price = calculateActualPrice(data.price, data.expo);
                    data.confidence = calculateActualPrice(data.confidence, data.expo);
                    
                    // Get EMA price if available
                    JsonVariant emaVar = priceInfo["ema_price"];
                    if (!emaVar.isNull() && emaVar.is<JsonObject>()) {
                        JsonObject emaPrice = emaVar.as<JsonObject>();
                        JsonVariant emaPriceVar = emaPrice["price"];
                        if (!emaPriceVar.isNull()) {
                            data.priceEMA = strtod(emaPriceVar.as<const char*>(), NULL);
                            data.priceEMA = calculateActualPrice(data.priceEMA, data.expo);
                        }
                    } else {
                        data.priceEMA = data.price;
                    }
                    
                    // Extract symbol/pair if available
                    JsonVariant metaVar = priceInfo["metadata"];
                    if (!metaVar.isNull() && metaVar.is<JsonObject>()) {
                        JsonObject metadata = metaVar.as<JsonObject>();
                        JsonVariant symbolVar = metadata["symbol"];
                        if (!symbolVar.isNull()) {
                            data.symbol = symbolVar.as<String>();
                        }
                    }
                    
                    data.valid = true;
                }
            }
        }
    }
    
    return data;
}

PriceData PriceOracle::getLatestPrice(String feedId) {
    PriceData data;
    data.valid = false;
    
    // Construct the API endpoint
    String endpoint = hermesUrl + "/api/latest_price_feeds?ids[]=" + feedId;
    
    Serial.println("Fetching price from: " + endpoint);
    
    // Make HTTP GET request
    http.begin(endpoint);
    http.addHeader("Accept", "application/json");
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        data = parsePythResponse(response, feedId);
        
        if (data.valid) {
            Serial.println("Price fetched successfully!");
            Serial.print("Price: $");
            Serial.println(data.price);
        }
    } else {
        Serial.print("HTTP request failed with code: ");
        Serial.println(httpCode);
        if (httpCode > 0) {
            String error = http.getString();
            Serial.println("Error response: " + error);
        }
    }
    
    http.end();
    return data;
}

PriceData PriceOracle::getSolanaPrice() {
    PriceData data = getLatestPrice(PYTH_SOL_USD_FEED_ID);
    if (data.symbol.isEmpty()) {
        data.symbol = "SOL/USD";
    }
    return data;
}

PriceData PriceOracle::getEthereumPrice() {
    PriceData data = getLatestPrice(PYTH_ETH_USD_FEED_ID);
    if (data.symbol.isEmpty()) {
        data.symbol = "ETH/USD";
    }
    return data;
}

PriceData PriceOracle::getBitcoinPrice() {
    PriceData data = getLatestPrice(PYTH_BTC_USD_FEED_ID);
    if (data.symbol.isEmpty()) {
        data.symbol = "BTC/USD";
    }
    return data;
}

double PriceOracle::calculateActualPrice(double rawPrice, int32_t expo) {
    return rawPrice * pow(10, expo);
}

String PriceOracle::formatPrice(PriceData data, uint8_t decimals) {
    if (!data.valid) {
        return "N/A";
    }
    
    // Use dtostrf for proper double to string conversion with decimals
    char priceStr[20];
    char confStr[20];
    dtostrf(data.price, 0, decimals, priceStr);
    dtostrf(data.confidence, 0, decimals, confStr);
    
    String formatted = data.symbol.isEmpty() ? "" : data.symbol + ": ";
    formatted += "$";
    formatted += priceStr;
    formatted += " ±$";
    formatted += confStr;
    
    return formatted;
}