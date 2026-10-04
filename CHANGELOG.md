# Changelog

## [v0.2.0] - 2026-10-03
### Fases 3 y 4 completadas + Protocolo de datos v1 validado
- SX1262: las 7 señales SPI/control confirmadas (NSS=PA4, SCK=PA5, MISO=PA6, MOSI=PA7, BUSY=PA2, NRST=PA3, DIO1=PC15)
- LoRa P2P: 920.0 MHz, SF7, BW 125 kHz, CR 4/5, +14 dBm, CRC de radio activado
- Protocolo v1 (3 bytes): ID_NODO, TIPO_EVENTO, SEQ
  - Eventos críticos (0x02, 0x03) reenviados 3 veces con el mismo SEQ
  - Deduplicación en receptor por (ID_NODO, SEQ)
- Validación: 500+ paquetes en ambos nodos, decodificación 100% correcta, deduplicación sin excepciones, CrcErr=0, RSSI entre -48 y -56 dBm, SNR entre 12 y 14 dB
- Corrección: RssiPkt se lee como uint8 sin signo (rssi_dbm = -raw/2); SnrPkt sí es complemento a dos
- Corrección: PA3 (NRST) debe ser salida push-pull en el .ioc
- Estación Base: driver SSD1306 propio en HAL puro (fuente limitada a A-Z, pendiente dígitos)

## [v0.1.0] - 2026-09-25
### Fase 1 completada — Comunicación UART
- USART1 configurado en PA9(TX)/PA10(RX), 115200 baud, 8N1
- Reloj: HSE 8 MHz + PLL x9 → SYSCLK 72 MHz
- Validado: mensaje periódico cada 1s por terminal serie
- Reconstrucción tras pérdida total de archivos por formateo de PC
