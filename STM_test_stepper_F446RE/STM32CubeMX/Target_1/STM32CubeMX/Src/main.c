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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
void set_coil_a(int);
void set_coil_b(int);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
GPIO_TypeDef* GPIO_IN1 = GPIOA;
GPIO_TypeDef* GPIO_IN2 = GPIOB;
GPIO_TypeDef* GPIO_IN3 = GPIOB;
GPIO_TypeDef* GPIO_IN4 = GPIOB;
GPIO_TypeDef* GPIO_ENA = GPIOC;
GPIO_TypeDef* GPIO_ENB = GPIOB;
uint16_t PIN_IN1 = GPIO_PIN_8;
uint16_t PIN_IN2 = GPIO_PIN_10;
uint16_t PIN_IN3 = GPIO_PIN_4;
uint16_t PIN_IN4 = GPIO_PIN_5;
uint16_t PIN_ENA = GPIO_PIN_9;
uint16_t PIN_ENB = GPIO_PIN_8;
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
  /* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(GPIO_ENA, PIN_ENA, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIO_ENB, PIN_ENB, GPIO_PIN_SET);
	
	// full step
//	int steps[4][2] = {
//		{ 1,  1},
//		{-1,  1},
//		{-1, -1},
//		{ 1, -1}
//	};
	
	// half step
	int steps[8][2] = {
		{ 1,  0},
		{ 1,  1},
		{ 0,  1},
		{-1,  1},
		{-1,  0},
		{-1, -1},
		{ 0, -1},
		{ 1, -1}
	};
	
	int step = 0;
	int reverse = 0;
	
	float step_deg = 7.5;
	float round_steps = 360 / step_deg;
	
	int accel_steps = 10;
	
	float current_rps = 0;
	float target_rps = 4;
	
	float rps_step = target_rps / accel_steps;
	float delay_ms = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		set_coil_a(steps[step][0]);
		set_coil_b(steps[step][1]);
		
		if (reverse) {
			step = (step - 1) % 4;
		}	else {
			step = (step + 1) % 4;
		}
		
		
		if (current_rps < target_rps) {
			current_rps = current_rps + rps_step;
		}
		else {
			current_rps = target_rps;
		}
		
		delay_ms = 1000 / (round_steps * current_rps);		
		HAL_Delay(delay_ms);
		
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|IN1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, IN2_Pin|IN3_Pin|IN4_Pin|ENA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ENB_GPIO_Port, ENB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA5 IN1_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_5|IN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : IN2_Pin IN3_Pin IN4_Pin ENA_Pin */
  GPIO_InitStruct.Pin = IN2_Pin|IN3_Pin|IN4_Pin|ENA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ENB_Pin */
  GPIO_InitStruct.Pin = ENB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ENB_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void set_coil_a(int direction) {

	if (direction == 1) {
		HAL_GPIO_WritePin(GPIO_IN1, PIN_IN1, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIO_IN2, PIN_IN2, GPIO_PIN_RESET);
	}
	else if (direction == -1) {
		HAL_GPIO_WritePin(GPIO_IN1, PIN_IN1, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIO_IN2, PIN_IN2, GPIO_PIN_SET);
	}
	else {
		HAL_GPIO_WritePin(GPIO_IN1, PIN_IN1, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIO_IN2, PIN_IN2, GPIO_PIN_RESET);
	}		
}

void set_coil_b(int direction) {

	if (direction == 1) {
		HAL_GPIO_WritePin(GPIO_IN3, PIN_IN3, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIO_IN4, PIN_IN4, GPIO_PIN_RESET);
	}
	else if (direction == -1) {
		HAL_GPIO_WritePin(GPIO_IN3, PIN_IN3, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIO_IN4, PIN_IN4, GPIO_PIN_SET);
	}
	else {
		HAL_GPIO_WritePin(GPIO_IN3, PIN_IN3, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIO_IN4, PIN_IN4, GPIO_PIN_RESET);
	}		
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
#ifdef USE_FULL_ASSERT
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
