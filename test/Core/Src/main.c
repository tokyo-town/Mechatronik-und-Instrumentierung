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
#include "letters.h"
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
void setFullstepHalfstep(uint8_t); // 0=Fullstep, 1=Halfstep
void stift(uint8_t); // 0= Stift in der Luft, 1= Stift auf Papier
void move(Coord);
void zeichne(Coord[100]);

void travel(Coord co);
void draw(Coord co);

void drawLetter(char letter, Coord pos);
void letterRect(char letter, float* x, float* y, float* w, float* h);
void drawText(char* chars, Coord pos);

float LETTER_SCALE = 0.08;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
const uint16_t fullstep_pattern[4] = {
    LL_GPIO_PIN_0 | LL_GPIO_PIN_1,
    LL_GPIO_PIN_1 | LL_GPIO_PIN_2,
    LL_GPIO_PIN_2 | LL_GPIO_PIN_3,
    LL_GPIO_PIN_3 | LL_GPIO_PIN_0
};
const uint16_t fullstep_pattern_2[4] = {
    LL_GPIO_PIN_5 | LL_GPIO_PIN_6,
    LL_GPIO_PIN_6 | LL_GPIO_PIN_8,
    LL_GPIO_PIN_8 | LL_GPIO_PIN_9,
    LL_GPIO_PIN_9 | LL_GPIO_PIN_5
};
const uint16_t halfstep_pattern[8] = {
    LL_GPIO_PIN_0,
	LL_GPIO_PIN_0 | LL_GPIO_PIN_1,
	LL_GPIO_PIN_1,
    LL_GPIO_PIN_1 | LL_GPIO_PIN_2,
	LL_GPIO_PIN_2,
    LL_GPIO_PIN_2 | LL_GPIO_PIN_3,
	LL_GPIO_PIN_3,
    LL_GPIO_PIN_3 | LL_GPIO_PIN_0
};
const uint16_t halfstep_pattern_2[8] = {
	LL_GPIO_PIN_5,
    LL_GPIO_PIN_5 | LL_GPIO_PIN_6,
	LL_GPIO_PIN_6,
    LL_GPIO_PIN_6 | LL_GPIO_PIN_8,
	LL_GPIO_PIN_8,
    LL_GPIO_PIN_8 | LL_GPIO_PIN_9,
	LL_GPIO_PIN_9,
    LL_GPIO_PIN_9 | LL_GPIO_PIN_5
};

const uint16_t* pattern_1 = fullstep_pattern; // pointer, der sich ändern kann. Sein Inhalt jedoch nicht überschreibbar
const uint16_t* pattern_2 = fullstep_pattern_2;
volatile uint8_t maxSteps = 4; //mit setFullstepHalfstep veränderbar!

volatile int8_t step_index_1 = 0;
volatile int8_t step_index_2 = 0;
int direction_1 = 1; // 1,-1 oder 0 für festen Stopp
int direction_2 = 1;

int priority_grouping = 6;//2 Gruppen- und 8 Subprioritäten für NVIC

volatile int nSteps = 0;
//int nSteps_2 = 0;
volatile uint8_t drawing = 0;

volatile Coord startCoord = {0,0};
volatile Coord endCoord = {0,0};
volatile int d1 = 0;
volatile int d2 = 0;
volatile int fehler = 0;

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
  //LL_TIM_EnableCounter(TIM2);

  setFrequency(200);
	setFullstepHalfstep(1); // Halfstep
	
	stift(1);
	
	/* schräges Quadrat im normalen Quadrat*/
