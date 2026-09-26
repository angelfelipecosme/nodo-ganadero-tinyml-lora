/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - NODO A (TRANSMISOR) - Protocolo real v1
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stddef.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
#define SX_NSS_PIN   GPIO_PIN_4
#define SX_NSS_PORT  GPIOA
#define SX_NRST_PIN  GPIO_PIN_3
#define SX_NRST_PORT GPIOA
#define SX_BUSY_PIN  GPIO_PIN_2
#define SX_BUSY_PORT GPIOA
#define SX_DIO1_PIN  GPIO_PIN_15
#define SX_DIO1_PORT GPIOC

#define OP_GET_STATUS        0xC0
#define OP_SET_STANDBY       0x80
#define OP_SET_PACKET_TYPE   0x8A
#define OP_SET_RF_FREQUENCY  0x86
#define OP_SET_PA_CONFIG     0x95
#define OP_SET_TX_PARAMS     0x8E
#define OP_SET_BUFFER_BASE   0x8F
#define OP_WRITE_BUFFER      0x0E
#define OP_SET_MOD_PARAMS    0x8B
#define OP_SET_PACKET_PARAMS 0x8C
#define OP_SET_DIO_IRQ       0x08
#define OP_GET_IRQ_STATUS    0x12
#define OP_CLEAR_IRQ_STATUS  0x02
#define OP_SET_TX            0x83

#define STDBY_RC        0x00
#define PACKET_TYPE_LORA 0x01

#define IRQ_TXDONE_MASK  0x0001
#define IRQ_TIMEOUT_MASK 0x0200

#define SF7   0x07
#define BW_125 0x04
#define CR_4_5 0x01
#define LDRO_OFF 0x00

/* --- Protocolo de datos v1 (Seccion 16, confirmado con Angel) --- */
#define MY_NODE_ID     0x01
#define EVT_NORMAL         0x00
#define EVT_ANOMALIA        0x01
#define EVT_FUERA_DE_ZONA   0x02
#define EVT_COMBINADO       0x03
#define PACKET_LEN 3  /* ID_NODO + TIPO_EVENTO + SEQ */

#define RF_FREQ_920MHZ 0x39800000UL
#define TX_TIMEOUT_STEPS 128000UL
/* USER CODE END PV */

void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void SX1262_Reset(void);
uint8_t SX1262_WaitBusy(uint32_t timeout_ms);
uint8_t SX1262_GetStatus(void);
void SX1262_DecodeStatus(uint8_t status, char* out, size_t out_size);
void SX1262_SetStandby(uint8_t stdbyConfig);
void SX1262_SetPacketType(uint8_t packetType);
void SX1262_SetRfFrequency(uint32_t rfFreq);
void SX1262_SetPaConfig(uint8_t paDutyCycle, uint8_t hpMax, uint8_t deviceSel, uint8_t paLut);
void SX1262_SetTxParams(int8_t power, uint8_t rampTime);
void SX1262_SetBufferBaseAddress(uint8_t txBase, uint8_t rxBase);
void SX1262_WriteBuffer(uint8_t offset, uint8_t* data, uint8_t len);
void SX1262_SetModulationParamsLoRa(uint8_t sf, uint8_t bw, uint8_t cr, uint8_t ldro);
void SX1262_SetPacketParamsLoRa(uint16_t preambleLen, uint8_t headerType, uint8_t payloadLen, uint8_t crcType, uint8_t invertIq);
void SX1262_SetDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask);
uint16_t SX1262_GetIrqStatus(void);
void SX1262_ClearIrqStatus(uint16_t clearMask);
void SX1262_SetTx(uint32_t timeoutSteps);
const char* EventName(uint8_t evt);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void SX1262_Reset(void)
{
    HAL_GPIO_WritePin(SX_NRST_PORT, SX_NRST_PIN, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(SX_NRST_PORT, SX_NRST_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
}

uint8_t SX1262_WaitBusy(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while (HAL_GPIO_ReadPin(SX_BUSY_PORT, SX_BUSY_PIN) == GPIO_PIN_SET)
    {
        if ((HAL_GetTick() - start) > timeout_ms) return 0;
    }
    return 1;
}

static void SX1262_SPI(uint8_t* tx, uint8_t* rx, uint16_t len)
{
    SX1262_WaitBusy(1000);
    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, len, 200);
    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);
    SX1262_WaitBusy(1000);
}

uint8_t SX1262_GetStatus(void)
{
    uint8_t tx[2] = { OP_GET_STATUS, 0x00 };
    uint8_t rx[2] = { 0 };
    SX1262_SPI(tx, rx, 2);
    return rx[1];
}

