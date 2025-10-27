#ifndef SOLANA_UTILS_H
#define SOLANA_UTILS_H

#include <Arduino.h>
#include <vector>
#include "IoTxChain-lib.h"

// Function declarations
bool sendAnchorInstruction(
    const String& functionName,
    const std::vector<std::vector<uint8_t>>& customSeeds,
    const std::vector<uint8_t>& payload,
    bool isInit
);

String createPDAFromSeeds(const std::vector<std::vector<uint8_t>>& customSeeds);

// External variables needed
extern IoTxChain solana;
extern String PROGRAM_ID;
extern String PUBLIC_KEY_BASE58;
extern uint8_t PRIVATE_KEY[64];

#endif