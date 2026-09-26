# Changelog

## [v0.1.0] - 2026-09-25
### Fase 1 completada — Comunicación UART
- USART1 configurado en PA9(TX)/PA10(RX), 115200 baud, 8N1
- Reloj: HSE 8 MHz + PLL x9 → SYSCLK 72 MHz
- Validado: mensaje periódico cada 1s por terminal serie
- Reconstrucción tras pérdida total de archivos por formateo de PC