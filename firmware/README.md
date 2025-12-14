# Firmware Development

This directory contains firmware for various hardware components of the drone delivery system.

## Hardware Projects

### ESP32 Letterbox (`esp32-letterbox/`)

ESP32-based letterbox that monitors its status and updates the Solana blockchain.

### ESP32 Drone (`eventcpp/`)

ESP32-based delivery drone firmware with Solana blockchain integration.

The ESP32 firmware projects are located in the `firmware/` directory

## Prerequisites

- [PlatformIO IDE](https://platformio.org/platformio-ide) or [VS Code with PlatformIO extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
- ESP32 development board 
- USB cable for programming

## Getting Started

1. **Open the project in PlatformIO:**
   - For letterbox: Open `firmware/esp32-letterbox/` 
   - For drone: Open `firmware/eventcpp/`

2. **Configure your WiFi credentials:**
   Edit `src/main.cpp` or `src/ssid.cpp` and update:
   ```cpp
   const char* WIFI_SSID = "your_wifi_ssid";
   const char* WIFI_PASSWORD = "your_wifi_password";
   ```
   Note: Arduino ESP32 only supports 2.4GHz WiFi

3. **Configure your Solana keypair:**
   - Update keypair in `src/keypair.cpp`
   - Ensure your keypair has SOL for transaction fees

4. **Configure Solana RPC endpoint:**
   Update the RPC URL in `src/SolanaUtils.cpp`:
   ```cpp
   const String RPC_URL = "https://api.devnet.solana.com";  // or your preferred RPC
   ```

5. **Build and upload:**
   ```bash
   pio run -t upload
   ```

6. **Monitor serial output:**
   ```bash
   pio device monitor
   ```

## Project Configuration

The firmware projects use `platformio.ini` with the following key settings:

- **Board:** ESP32-based boards (varies by project)
- **Framework:** Arduino
- **Libraries:** 
  - IoTxChain (local Solana library for ESP32)
  - ArduinoJson
  - Cryptographic libraries (SHA256, ECC, Ed25519)

## Key Features

- **Solana Integration:** Send transactions and read account state from Solana blockchain
- **WiFi Connectivity:** ESP32 WiFi capabilities for blockchain communication
- **Cryptographic Operations:** Ed25519 signing, Base58 encoding
- **Anchor Program Support:** Interact with Solana Anchor programs via instruction encoding
- **Event Monitoring:** Listen for on-chain events (letterbox firmware)
- **Memory Optimization:** Bluetooth disabled to conserve memory on ESP32

## Other Firmware Projects

- **raspberry-pi5-drone/**: Raspberry Pi 5 drone controller


