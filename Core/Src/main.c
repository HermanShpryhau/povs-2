/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define T_25   1.43f      // Напряжение (в вольтах) на датчике при температуре 25 °C.
#define T_SLOPE  0.0043f    // Изменение напряжения (в вольтах) при изменении температуры на градус.
#define V_REF    3.3f       // Образцовое напряжение АЦП (в вольтах).
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#define SMB_0 0b00000011
#define SMB_1 0b10011111
#define SMB_2 0b00100101
#define SMB_3 0b00001101
#define SMB_4 0b10011001
#define SMB_5 0b01001001
#define SMB_6 0b01000001
#define SMB_7 0b00011111
#define SMB_8 0b00000001
#define SMB_9 0b00001001
#define SMB_NONE 0b11111111

#define DIG_1 0b10001111
#define DIG_2 0b01001111
#define DIG_3 0b00100000
#define DIG_4 0b00010000
const uint8_t NUMBERS[] = {
        SMB_0,
        SMB_1,
        SMB_2,
        SMB_3,
        SMB_4,
        SMB_5,
        SMB_6,
        SMB_7,
        SMB_8,
        SMB_9};

const uint8_t SEGMENTS[] = {DIG_1, DIG_2, DIG_3, DIG_4};

uint8_t NUMBER[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int8_t numberSize = 10;

uint8_t isNumberSizeChoice = 0;

uint8_t isGenerating = 0;

uint16_t drawingDelay = 300;

int8_t currPos = -4;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

static void MX_GPIO_Init(void);

static void MX_USART2_UART_Init(void);

static void MX_TIM1_Init(void);

static void MX_ADC1_Init(void);

/* USER CODE BEGIN PFP */
uint8_t nextRandom();

void writeNumberToDisplay(uint8_t number, uint8_t segment);

void writeByteToRegister(uint8_t number);

void generateNewNumber();

void animateLeds();

void drawNumbers(int8_t begin);

void drawNumber(uint8_t number);

void drawClearDisplay();

uint8_t debounceButton();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_ADC_Start(&hadc1);
  float temp = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);
    drawClearDisplay();
    HAL_ADC_PollForConversion(&hadc1, 1000);
    uint32_t adcVal = HAL_ADC_GetValue(&hadc1);
    temp = (float) adcVal / 4096 * V_REF;
    temp = (temp - T_25) / T_SLOPE + 25;
    HAL_StatusTypeDef result = HAL_UART_Transmit(&huart2, (uint8_t *) &temp, sizeof(float), HAL_MAX_DELAY);
    if (result == HAL_OK)
    {
      HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    }
    HAL_Delay(1000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_DISABLE;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD1_Pin | LD2_Pin | LD3_Pin | DISPLAY_CLK_Pin
                           | DISPLAY_DATA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DISPLAY_LATCH_Pin | LD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : B_START_Pin B_DOWN_Pin */
  GPIO_InitStruct.Pin = B_START_Pin | B_DOWN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD2_Pin LD3_Pin DISPLAY_CLK_Pin
                           DISPLAY_DATA_Pin */
  GPIO_InitStruct.Pin = LD1_Pin | LD2_Pin | LD3_Pin | DISPLAY_CLK_Pin
                        | DISPLAY_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : B_UP_Pin */
  GPIO_InitStruct.Pin = B_UP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B_UP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DISPLAY_LATCH_Pin LD4_Pin */
  GPIO_InitStruct.Pin = DISPLAY_LATCH_Pin | LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
uint32_t lastStartPress = 0;
uint32_t lastBtnPress = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
    case B_START_Pin:
      if ((HAL_GetTick() - lastBtnPress) < 100)
      {
        return;
      }
      if ((HAL_GetTick() - lastStartPress) < 700)
      {
        if (isNumberSizeChoice == 0)
        {
          isNumberSizeChoice = 1;
        }
        lastStartPress = HAL_GetTick();
        break;
      }
      if (isNumberSizeChoice == 1)
      {
        isNumberSizeChoice = 0;
      }
      lastStartPress = HAL_GetTick();
      generateNewNumber();
      lastBtnPress = HAL_GetTick();
      break;
    case B_DOWN_Pin:
      if ((HAL_GetTick() - lastBtnPress) < 100)
      {
        return;
      }
      if (isNumberSizeChoice)
      {
        if (numberSize - 1 >= 1)
        {
          numberSize--;
        }
      } else
      {
        if (drawingDelay < 650)
        {
          drawingDelay += 50;
        }
      }
      lastBtnPress = HAL_GetTick();
      break;
    case B_UP_Pin:
      if ((HAL_GetTick() - lastBtnPress) < 100)
      {
        return;
      }
      if (isNumberSizeChoice)
      {
        if (numberSize + 1 <= 15)
        {
          numberSize++;
        }
      } else
      {
        if (drawingDelay > 150)
        {
          drawingDelay -= 50;
        }
      }
      lastBtnPress = HAL_GetTick();
      break;
  }
}

