# Nodo Ganadero Cognitivo de Doble Umbral

Diseño e implementación de un nodo ganadero basado en TinyML y estimación de proximidad relativa por RSSI mediante LoRa, para zonas rurales sin conectividad.

Proyecto de grado — Ingeniería en Telecomunicaciones, Universidad Compensar (Meta, Colombia).

## Estado actual

- [x] **Fase 1** — Comunicación UART validada (STM32F103C8T6, USART1, PA9/PA10, 115200 baud, 8N1)
- [ ] Fase 2 — MPU6050 (pendiente: sensor aún no disponible)
- [x] **Fase 3** — SX1262 validado (7 señales SPI/control confirmadas experimentalmente en ambos nodos: NSS, SCK, MISO, MOSI, BUSY, NRST, DIO1)
- [x] **Fase 4** — Comunicación LoRa P2P funcional (920.0 MHz, SF7, BW=125 kHz, CR=4/5, +14 dBm)
- [ ] Fase 5 — OLED (Estación Base)
- [ ] Fase 6 — Adquisición de datos del MPU6050
- [ ] Fase 7 — Dataset y TinyML
- [ ] Fase 8 — Implementación del modelo TinyML en STM32
- [ ] Fase 9 — RSSI (estimación de proximidad relativa)
- [ ] Fase 10 — Estimación de proximidad relativa
- [ ] Fase 11 — Máquina de estados
- [ ] Fase 12 — Deep Sleep y optimización energética
- [ ] Fase 13 — Integración completa
- [ ] Fase 14 — Pruebas de campo

### Protocolo de datos (Sección 16)

- **Implementado** (pendiente de validar en hardware): payload de 3 bytes.

| Byte | Campo | Descripción |
|---|---|---|
| 0 | `ID_NODO` | Identificador del nodo (0x00–0xFF) |
| 1 | `TIPO_EVENTO` | `0x00`=NORMAL, `0x01`=ANOMALÍA_COMPORTAMENTAL, `0x02`=FUERA_DE_ZONA, `0x03`=EVENTO_COMBINADO |
| 2 | `SEQ` | Contador de secuencia (0x00–0xFF, con rollover) |

- Sin CRC de aplicación — se usa el CRC de radio del SX1262 (ya activado en la configuración de paquete).
- Sin ACK — eventos críticos (`FUERA_DE_ZONA`, `EVENTO_COMBINADO`) se reenvían 3 veces con el mismo `SEQ`; el receptor descarta duplicados comparando `(ID_NODO, SEQ)`.

## Hardware

| Componente | Detalle |
|---|---|
| Placa principal (ambos nodos) | DX-SMART DX-PJ26-V1.1 — STM32F103C8T6 (Cortex-M3, LQFP48) |
| Módulo LoRa (ambos nodos) | DX-LR30-900M22S — chip SX1262, socket integrado en la placa principal |
| Nodo Bovino | Módulo A — transmisor. Futuro: MPU6050 + TinyML |
| Estación Base | Módulo B — receptor. Futuro: OLED 0.96" (I2C) |

### Mapeo de pines SX1262 (confirmado experimentalmente, Fase 3)

| Señal | Pin STM32 |
|---|---|
| NSS | PA4 |
| SCK | PA5 |
| MISO | PA6 |
| MOSI | PA7 |
| BUSY | PA2 |
| NRST | PA3 |
| DIO1 | PC15 |

### Parámetros de radio (confirmados, Fase 4)

- Frecuencia: 920.0 MHz (dentro de la ventana de uso libre 915–928 MHz vigente en Colombia — ver Resolución ANE 28 de 2026)
- Modulación LoRa: SF7, BW=125 kHz, CR=4/5, LowDataRateOptimize=OFF
- Potencia TX: +14 dBm (PaConfig: paDutyCycle=0x02, hpMax=0x02, deviceSel=0x00, paLut=0x01)
- CRC de paquete: activado

## Estructura del repositorio
## Herramientas

- STM32CubeMX / STM32CubeIDE / STM32CubeProgrammer
- Programación vía bootloader UART (sin ST-Link)

## Ver [CHANGELOG.md](CHANGELOG.md) para el detalle de cada versión.
