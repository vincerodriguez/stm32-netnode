# stm32-netnode

Usb powered stm32f407-based board that reports temperature and telemetry data over ethernet. BME688 is used for the temp sensor, with 1MB flash. SPI and I2C connectors for something in the future, good to have. 
## Status

- [x] Schematic complete
- [x] PCB layout complete  
- [x] v1 boards ordered from JLCPCB (2026-04-19)
- [x] Bring-up
- [x] Firmware MVP
- [x] Deployment


## Specs

| | |
|---|---|
| MCU | STM32F407VET6 (Cortex-M4F @ 168 MHz, 512 KB flash, 192 KB SRAM, 100-LQFP) |
| Ethernet PHY | Microchip LAN8720A, RMII |
| Magjack | Pulse J0011D01BNL |
| Sensor | Bosch BME688 (temperature, humidity, pressure, VOC) |
| Storage | Winbond W25Q80DVUXIE SPI NOR, 8 MBIT (1MB) |
| Power | USB-C, 5 V → 3.3 V via AP2112K-3.3 LDO |
| PCB | 60 × 60 mm, 4-layer, JLCPCB |
| Firmware | C - LWIP, ST-HAL (planned port to Rust) |

## Repo layout
hardware/ KiCad project structure
docs/ Design documentation and ADRs
firmware/ C firmware / Rust (coming soon)
test/ Bring-up checklists and general bring-up headaches


## Documents

- [Requirements](docs/01-design.md) *(coming)*
- [Architecture](docs/02-architecture.md) 
- [Decision records](docs/03-bringup-analysis.md) *(coming)*

## License

MIT 