void writeNumberToDisplay(uint8_t number, uint8_t segment)
{
  writeByteToRegister(number);
  writeByteToRegister(segment);
  HAL_GPIO_WritePin(DISPLAY_LATCH_GPIO_Port, DISPLAY_LATCH_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DISPLAY_LATCH_GPIO_Port, DISPLAY_LATCH_Pin, GPIO_PIN_SET);
}

void writeByteToRegister(uint8_t number)
{
  for (int i = 0; i < 8; i++)
  {
    HAL_GPIO_WritePin(DISPLAY_CLK_GPIO_Port, DISPLAY_CLK_Pin, GPIO_PIN_RESET);
    uint8_t bit = (number & (0b00000001 << i));
    HAL_GPIO_WritePin(DISPLAY_DATA_GPIO_Port, DISPLAY_DATA_Pin, bit == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(DISPLAY_CLK_GPIO_Port, DISPLAY_CLK_Pin, GPIO_PIN_SET);
  }
}

void drawNumbers(int8_t begin)
{
  for (int8_t i = 0; i < 4; i++)
  {
    if (begin + i < numberSize && begin + i >= 0)
    {
      writeNumberToDisplay(NUMBERS[NUMBER[begin + i]], SEGMENTS[i]);
    } else
    {
      writeNumberToDisplay(0xFF, SEGMENTS[i]);
    }
  }
}

void drawNumber(uint8_t number)
{
  uint8_t t = number / 1000;
  uint8_t h = number / 100;
  uint8_t d = number / 10;
  uint8_t o = number % 10;
  if (t > 0)
  {
    writeNumberToDisplay(NUMBERS[t], SEGMENTS[0]);
  }
  if (h > 0)
  {
    writeNumberToDisplay(NUMBERS[h], SEGMENTS[1]);
  }
  if (d > 0)
  {
    writeNumberToDisplay(NUMBERS[d], SEGMENTS[2]);
  }
  if (o > 0)
  {
    writeNumberToDisplay(NUMBERS[o], SEGMENTS[3]);
  } else
  {
    writeNumberToDisplay(NUMBERS[0], SEGMENTS[3]);
  }
}

uint32_t seed = 7;
uint32_t prev = 1;

uint8_t nextRandom()
{
  uint32_t res = prev * seed % 10;
  prev = res;
  return (uint8_t) res;
}

void generateNewNumber()
{
  isGenerating = 1;
  currPos = -4;
  for (uint8_t i = 0; i < numberSize; i++)
  {
    NUMBER[i] = nextRandom();
  }
}

uint32_t animationStart = 0;

void animateLeds()
{
  drawClearDisplay();
  if (isGenerating)
  {
    HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);

    if (animationStart == 0)
      animationStart = HAL_GetTick();

    if (HAL_GetTick() - animationStart > 200)
      HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);

    if (HAL_GetTick() - animationStart > 400)
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    if (HAL_GetTick() - animationStart > 600)
      HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

    if (HAL_GetTick() - animationStart > 800)
    {
      HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);
    }

    if (HAL_GetTick() - animationStart > 1200)
    {
      isGenerating = 0;
      animationStart = 0;
    }
  }
}

void drawClearDisplay()
{
  writeNumberToDisplay(SMB_NONE, SEGMENTS[0]);
  writeNumberToDisplay(SMB_NONE, SEGMENTS[1]);
  writeNumberToDisplay(SMB_NONE, SEGMENTS[2]);
  writeNumberToDisplay(SMB_NONE, SEGMENTS[3]);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
