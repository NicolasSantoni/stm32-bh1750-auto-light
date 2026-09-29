/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bh1750.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
  uint16_t manualCountdown;
} Contexto;

typedef enum {
  INIT,
  AGUARDANDO,
  MEDIR,
  CLARO,
  ESCURO,
  MANUAL,
  ERRO
} EstadoID;

typedef EstadoID EstadoFunc(Contexto *ctx);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LUZ_MIN 10
#define LUZ_MAX 500
#define MANUAL_TIMEOUT_TICKS 50 // 50 * 200ms = 10s
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t g_timerFlag = 0;
volatile uint8_t g_buttonFlag = 0;
float lux = 0;
float lux_minima = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch) {
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

static EstadoID init(Contexto *ctx) {
  printf("Estado: INIT\r\n");
  return AGUARDANDO;
}

static EstadoID aguardando(Contexto *ctx) {
  if (g_buttonFlag) {
    g_buttonFlag = 0;
    printf("LED ligado - 10s\r\n");
    ctx->manualCountdown = MANUAL_TIMEOUT_TICKS;
    return MANUAL;
  }
  if (g_timerFlag) {
    g_timerFlag = 0;
    return MEDIR;
  }
  __WFI(); // dorme ate a proxima interrupcao (timer ou botao)
  return AGUARDANDO;
}

static EstadoID medir(Contexto *ctx) {
  uint32_t potenciometro = 0;
  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
    potenciometro = HAL_ADC_GetValue(&hadc1);
  }
  HAL_ADC_Stop(&hadc1);
  lux_minima = LUZ_MIN + (potenciometro / 4095.0f) * (LUZ_MAX - LUZ_MIN);

  HAL_StatusTypeDef st_read = BH1750_ReadLux(&hi2c1, &lux);
  printf(
      "Luz: %.2f Minímo: %.1f \r\n",
      lux, lux_minima);
  if (st_read != HAL_OK) {
    return ERRO;
  }
  return (lux < lux_minima) ? ESCURO : CLARO;
}

static EstadoID claro(Contexto *ctx) {
  HAL_GPIO_WritePin(LED_EXT_GPIO_Port, LED_EXT_Pin, GPIO_PIN_RESET);
  return AGUARDANDO;
}

static EstadoID escuro(Contexto *ctx) {
  HAL_GPIO_WritePin(LED_EXT_GPIO_Port, LED_EXT_Pin, GPIO_PIN_SET);
  return AGUARDANDO;
}

static EstadoID manual(Contexto *ctx) {
  /* LED fixo aceso por 10s, sensor ignorado */
  HAL_GPIO_WritePin(LED_EXT_GPIO_Port, LED_EXT_Pin, GPIO_PIN_SET);
  if (g_buttonFlag) {
    g_buttonFlag = 0;
    printf("LED desligado - botão\r\n");
    HAL_GPIO_WritePin(LED_EXT_GPIO_Port, LED_EXT_Pin, GPIO_PIN_RESET);
    return AGUARDANDO;
  }
  if (g_timerFlag) {
    g_timerFlag = 0;
    ctx->manualCountdown--;
    if (ctx->manualCountdown == 0) {
      printf("LED desligado - 10s\r\n");
      HAL_GPIO_WritePin(LED_EXT_GPIO_Port, LED_EXT_Pin, GPIO_PIN_RESET);
      return AGUARDANDO;
    }
    return MANUAL;
  }
  __WFI();
  return MANUAL;
}

static EstadoID erro(Contexto *ctx) {
  printf("Erro na leitura do sensor\r\n");
  HAL_GPIO_WritePin(LED_EXT_GPIO_Port, LED_EXT_Pin, GPIO_PIN_RESET);
  return AGUARDANDO;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  printf("Boot OK\r\n");
  HAL_StatusTypeDef st_power = BH1750_PowerOn(&hi2c1);
  HAL_StatusTypeDef st_mode = BH1750_SetMode(&hi2c1, BH1750_MODE_CONTINUOUS_H_RES);
  HAL_Delay(180); // primeira conversão

  HAL_TIM_Base_Start_IT(&htim6);

  Contexto ctx = {0};
  EstadoID estadoAtual = INIT;

  EstadoFunc *tabela_estados[] = {
      [INIT] = init,     [AGUARDANDO] = aguardando,
      [MEDIR] = medir,   [CLARO] = claro,
      [ESCURO] = escuro, [MANUAL] = manual,
      [ERRO] = erro,
  };
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE BEGIN WHILE */
    estadoAtual = tabela_estados[estadoAtual](&ctx);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM6) {
    g_timerFlag = 1;
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == B1_Pin) {
    g_buttonFlag = 1;
  }
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
