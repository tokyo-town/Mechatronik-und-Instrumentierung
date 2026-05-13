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
typedef struct Coord{
	int x;
	int y;
} Coord;

void USART_Config();
void gpio_Config();
void tim2_Config(void);
void befehl(char[4]);
void setFrequency(int);
void goTo(Coord);
void zeichne(Coord[100]);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t fullstep_pattern[4] = {
    LL_GPIO_PIN_0 | LL_GPIO_PIN_1,
    LL_GPIO_PIN_1 | LL_GPIO_PIN_2,
    LL_GPIO_PIN_2 | LL_GPIO_PIN_3,
    LL_GPIO_PIN_3 | LL_GPIO_PIN_0
};
uint16_t fullstep_pattern_2[4] = {
    LL_GPIO_PIN_5 | LL_GPIO_PIN_6,
    LL_GPIO_PIN_6 | LL_GPIO_PIN_8,
    LL_GPIO_PIN_8 | LL_GPIO_PIN_9,
    LL_GPIO_PIN_9 | LL_GPIO_PIN_5
};
uint16_t halfstep_pattern[4] = {
    LL_GPIO_PIN_0,
	LL_GPIO_PIN_0 | LL_GPIO_PIN_1,
	LL_GPIO_PIN_1
    LL_GPIO_PIN_1 | LL_GPIO_PIN_2,
	LL_GPIO_PIN_2,
    LL_GPIO_PIN_2 | LL_GPIO_PIN_3,
	LL_GPIO_PIN_3,
    LL_GPIO_PIN_3 | LL_GPIO_PIN_0
};
uint16_t halfstep_pattern_2[4] = {
	LL_GPIO_PIN_5,
    LL_GPIO_PIN_5 | LL_GPIO_PIN_6,
	LL_GPIO_PIN_6
    LL_GPIO_PIN_6 | LL_GPIO_PIN_8,
	LL_GPIO_PIN_8,
    LL_GPIO_PIN_8 | LL_GPIO_PIN_9,
	LL_GPIO_PIN_9,
    LL_GPIO_PIN_9 | LL_GPIO_PIN_5
};

volatile uint8_t step_index = 0;
volatile uint8_t step_index_2 = 0;
int direction_1 = 1; // 1,-1 oder 0 für festen Stopp
int direction_2 = 1;

int priority_grouping = 6;//2 Gruppen- und 8 Subprioritäten für NVIC

int nSteps = 0;
//int nSteps_2 = 0;
uint8_t drawing = 0;

Coord startCoord = {0,0};
Coord endCoord = {0,0};
int dx = 0;
int dy = 0;
int fehler = 0;

Coord rechteck[4] = { {100,0}, {100,100}, {0,100}, {0,0}};
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

  /* USER CODE BEGIN SysInit */
	USART_Config();
	gpio_Config();
  SystemClock_Config();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
	
	tim2_Config();
	//-------config fertig
	/* Enable counter(tim2 f?ngt zu z?hlen an) */
  LL_TIM_EnableCounter(TIM2);

  setFrequency(50);

  goTo({100,0});
  while(drawing){}
  goTo({100,100});
  while(drawing){}
  goTo({0,100});
  while(drawing){}
  goTo({0,0});
  while(drawing){}
 
  //zeichne(rechteck);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		__WFI();
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

void gpio_Config(){
	// 1ter Schritt: GPIO Port ?ber den Advanced Hyper Bus (AHB) aktivieren
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
		// 2ter Schritt: Pin 5 (von GPIO A) als Push-Pull Ausgang konfigurieren
	LL_GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStructure.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_8 | LL_GPIO_PIN_9;            
	GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;   // Wir ben?tigen keine Pull-Up oder Pull-Down Widerst?nde
	GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;  // Die Geschwindigkeit ist hier wirklich egal, wir drehen nur ein LED auf

	LL_GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	//----blinker
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	
	// 2ter Schritt: Pin 5 (von GPIO A) als Push-Pull Ausgang konfigurieren	
	GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStructure.Pin = LL_GPIO_PIN_5;            
	GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;   // Wir ben?tigen keine Pull-Up oder Pull-Down Widerst?nde
	GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;  // Die Geschwindigkeit ist hier wirklich egal, wir drehen nur ein LED auf

	LL_GPIO_Init(GPIOA,&GPIO_InitStructure);
}