//	move((Coord) {100,100});
//	move((Coord) {200,0});
//	move((Coord) {100,-100});
//	move((Coord) {0,0});
//	move((Coord) {100,-100});
//	move((Coord) {200,0});
//	move((Coord) {100,100});
//	move((Coord) {0,0});
//	move((Coord) {100,0});
//	move((Coord) {100,100});
//	move((Coord) {0,100});
//	move((Coord) {0,0});
	
	
//	move((Coord) {100,10});
//	move((Coord) {0,0});
//	move((Coord) {100,10});
//	move((Coord) {0,0});
//	move((Coord) {100,10});
//	move((Coord) {0,0});
//	move((Coord) {100,10});
//	move((Coord) {0,0});
//	move((Coord) {100,10});
//	move((Coord) {0,0});
	
	/* A 
  move((Coord){100,200});
  move((Coord){200,0});
  move((Coord){150,100});
  move((Coord){100,100});
	move((Coord) {0,0});*/
	
	/* Zickzack 
	move( (Coord) {200,10});
	move( (Coord) {0,20});
	move( (Coord) {200,30});
	move( (Coord) {0,40});
	move( (Coord) {0,0}); 
	move( (Coord) {200,-10});
	move( (Coord) {0,-20});
	move( (Coord) {200,-30});
	move( (Coord) {0,-40});
	move( (Coord) {0,0});
	*/
	
	/* Stern 
	stift(0);
	move((Coord){   0, 100});
	stift(1);
	move((Coord){  30,  30});
	move((Coord){ 100,  30});
	move((Coord){  45, -10});
	move((Coord){  70,-100});
	move((Coord){   0, -40});
	move((Coord){ -70,-100});
	move((Coord){ -45, -10});
	move((Coord){-100,  30});
	move((Coord){ -30,  30});
	move((Coord){   0, 100});
	*/
	
	//drawLetter('A', (Coord) {0, 0});
	drawText("Mechatronik", (Coord) {0, 0});
	
	stift(0);
 
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
	
	//----Elektromagnet zum Stiftanheben
	
	GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStructure.Pin = LL_GPIO_PIN_10;
	GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;   // Wir ben?tigen keine Pull-Up oder Pull-Down Widerst?nde
	GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;

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
	// SEHR WICHTIG!!!
	LL_TIM_ClearFlag_UPDATE(TIM2);

	if(nSteps >= 0){	//ist bei unendlichen Schritten negativ
		nSteps--;
		if(nSteps == -1){	//soll keinen Schritt mehr machen und Muster nicht verändern
			LL_TIM_ClearFlag_UPDATE(TIM2);
			LL_TIM_DisableCounter(TIM2);
			startCoord = endCoord;
			drawing = 0;

			return;
//			while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
//			LL_USART_TransmitData8(USART2,'T');
		}
	}

	LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_5);
	
	/* alle Stepper Pins löschen */
  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3 |
																LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_8 | LL_GPIO_PIN_9);
  
  /* neues Muster setzen */
  LL_GPIO_SetOutputPin(GPIOC, pattern_1[step_index_1] | pattern_2[step_index_2]);
	
  // beide Motoren in gleiche Richtung -> x-Achse, beide Motoren entgegengesetzt -> y-Achse
  // Motor1 still und Motor2++ -> Diagonale y=x, Motor1++ und Motor2 still -> Diagonale y=-x
  if(d1 >= d2){
		//x-Schritt, da schnelle Richtung
		step_index_1 += direction_1;
		fehler -= d2;
		if(fehler <= 0){
			step_index_2 += direction_2;
			fehler += d1;
		} 
  }
  else{ // d1 < d2
		step_index_2 += direction_2;
		fehler -= d1;
		if(fehler <= 0){
			step_index_1 += direction_1;
			fehler += d2;
		} 
  }

    if(step_index_1 >= maxSteps){
		step_index_1 = 0;
	}
	else if(step_index_1 < 0){
		step_index_1 = maxSteps -1;
	}
	if(step_index_2 >= maxSteps){
		step_index_2 = 0;
	}
	else if(step_index_2 < 0){
		step_index_2 = maxSteps -1;
	}
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

void setFrequency(int zahl){ // maximale Frequenz bei 5kHz, stepper schaffen eh nicht mehr als 500Hz
	if(zahl <= 0){
		LL_TIM_DisableCounter(TIM2);
	}
	else{
		if(zahl > 5000){
			zahl = 5000;
		}
		LL_TIM_SetCounter(TIM2,0);
		LL_TIM_SetAutoReload(TIM2, (10000/zahl) -1); // T = 10kHz/f
		LL_TIM_EnableCounter(TIM2);
	}
}

void setFullstepHalfstep(uint8_t mode){
	if(mode == 0){ //Fullstep
		maxSteps = 4;
		pattern_1 = fullstep_pattern;
		pattern_2 = fullstep_pattern_2;
	}else if(mode == 1){ // Halfstep
		maxSteps = 8;
		pattern_1 = halfstep_pattern;
		pattern_2 = halfstep_pattern_2; // wie macht man das? unbedingt noch ändern!!!!!
	}
}