void SX1262_DecodeStatus(uint8_t status, char* out, size_t out_size)
{
    uint8_t chip_mode = (status >> 4) & 0x07;
    const char* mode_str;
    switch (chip_mode)
    {
        case 0x2: mode_str = "STBY_RC"; break;
        case 0x3: mode_str = "STBY_XOSC"; break;
        case 0x4: mode_str = "FS"; break;
        case 0x5: mode_str = "RX"; break;
        case 0x6: mode_str = "TX"; break;
        default:  mode_str = "DESCONOCIDO"; break;
    }
    snprintf(out, out_size, "Mode=%s (0x%X)", mode_str, chip_mode);
}

void SX1262_SetStandby(uint8_t stdbyConfig)
{
    uint8_t tx[2] = { OP_SET_STANDBY, stdbyConfig };
    uint8_t rx[2] = { 0 };
    SX1262_SPI(tx, rx, 2);
}

void SX1262_SetPacketType(uint8_t packetType)
{
    uint8_t tx[2] = { OP_SET_PACKET_TYPE, packetType };
    uint8_t rx[2] = { 0 };
    SX1262_SPI(tx, rx, 2);
}

void SX1262_SetRfFrequency(uint32_t rfFreq)
{
    uint8_t tx[5], rx[5] = {0};
    tx[0] = OP_SET_RF_FREQUENCY;
    tx[1] = (uint8_t)(rfFreq >> 24);
    tx[2] = (uint8_t)(rfFreq >> 16);
    tx[3] = (uint8_t)(rfFreq >> 8);
    tx[4] = (uint8_t)(rfFreq & 0xFF);
    SX1262_SPI(tx, rx, 5);
}

void SX1262_SetPaConfig(uint8_t paDutyCycle, uint8_t hpMax, uint8_t deviceSel, uint8_t paLut)
{
    uint8_t tx[5] = { OP_SET_PA_CONFIG, paDutyCycle, hpMax, deviceSel, paLut };
    uint8_t rx[5] = {0};
    SX1262_SPI(tx, rx, 5);
}

void SX1262_SetTxParams(int8_t power, uint8_t rampTime)
{
    uint8_t tx[3] = { OP_SET_TX_PARAMS, (uint8_t)power, rampTime };
    uint8_t rx[3] = {0};
    SX1262_SPI(tx, rx, 3);
}

void SX1262_SetBufferBaseAddress(uint8_t txBase, uint8_t rxBase)
{
    uint8_t tx[3] = { OP_SET_BUFFER_BASE, txBase, rxBase };
    uint8_t rx[3] = {0};
    SX1262_SPI(tx, rx, 3);
}

void SX1262_WriteBuffer(uint8_t offset, uint8_t* data, uint8_t len)
{
    uint8_t tx[2 + 32], rx[2 + 32] = {0};
    tx[0] = OP_WRITE_BUFFER;
    tx[1] = offset;
    memcpy(&tx[2], data, len);
    SX1262_SPI(tx, rx, len + 2);
}

void SX1262_SetModulationParamsLoRa(uint8_t sf, uint8_t bw, uint8_t cr, uint8_t ldro)
{
    uint8_t tx[5] = { OP_SET_MOD_PARAMS, sf, bw, cr, ldro };
    uint8_t rx[5] = {0};
    SX1262_SPI(tx, rx, 5);
}

void SX1262_SetPacketParamsLoRa(uint16_t preambleLen, uint8_t headerType, uint8_t payloadLen, uint8_t crcType, uint8_t invertIq)
{
    uint8_t tx[7], rx[7] = {0};
    tx[0] = OP_SET_PACKET_PARAMS;
    tx[1] = (uint8_t)(preambleLen >> 8);
    tx[2] = (uint8_t)(preambleLen & 0xFF);
    tx[3] = headerType;
    tx[4] = payloadLen;
    tx[5] = crcType;
    tx[6] = invertIq;
    SX1262_SPI(tx, rx, 7);
}

void SX1262_SetDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask)
{
    uint8_t tx[9], rx[9] = {0};
    tx[0] = OP_SET_DIO_IRQ;
    tx[1] = (uint8_t)(irqMask >> 8);   tx[2] = (uint8_t)(irqMask & 0xFF);
    tx[3] = (uint8_t)(dio1Mask >> 8);  tx[4] = (uint8_t)(dio1Mask & 0xFF);
    tx[5] = (uint8_t)(dio2Mask >> 8);  tx[6] = (uint8_t)(dio2Mask & 0xFF);
    tx[7] = (uint8_t)(dio3Mask >> 8);  tx[8] = (uint8_t)(dio3Mask & 0xFF);
    SX1262_SPI(tx, rx, 9);
}

uint16_t SX1262_GetIrqStatus(void)
{
    uint8_t tx[4] = { OP_GET_IRQ_STATUS, 0x00, 0x00, 0x00 };
    uint8_t rx[4] = { 0 };
    SX1262_SPI(tx, rx, 4);
    return (uint16_t)((rx[2] << 8) | rx[3]);
}

