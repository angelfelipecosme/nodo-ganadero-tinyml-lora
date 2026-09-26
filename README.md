# Nodo Ganadero Cognitivo de Doble Umbral

Diseño e implementación de un nodo ganadero basado en TinyML y estimación de 
proximidad relativa por RSSI mediante LoRa, para zonas rurales sin conectividad.

## Estado actual

- ✅ Fase 1 — Comunicación UART validada (STM32F103C8T6, USART1, 115200 baud)
- ⬜ Fase 2 — MPU6050
- ⬜ Fase 3 — SX1262
- ⬜ Fase 4 — LoRa P2P
- ⬜ Fase 5 — OLED
- ⬜ Fase 6 — Adquisición MPU6050
- ⬜ Fase 7 — Dataset y TinyML
- ⬜ Fase 8 — Modelo TinyML embebido
- ⬜ Fase 9 — RSSI
- ⬜ Fase 10 — Proximidad relativa
- ⬜ Fase 11 — Máquina de estados
- ⬜ Fase 12 — Deep Sleep
- ⬜ Fase 13 — Integración completa
- ⬜ Fase 14 — Pruebas de campo

## Hardware

- 2x DX-SMART DX-PJ26-V1.1 (STM32F103C8T6)
- 2x DX-LR30-900M22S (SX1262, banda 915–928 MHz, regulación ANE Colombia)
- MPU6050
- OLED 0.96" I2C

## Herramientas

- STM32CubeMX / STM32CubeIDE / STM32CubeProgrammer
- Programación vía bootloader UART (sin ST-Link)

## Ver [CHANGELOG.md](CHANGELOG.md) para el detalle de cada versión.