void stift(uint8_t in){
	if(in == 0){ // Stift in der Luft
		LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_10);		
	}else if(in == 1){ // Stift auf Papier
		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_10);		
	}
}

void move(Coord input){ // Bresenham Algorythmus
	endCoord = input;
	d1 = (input.x - startCoord.x) + (input.y - startCoord.y); // = dx + dy
	d2 = (input.x - startCoord.x) - (input.y - startCoord.y); // = dy - dy
	
	direction_1 = (d1 >= 0) ? 1 : -1;
	direction_2 = (d2 >= 0) ? 1 : -1;
	d1 = d1 * direction_1;
	d2 = d2 * direction_2;

	if(d1 >= d2){
		fehler = d1/2; // "/2", damit bei x:y=2:1 der y-Schritt in der Mitte passiert
		nSteps = d1;
	}
	else{
		fehler = d2/2; // runden nicht schlimm?
		nSteps = d2;
	}
	
	drawing = 1;
	LL_TIM_EnableCounter(TIM2);
  while(drawing){} // busy-wait, damit nicht schon der nächste move ausgelöst wird
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

/* -------------------------------------
   Draw Letters
------------------------------------- */

void drawLetter(char letter, Coord pos) {
    int letterIndex;

    short int (*coordinates)[100][2];
    unsigned char (*endPts)[3];

    int endpt_i = 0;
    int endpt;

    int i;

    if (letter >= 'a') {
        letterIndex = letter - 'a';
				coordinates = &lowercase_letters_coordinates[letterIndex];
				endPts = &lowercase_letters_endPts[letterIndex];

    } else {
        letterIndex = letter - 'A';
        coordinates = &uppercase_letters_coordinates[letterIndex];
        endPts = &uppercase_letters_endPts[letterIndex];
    }

    endpt = (*endPts)[endpt_i];

    for (i = 0; i < 100; i++) {

        if (&coordinates[i][0] == 0 && &coordinates[i][1] == 0) {
            break;
        }

        Coord co;
        co.x = (int) ( (*coordinates)[i][0] * LETTER_SCALE + pos.x);
        co.y = (int) ( (*coordinates)[i][1] * LETTER_SCALE + pos.y);

        if (i == 0 || i - 1 == endpt) {
            travel(co);
        } else {
            draw(co);
        }

        if (i - 2 == endpt) {
            endpt_i++;
            endpt = (*endPts)[endpt_i];
        }
    }
}


void drawText(char* chars, Coord pos) {
    Coord start = pos;
    int i;

    for (i = 0; chars[i] != '\0'; i++) {
        char c = chars[i];

        if (c == '\n') {
            start.x = pos.x;
            start.y -= 100;

        } else if (c == ' ') {
            start.x += 20;

        } else {
            float x, y, w, h;

            drawLetter(c, start);
            letterRect(c, &x, &y, &w, &h);

            start.x += w;
        }
    }
}


void letterRect(
    char letter,
    float* x,
    float* y,
    float* w,
    float* h
) {
    int letterIndex;

    short int (*coordinates)[100][2];

    int i;

    int min_x;
    int min_y;
    int max_x;
    int max_y;

    if (letter >= 'a') {
        letterIndex = letter - 'a';
        coordinates = &lowercase_letters_coordinates[letterIndex];
    } else {

        letterIndex = letter - 'A';
        coordinates = &uppercase_letters_coordinates[letterIndex];
    }

    min_x = (*coordinates)[0][0];
    min_y = (*coordinates)[0][1];

    max_x = (*coordinates)[0][0];
    max_y = (*coordinates)[0][1];

    for (i = 0; i < 400; i++) {
        int px = (*coordinates)[i][0];
        int py = (*coordinates)[i][1];

        if (px == 0 && py == 0) {
            break;
        }

        if (px < min_x) min_x = px;
        if (py < min_y) min_y = py;

        if (px > max_x) max_x = px;
        if (py > max_y) max_y = py;
    }

    *x = min_x * LETTER_SCALE;
    *y = min_y * LETTER_SCALE;

    *w = (max_x - min_x) * LETTER_SCALE;
    *h = (max_y - min_y) * LETTER_SCALE;
}


void travel(Coord co) {
    stift(0);
    move(co);
}

void draw(Coord co) {
    stift(1);
    move(co);
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