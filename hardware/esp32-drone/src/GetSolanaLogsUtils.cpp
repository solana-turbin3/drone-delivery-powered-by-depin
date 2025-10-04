#include "GetSolanaLogsUtils.h"
#include "SolanaUtils.h"

// Custom sendRawTransaction with enhanced JSON-RPC parameters
bool sendRawTxAndGetLogs(const String &txBase64, String &outSignature) {
  // Custom implementation with your desired JSON-RPC parameters
  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate validation (for ESP32)
  HTTPClient http;

  // Use the RPC_URL from your constants
  if (!http.begin(client, solanaRpcUrl)) {
    Serial.println("HTTP begin failed for sendRawTransaction");
    return false;
  }

  // Set request headers for JSON-RPC
  http.addHeader("Content-Type", "application/json");

  // Your custom JSON-RPC body with the parameters you want
  String body =
      String() +
      R"(
      {
        "jsonrpc":"2.0",
        "id":1,
        "method":"sendTransaction",
        "params":[")" +
            txBase64 +
            R"(",{
            "encoding":"base64",
            "commitment":"confirmed",
            "sigVerify":false,
            "replaceRecentBlockhash":true,
            "accounts":{
                "encoding":"base64",
                "addresses":[]
                },
            "logs":true
            }]
        })";

  // Execute HTTP POST request
  int code = http.POST(body);
  if (code != 200) {
    Serial.printf("sendTransaction HTTP code: %d\n", code);
    String resp = http.getString();
    Serial.println("Response: " + resp);
    http.end();
    return false;
  }

  // Parse response and deserialize JSON
  String response = http.getString();
  http.end();

  DynamicJsonDocument doc(2048);
  auto err = deserializeJson(doc, response);
  if (err) {
    Serial.println("JSON parse error in sendTransaction");
    return false;
  }

  // Check for error field in the RPC response
  if (doc["error"]) {
    Serial.println("RPC Error: " + doc["error"]["message"].as<String>());
    return false;
  }

  // Extract transaction signature from result
  outSignature = doc["result"].as<String>();
  return true;
}
