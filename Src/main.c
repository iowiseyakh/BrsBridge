/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFLEN 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
  uint8_t U1InBuf[BUFLEN];
  int U1Bot = 0, U1Top = 0;
  uint8_t U2InBuf[BUFLEN];
  int U2Bot = 0, U2Top = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
}
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOB, MODEM_EN_Pin, GPIO_PIN_RESET); // Подтягиваем к земле пин
  HAL_Delay(1500); // Ждем от 1 до 12с, как того требует даташит на модем
  HAL_GPIO_WritePin(GPIOB, MODEM_EN_Pin, GPIO_PIN_SET); // Отпускаем пин
  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    /*
      Typedef enum
      {
      HAL_OK = 0x00, HAL_ERROR = 0x01, HAL_BUSY = 0x02, HAL_TIMEOUT = 0x03
      } HAL_StatusTypeDef;
    */
  uint8_t c;
  HAL_StatusTypeDef e;
  HAL_UART_StateTypeDef u;
  int s=0;
  while (1)
  {
    e = HAL_UART_Receive(&huart1, U1InBuf + U1Top, 1, 0);
    if(e == HAL_OK) // Приняли байт
    {
      if(++U1Top >= BUFLEN) // Увеличивваем вершину кольцевого буфера на 1
        U1Top = 0; // Если вышли за границу буфера, значит сбрасываем указатель
      U1InBuf[U1Top] = c; // Записываем на вершину буфера принятый байт
      //HAL_GPIO_TogglePin(GPIOB, LED_R_Pin);
    }
    s += e;
    
    // То же для второго УАРТа
    e = HAL_UART_Receive(&huart2, U2InBuf + U2Top, 1, 0);
    if(e == HAL_OK)
    {
      if(++U2Top >= BUFLEN)
        U2Top = 0;
      U2InBuf[U2Top] = c;
    }
    s += e;
    
    // Отправляем из первого буфера во второй УАРТ и наоборот
    if(U1Top != U1Bot) // Если вершина не равна основанию, значит в буфере есть данные
    {
      u = HAL_UART_GetState(&huart2); // УАРТ не занят и готов отправлять новую порцию данных
      if(u != HAL_UART_STATE_BUSY_TX)
      {
        HAL_UART_Transmit_IT(&huart2, U1InBuf + U1Bot, 1); // Отправляем байт из основания буфера 
        HAL_GPIO_TogglePin(GPIOB, LED_B_Pin); // Блымаем блымочкой
        if(++U1Bot >= BUFLEN) // Увеличивваем основание кольцевого буфера на 1
          U1Bot = 0; // Если вышли за границу буфера, значит сбрасываем указатель
      }
      s += u;
    }
    
    // То же для первого УАРТа
    if(U2Top != U2Bot)// Если вершина не равна основанию, значит в буфере есть данные
    {
      u = HAL_UART_GetState(&huart1);
      if(u != HAL_UART_STATE_BUSY_TX) // УАРТ не занят и готов отправлять новую порцию данных
      {
        HAL_UART_Transmit_IT(&huart1, U2InBuf + U2Bot, 1); // Отправляем байт из основания буфера
        HAL_GPIO_TogglePin(GPIOB, LED_R_Pin); // Блымаем блымочкой
        if(++U2Bot >= BUFLEN) // Увеличивваем основание кольцевого буфера на 1
          U2Bot = 0; // Если вышли за границу буфера, значит сбрасываем указатель
      }
      s += u;
    }
    
    if(s > 1000000)
    {
      HAL_UART_Transmit_IT(&huart1, U2InBuf + s, 1);
      s = 0;
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

