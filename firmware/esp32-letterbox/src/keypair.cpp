#include "pins_arduino.h"
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include "IoTxChain-lib.h"
#include "SolanaUtils.h"


// Your Solana wallet (Base58 format)
//don't commit
const String PRIVATE_KEY_BASE58 = "PRIVATE_KEY_BASE58";  // 64-byte base58
//don't commit
const String PUBLIC_KEY_BASE58 = "";