void SX1262_ClearIrqStatus(uint16_t clearMask)
{
    uint8_t tx[3] = { OP_CLEAR_IRQ_STATUS, (uint8_t)(clearMask >> 8), (uint8_t)(clearMask & 0xFF) };
    uint8_t rx[3] = {0};
    SX1262_SPI(tx, rx, 3);
}

void SX1262_SetTx(uint32_t timeoutSteps)
{
    uint8_t tx[4], rx[4] = {0};
    tx[0] = OP_SET_TX;
    tx[1] = (uint8_t)((timeoutSteps >> 16) & 0xFF);
    tx[2] = (uint8_t)((timeoutSteps >> 8) & 0xFF);
    tx[3] = (uint8_t)(timeoutSteps & 0xFF);
    SX1262_SPI(tx, rx, 4);
}

const char* EventName(uint8_t evt)
{
    switch (evt)
    {
        case EVT_NORMAL:        return "NORMAL";
        case EVT_ANOMALIA:      return "ANOMALIA_COMPORTAMENTAL";
        case EVT_FUERA_DE_ZONA: return "FUERA_DE_ZONA";
        case EVT_COMBINADO:     return "EVENTO_COMBINADO";
        default:                return "DESCONOCIDO";
    }
}
/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  char msg[140];
  char decoded[64];
  uint8_t seq = 0;
  /* SIMULACION: ciclo de eventos de demostracion. Se reemplaza por la salida
     real de TinyML/MPU6050 en Fase 6-8. */
  const uint8_t eventCycle[] = { EVT_NORMAL, EVT_ANOMALIA, EVT_FUERA_DE_ZONA, EVT_COMBINADO };
  uint8_t cycleIdx = 0;
  /* USER CODE END 1 */

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);
  SX1262_Reset();
  SX1262_WaitBusy(1000);

  SX1262_SetStandby(STDBY_RC);
  SX1262_SetPacketType(PACKET_TYPE_LORA);
  SX1262_SetRfFrequency(RF_FREQ_920MHZ);

  /* Perfil +14 dBm confirmado (Tabla 13-21 del datasheet) */
  SX1262_SetPaConfig(0x02, 0x02, 0x00, 0x01);
  SX1262_SetTxParams(22, 0x02);

  SX1262_SetBufferBaseAddress(0x00, 0x00);
  SX1262_SetModulationParamsLoRa(SF7, BW_125, CR_4_5, LDRO_OFF);
  SX1262_SetPacketParamsLoRa(8, 0x00, PACKET_LEN, 0x01, 0x00);

  SX1262_SetDioIrqParams(IRQ_TXDONE_MASK | IRQ_TIMEOUT_MASK,
                          IRQ_TXDONE_MASK | IRQ_TIMEOUT_MASK, 0x0000, 0x0000);
  SX1262_ClearIrqStatus(0xFFFF);

  sprintf(msg, "=== NODO A (TX) - Protocolo v1 - Nodo ID=0x%02X ===\r\n", MY_NODE_ID);
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
      uint8_t evt = eventCycle[cycleIdx];
      cycleIdx = (cycleIdx + 1) % 4;

      /* Eventos criticos (FUERA_DE_ZONA, COMBINADO) se reenvian 3 veces con el
         mismo SEQ para aumentar probabilidad de entrega, sin usar ACK (decision
         confirmada con Angel para esta version del protocolo). */
      uint8_t copies = (evt == EVT_FUERA_DE_ZONA || evt == EVT_COMBINADO) ? 3 : 1;

      for (uint8_t c = 0; c < copies; c++)
      {
          uint8_t payload[PACKET_LEN];
          payload[0] = MY_NODE_ID;
          payload[1] = evt;
          payload[2] = seq;

          SX1262_WriteBuffer(0x00, payload, PACKET_LEN);
          SX1262_ClearIrqStatus(0xFFFF);
          SX1262_SetTx(TX_TIMEOUT_STEPS);

          uint32_t start = HAL_GetTick();
          while (HAL_GPIO_ReadPin(SX_DIO1_PORT, SX_DIO1_PIN) == GPIO_PIN_RESET)
          {
              if ((HAL_GetTick() - start) > 2500) break;
          }

          uint16_t irq = SX1262_GetIrqStatus();
          uint8_t status = SX1262_GetStatus();
          SX1262_DecodeStatus(status, decoded, sizeof(decoded));

          sprintf(msg, "TX ID=0x%02X EVT=%s(0x%02X) SEQ=%d copia=%d/%d | IrqStatus=0x%04X (TxDone=%d) | %s\r\n",
                  MY_NODE_ID, EventName(evt), evt, seq, c + 1, copies,
                  irq, (int)(irq & 0x01), decoded);
          HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

          SX1262_ClearIrqStatus(0xFFFF);

          if (c < copies - 1) HAL_Delay(150);
      }

      seq++;
      HAL_Delay(2000);
  }
  /* USER CODE END WHILE */
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
