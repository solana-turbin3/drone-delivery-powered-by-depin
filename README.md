# Drone Delivery Powered by DePIN

(description)

## Project Structure

```
drone-delivery-powered-by-depin/
├── anchor-drone-delivery/            # Anchor framework files
├── frontend/                         # Next.js
├── backend/                          
├── hardware/                         
│   ├── esp32-drone/                 # delivery drone
│   └── docs/                        # Hardware documentation
└── docs/                          # Architecture diagrams, whitepaper
```

## Hardware Setup

### ESP32 Drone Development

The ESP32 drone code is located in the `hardware/` directory

#### Prerequisites
- [PlatformIO IDE](https://platformio.org/platformio-ide) or [VS Code with PlatformIO extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
- ESP32 development board 
- USB cable for programming

#### Getting Started

1. **Open the project in PlatformIO:**

2. **Configure your WiFi credentials:**
   Edit `src/main.cpp` and update:
   ```cpp
   const char* WIFI_SSID = "your_wifi_ssid";
   const char* WIFI_PASSWORD = "your_wifi_password";
   ```
   Arduino esp32 only supports 2.4Ghz

3. **Configure your pubkey and privkey**

4. **Configure Solana RPC endpoint:**
   Update the RPC URL in `src/SolanaUtils.cpp`:
   ```cpp
   const String RPC_URL = "https://api.devnet.solana.com";  // or your preferred RPC
   ```

5. **Build and upload:**
  
6. **Monitor serial output:**
   


#### Project Configuration

The project uses `platformio.ini` with the following key settings:

- **Board:** Arduino Nano ESP32
- **Framework:** Arduino
- **Libraries:** 
  - IoTxChain (local Solana library)
  - ArduinoJson
  - Cryptographic libraries (SHA256, ECC)

#### Key Features

- **Solana Integration:** Send transactions to Solana blockchain
- **WiFi Connectivity:** ESP32 WiFi capabilities
- **Cryptographic Operations:** Ed25519 signing, Base58 encoding
- **Anchor Program Support:** Interact with Solana Anchor programs
- **Memory:** The project disables Bluetooth to save memory



## Data storage
[Tapedrive](https://tapedrive.io/) (not live yet)
[Data Anchor](https://www.termina.technology/post/data-anchor)

## Rewards distribution
Claim based
Push based - [TukTuk](https://www.tuktuk.fun/docs/overview)


