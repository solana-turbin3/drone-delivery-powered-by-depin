#ifndef GET_SOLANA_LOGS_UTILS_H
#define GET_SOLANA_LOGS_UTILS_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Custom sendRawTransaction with enhanced JSON-RPC parameters
bool sendRawTxAndGetLogs(const String &txBase64, String &outSignature);

#endif // CUSTOM_SOLANA_UTILS_H
