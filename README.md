# stm32-netnode

Usb powered stm32f407-based board that reports temperature and telemetry data over ethernet. BME688 is used for the temp sensor. Expansion header provided for future connectivity.

## Status

- [x] Schematic complete
- [x] PCB layout complete  
- [x] v1 boards ordered from JLCPCB (2026-04-19)
- [ ] Bring-up
- [ ] Firmware MVP
- [ ] Deployment


## Specs

| | |
|---|---|
| MCU | STM32F407VET6 (Cortex-M4F @ 168 MHz, 512 KB flash, 192 KB SRAM, 100-LQFP) |
| Ethernet PHY | Microchip LAN8720A, RMII |
| Magjack | Pulse J0011D01BNL |
| Sensor | Bosch BME688 (temperature, humidity, pressure, VOC) |
| Storage | Winbond W25Q80DVUXIE SPI NOR, 8 MBIT |
| USB console | Silicon Labs CP2102N |
| Power | USB-C, 5 V → 3.3 V via AP2112K-3.3 LDO |
| PCB | 60 × 60 mm, 4-layer, JLCPCB |
| Firmware | Rust (embassy-stm32 + smoltcp) — planned |

## Repo layout
hardware/ KiCad project structure
docs/ Design documentation and ADRs
firmware/ Rust/C firmware (coming soon)
test/ Bring-up checklists and general bring-up headaches


## Documents

- [Requirements](docs/01-requirements.md) *(coming)*
- [Architecture](docs/02-architecture.md) *(coming)*
- [Decision records](docs/decisions/) *(coming)*

## License

MIT — see [LICENSE](LICENSE).