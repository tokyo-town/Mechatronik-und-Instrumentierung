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
/* USER CODE BEGIN PFP */
#define PWM_MAX 800
static uint16_t duty = 2000; // Startleistung
void USART_Config();
void GPIO_Init_All();
void TIM1_Init();
void BLDC_Step(uint8_t step);
void delay(volatile uint32_t t);
int t = 5000;

int priority_grouping = 6;//2 Gruppen- und 8 Subprioritäten für NVIC
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock to 16 MHz */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
	USART_Config();
	GPIO_Init_All();
    TIM1_Init();

    uint8_t step = 0;

    while(1)
    {
        BLDC_Step(step);

        step++;
        if(step >= 6) step = 0;

        delay(t); // Geschwindigkeit einstellen!
    }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


	 // --------------------------USART ---------------------------------
void USART_Config(){

	//GPIOA mit AHB1 Bus verbinden
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	
	//TX und RX an Pins A2 und A3 als AF7 
  LL_GPIO_InitTypeDef     pinA23;
	
	pinA23.Mode = LL_GPIO_MODE_ALTERNATE;
	pinA23.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	pinA23.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3;
	pinA23.Pull = LL_GPIO_PULL_NO;
	pinA23.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	pinA23.Alternate = LL_GPIO_AF_7;//verbindet A2 und A3 mit USART2
	
	LL_GPIO_Init(GPIOA,&pinA23);

	//USART2 mit AHB1 Bus verbinden
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  LL_USART_InitTypeDef    usart2;
	LL_USART_StructInit(&usart2);
	
	usart2.BaudRate = 9600;
	usart2.DataWidth = LL_USART_DATAWIDTH_8B;
	usart2.StopBits = LL_USART_STOPBITS_1;
	usart2.Parity = LL_USART_PARITY_NONE;
	usart2.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	usart2.TransferDirection = LL_USART_DIRECTION_TX_RX;//bi-direktional
    
	LL_USART_Init(USART2,&usart2);
    
	// USART starten
	LL_USART_Enable(USART2);
  
	//Interrupt wenn receive register not empty
	LL_USART_EnableIT_RXNE(USART2);
	
	int encoded_priority = NVIC_EncodePriority(priority_grouping,1,1);//Gruppe=1, Sub=1
	NVIC_SetPriority(USART2_IRQn, encoded_priority);
  NVIC_EnableIRQ(USART2_IRQn);
	//usart fertig, unter der Endlosschleife Interrupt-Handler
}

// ---------- INIT GPIO ----------
void GPIO_Init_All(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

    // PWM Pins PA8, PA9, PA10
    LL_GPIO_InitTypeDef gpio = {0};
    gpio.Pin = LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_11;
    gpio.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio.Pull = LL_GPIO_PULL_NO;
    gpio.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(GPIOA, &gpio);

    // IN Pins PC0, PC1, PC2
    gpio.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2;
    gpio.Mode = LL_GPIO_MODE_OUTPUT;
    gpio.Alternate = 0;
    LL_GPIO_Init(GPIOC, &gpio);
}

// ---------- INIT TIM1 ----------
void TIM1_Init(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

    LL_TIM_SetPrescaler(TIM1, 0);
    LL_TIM_SetAutoReload(TIM1, PWM_MAX - 1);
    LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);

    // PWM Mode
    LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH4, LL_TIM_OCMODE_PWM1);

    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH2);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH4);

    // Start mit 0
    LL_TIM_OC_SetCompareCH1(TIM1, 0);
    LL_TIM_OC_SetCompareCH2(TIM1, 0);
    LL_TIM_OC_SetCompareCH4(TIM1, 0);

    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH4);

    LL_TIM_EnableAllOutputs(TIM1);
		LL_TIM_EnableAutomaticOutput(TIM1);
		
		// optional sicherheitshalber
		LL_TIM_DisableBRK(TIM1);
		
    LL_TIM_EnableCounter(TIM1);
}

// ---------- 6 STEP KOMMUTIERUNG ----------
void BLDC_Step(uint8_t step)
{
    switch(step)
    {
        case 0: // U PWM, V LOW, W OFF
            LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_0);
            LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_1 | LL_GPIO_PIN_2);

            LL_TIM_OC_SetCompareCH1(TIM1, duty);
            LL_TIM_OC_SetCompareCH2(TIM1, PWM_MAX);
            LL_TIM_OC_SetCompareCH4(TIM1, 0);
            break;

        case 1: // U PWM, W LOW, V OFF
            LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_0);
            LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_1 | LL_GPIO_PIN_2);

            LL_TIM_OC_SetCompareCH1(TIM1, duty);
            LL_TIM_OC_SetCompareCH2(TIM1, 0);
            LL_TIM_OC_SetCompareCH4(TIM1, PWM_MAX);
            break;

        case 2: // V PWM, W LOW, U OFF
            LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_1);
            LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0 | LL_GPIO_PIN_2);

            LL_TIM_OC_SetCompareCH1(TIM1, 0);
            LL_TIM_OC_SetCompareCH2(TIM1, duty);
            LL_TIM_OC_SetCompareCH4(TIM1, PWM_MAX);
            break;

        case 3: // V PWM, U LOW, W OFF
            LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_1);
            LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0 | LL_GPIO_PIN_2);

            LL_TIM_OC_SetCompareCH1(TIM1, PWM_MAX);
            LL_TIM_OC_SetCompareCH2(TIM1, duty);
            LL_TIM_OC_SetCompareCH4(TIM1, 0);
            break;

        case 4: // W PWM, U LOW, V OFF
            LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_2);
            LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0 | LL_GPIO_PIN_1);

            LL_TIM_OC_SetCompareCH1(TIM1, PWM_MAX);
            LL_TIM_OC_SetCompareCH2(TIM1, 0);
            LL_TIM_OC_SetCompareCH4(TIM1, duty);
            break;

        case 5: // W PWM, V LOW, U OFF
            LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_2);
            LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0 | LL_GPIO_PIN_1);

            LL_TIM_OC_SetCompareCH1(TIM1, 0);
            LL_TIM_OC_SetCompareCH2(TIM1, PWM_MAX);
            LL_TIM_OC_SetCompareCH4(TIM1, duty);
            break;
    }
}

// ---------- SIMPLE DELAY ----------
void delay(volatile uint32_t t)
{
    while(t--) __NOP();
}


int i = 0;
char cmd[4] = "";
void USART2_IRQHandler(){//wird nur aufgerufen wenn RXNE
	int recvd = LL_USART_ReceiveData8(USART2);//empfangenes Datenwort wird ausgelesen -> pending-bit wird hardware-seitig resetted
	
	if(recvd >= '0' && recvd <= '9'){
		cmd[i] = recvd;
		i++;
	}
	if(i>=4){
		i = 0;
		int zahl = 1000*(cmd[0]-'0') + 100*(cmd[1]-'0') + 10*(cmd[2]-'0') + (cmd[3]-'0');
		if(zahl == 0){
			
		}else{
			t = zahl;
		}
	}
}





/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
  LL_PWR_DisableOverDriveMode();
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(16000000);
  LL_SetSystemCoreClock(16000000);
  LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
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
