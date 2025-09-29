#ifndef SOLANA_UTILS_H
#define SOLANA_UTILS_H

#include <Arduino.h>
#include <vector>
#include "IoTxChain-lib.h"

// External variables that need to be referenced
extern IoTxChain solana;
extern const String solanaRpcUrl;
extern const String PUBLIC_KEY_BASE58;
extern const String PROGRAM_ID;
extern const String MINT;
extern const String VAULT;
extern const String TOKEN_PROGRAM_ID;

// Function declarations
void getSolBalance();
void confirmTransaction(const String signature);
void calculateDiscriminator(const String function_name);
String createPDAFromSeeds(const std::vector<std::vector<uint8_t>>& customSeeds);
void sendAnchorInstructionWithPDA(
  const String& functionName,
  const std::vector<std::vector<uint8_t>>& customSeeds,
  const std::vector<uint8_t>& payload
);

#endif // SOLANA_UTILS_H 