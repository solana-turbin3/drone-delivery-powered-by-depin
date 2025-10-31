# Drone Delivery Powered by DePIN

A decentralized drone delivery system built on Solana, enabling automated delivery coordination between drones and letterboxes through on-chain state management and events.

## Project Structure

```
drone-delivery-powered-by-depin/
├── anchor-drone-delivery-marketplace/  # Marketplace Anchor program
├── drone-contract/                     # Drone Delivery Anchor program
│   ├── programs/
│   │   └── delivery-events/           # Main contract (drone/letterbox state)
│   └── tests/                          # Contract tests
├── firmware/                           
│   ├── esp32-letterbox/               # ESP32 letterbox firmware 
│   ├── raspberry-pi5-drone/           # Raspberry Pi 5 drone controller
│   └── eventcpp/                      # ESP32 drone firmware
├── mobile-depin-drone/                # Mobile application
└── docs/                               # Architecture diagrams, whitepaper
    └── arc diagram/                    # System architecture diagrams
```

## Solana Contracts

### Drone Delivery Contract (`drone-contract/`)

The main on-chain program for managing drone and letterbox state:

- **Drone State**: Tracks drone operator, status (Available, ReadyToFly, Arrived, UnAvailable, Error), and last update timestamp
- **Letterbox State**: Tracks letterbox owner, status (Open, Closed, Error), and last update timestamp
- **Events**: Emits on-chain events for drone status changes and letterbox operations

**Key Instructions:**
- `initialize`: Initialize drone or letterbox accounts
- `update_dronestatus`: Update drone status and emit events
- `update_letterboxstatus`: Update letterbox status

**Testing:**
```bash
cd drone-contract
anchor test
```

### Marketplace Contract (`anchor-drone-delivery-marketplace/`)

Marketplace for coordinating delivery orders and payments.

## Hardware Setup

For detailed hardware setup and firmware development instructions, see [firmware/README.md](firmware/README.md).

The firmware directory contains:
- **ESP32 Letterbox** (`firmware/esp32-letterbox/`) - ESP32-based letterbox that monitors status and updates Solana
- **ESP32 Drone** (`firmware/eventcpp/`) - ESP32-based delivery drone firmware with Solana integration
- **Raspberry Pi 5 Drone** (`firmware/raspberry-pi5-drone/`) - Raspberry Pi 5 drone controller
- Other firmware projects for testing and development



## Data Storage

- **[Tapedrive](https://tapedrive.io/)** (not live yet) - Decentralized data storage solution
- **[Data Anchor](https://www.termina.technology/post/data-anchor)** - Anchor-based data storage patterns

## Rewards Distribution

- **Claim based**: Users manually claim rewards after completing deliveries
- **Push based**: Automated reward distribution using [TukTuk](https://www.tuktuk.fun/docs/overview)

## Development

### Prerequisites

- **Rust**: For Solana program development
- **Anchor**: Solana framework (v0.31.1)
- **Node.js & Yarn**: For TypeScript tests
- **PlatformIO**: For firmware development
- **Solana CLI**: For local validator and deployments

### Building Contracts

```bash
# Build delivery events contract
cd drone-contract
anchor build

# Build marketplace contract
cd ../anchor-drone-delivery-marketplace
anchor build
```

### Running Tests

```bash
cd drone-contract
anchor test
```


