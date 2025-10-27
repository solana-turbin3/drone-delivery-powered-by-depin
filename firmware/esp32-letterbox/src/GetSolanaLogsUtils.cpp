#include "GetSolanaLogsUtils.h"
#include "SolanaUtils.h"

// Custom sendRawTransaction 
/* bool sendRawTxAndGetLogs(const String &txBase64, String &outSignature) {

} */

//https://github.com/asimbugra/IoTxChain/blob/main/src/IoTxChain-lib.cpp
// --------------------------------------------------------------------------------
//"sendRawTransaction" -> Uses JSON-RPC method "sendTransaction" with base64 encoding
// --------------------------------------------------------------------------------
bool sendRawTxAndGetLogs(const String &txBase64, String &outSignature) {
  // --------------------------------------------------------------------------------
  // Sends a Base64-encoded raw transaction to the Solana blockchain via JSON-RPC.
  // This method is typically used after signing a transaction and encoding it.
  // Returns true if broadcast was successful and outputs the transaction signature.
  // --------------------------------------------------------------------------------
  WiFiClientSecure client;
  // Create a secure WiFi client with insecure certificate validation (for ESP32)
  client.setInsecure(); // Skip certificate validation (for simplicity)
  HTTPClient http;

  if (!http.begin(client, solanaRpcUrl)) {
      Serial.println("HTTP begin failed for sendRawTransaction");
      return false;
  }

  // Set request headers for JSON-RPC
  http.addHeader("Content-Type", "application/json");

  // Construct JSON-RPC body with base64-encoded transaction data
  // Prepare JSON body with "method": "sendTransaction" and "encoding": "base64"
  String body = String() +
      R"({"jsonrpc":"2.0","id":1,"method":"sendTransaction","params":[")" +
      txBase64 + 
      R"(",{"encoding":"base64","skipPreflight":false,"preflightCommitment":"confirmed"}]})";

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
  //hh added to output program logs
  Serial.println("raw http Response: " + response);
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