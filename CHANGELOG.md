# Changelog

## [v0.3.0] - 2026-10-04
### Fase 2 y 2b completadas — MPU de 6 ejes calibrado en Nodo Bovino
- I2C1 (PB6/PB7, 100 kHz), dirección 0x68. WHO_AM_I = 0x70 (el register map del MPU-6050 indica 0x68; 0x70 es consistente con MPU-6500 según fuente secundaria)
- Driver propio (mpu6500.c/.h), HAL puro: lectura en ráfaga de 14 bytes (accel, temperatura, gyro)
- Rango fijado explícitamente: ±2 g (16384 LSB/g) y ±250 °/s (131 LSB/(°/s)), leído de vuelta (0x00 en ambos)
- Salida en unidades físicas con enteros: mg, mdps y centésimas de °C (sin float)
- Registros y sensibilidades verificados contra el Register Map oficial MPU-6000/6050 Rev 4.0 y el datasheet MPU-6500 Rev 1.0
- Calibración del acelerometro (8 poses a mano, ver docs/calibracion_mpu.md): offsets X +11, Y −8, Z +274 mg; ganancias 1.004, 0.997, 1.020
- Offset de Z fuera de la especificación del MPU-6500 (±60 mg); X e Y dentro. Causa sin determinar
- Chequeo fuera de muestra: magnitud en reposo 0.989 g cruda → ≈ 1.000 g corregida
- Offsets provisionales del giroscopio (mdps): gx −830, gy +2310, gz +350 (ZRO spec ±5 °/s: dentro)
- Constantes en mpu_calib.h; MPU_APPLY_CALIB permite salida sin calibrar
- Descarte de las primeras 10 muestras tras el arranque (picos al presionar RST)
### Hallazgos abiertos
- Sesgo de gz cambió entre capturas (+0.3 a +0.7 °/s): posible deriva térmica, se registra temperatura para comprobarlo
- Calibración hecha a mano: incertidumbre ≈ ±6 mg y ±0.5 %; deriva térmica del offset de Z según datasheet ±1 mg/°C
- Conversión de temperatura referencial (datasheet MPU-6500 preliminar), sin validar en esta unidad
- Muestreo por polling a ~10 Hz (HAL_Delay), no determinista: se reemplaza por timer de hardware en Fase 6
### Nota de ramas
- El firmware TX del protocolo v1 en NodoBovino/Core/Src/main.c queda en el tag v0.2.0; la integración con el MPU se hace en Fase 13

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