void tim2_Config(){
	//Tim2 config
  /* Enable the timer peripheral clock */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2); 
	
	LL_TIM_InitTypeDef tim2;
	
	tim2.Prescaler = 1599;	//10kHz
	tim2.Autoreload = 199; //-> 50Hz
	
	LL_TIM_Init(TIM2, &tim2);
	
	LL_TIM_EnableARRPreload(TIM2);//ARR lässt sich nun direkt überschreiben
	
	/* Enable the update interrupt */
  LL_TIM_EnableIT_UPDATE(TIM2);
	
	/* Configure the NVIC to handle TIM2 update interrupt */
  NVIC_SetPriority(TIM2_IRQn, 0);
  NVIC_EnableIRQ(TIM2_IRQn);	
}

int i = 0;
char cmd[4] = "";

void USART2_IRQHandler(){//wird nur aufgerufen wenn RXNE
	int recvd = LL_USART_ReceiveData8(USART2);//empfangenes Datenwort wird ausgelesen -> pending-bit wird hardware-seitig resetted
	if(i==0 && (recvd == 's' || recvd == 'f' || recvd == 'n')){
		cmd[i] = recvd;
		i++;
	}
	else if((i != 0) && (recvd >= '0' && recvd <= '9')){
		cmd[i] = recvd;
		i++;
	}
	else{
		i = 0;
		while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
		LL_USART_TransmitData8(USART2,'F');
	}
	if(i==4){
		i = 0;
		
		befehl(cmd);
/*		int zahl = 100*(cmd[1]-'0') + 10*(cmd[2]-'0') + (cmd[3]-'0');
		if(zahl == 0){
			LL_TIM_DisableCounter(TIM2);
		}else{
			LL_TIM_SetCounter(TIM2,0);
			LL_TIM_SetAutoReload(TIM2,zahl);
			LL_TIM_EnableCounter(TIM2);
		}*/
		
		while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
		LL_USART_TransmitData8(USART2,cmd[0]);
		while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
		LL_USART_TransmitData8(USART2,cmd[1]);
		while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
		LL_USART_TransmitData8(USART2,cmd[2]);
		while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
		LL_USART_TransmitData8(USART2,cmd[3]);
	}
	
}
void TIM2_IRQHandler(){
	if(nSteps >= 0){	//ist bei unendlichen Schritten negativ
		nSteps--;
		if(nSteps == -1){	//soll keinen Schritt mehr machen und Muster nicht verändern
			LL_TIM_ClearFlag_UPDATE(TIM2);
			LL_TIM_DisableCounter(TIM2);
			return;
//			while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
//			LL_USART_TransmitData8(USART2,'T');
		}
	}
	
	/* alle Stepper Pins löschen */
  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3);
  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_8 | LL_GPIO_PIN_9);
  
  /* neues Muster setzen */
  LL_GPIO_SetOutputPin(GPIOC, fullstep_pattern[step_index]);
	
	LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_5);

  step_index += direction_1;
  if(step_index >= 4){
		step_index = 0;
	}
	else if(step_index < 0){
		step_index = 3;
	}
	
	// SEHR WICHTIG!!!
	LL_TIM_ClearFlag_UPDATE(TIM2);
}

void befehl(char input[4]){
	
	int zahl = 100*(input[1]-'0') + 10*(input[2]-'0') + (input[3]-'0');
	
	if(input[0] == 's'){
		
	}
	else if(input[0] == 'f'){
		if(zahl == 0){
			LL_TIM_DisableCounter(TIM2);
		}else{
			LL_TIM_SetCounter(TIM2,0);
			LL_TIM_SetAutoReload(TIM2,zahl);
			LL_TIM_EnableCounter(TIM2);
		}
	}
	else if(input[0] == 'n'){
		nSteps = zahl;		
		LL_TIM_SetCounter(TIM2,0);
		LL_TIM_EnableCounter(TIM2);
	}
	else{
		while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
		LL_USART_TransmitData8(USART2,'B');
	}
}

void setFrequency(int zahl){
	if(zahl <= 0){
		LL_TIM_DisableCounter(TIM2);
	}
	else{
		if(zahl > 10000){
			zahl = 10000;
		}
		LL_TIM_SetCounter(TIM2,0);
		LL_TIM_SetAutoReload(TIM2, (int) (10000/zahl) -1); // T = 10kHz/f
		LL_TIM_EnableCounter(TIM2);
	}
}

void goTo(Coord input){ // Bresenham Algorythmus
	endCoord = input;
	dx = input.x - startCoord.x;
	dy = input.y - startCoord.y;
	if(dx < 0){
		dx = -dx;
		direction_1 = -1;
	}
	if(dy < 0){
		dy = -dy;
		direction_2 = -1;
	}
	
	drawing = 1;
}

void zeichne(Coord input[100]){


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