/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : PWM Motorsteuerung mit Richtungssteuerung
  * @todo           : Homing Routine, Busy-wait (drawing) entfernen, Move-Queue, amplitude-Frequenz, Beschleunigung, tim6-Handler entlasten
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

//typedef struct Coord{
//	int x;
//	int y;
//} Coord;

void gpio_Config(void);
void USART_Config(void);
void tim3und4_Config(void);
void tim6_Config(void);

void setPWMs(uint16_t step, uint8_t motor);
void setFrequency(int zahl);
void setMicrosteps(uint16_t ms);
static int parseInt(const char *s, int *i);
static float parseFloat(const char *s, int *i);
void befehl(char input[32]);
void stift(uint8_t); // 0= Stift in der Luft, 1= Stift auf Papier
void move(Coord);
void homing();
void report(char symbol);
void reportString(char *string);

/* Private variables ---------------------------------------------------------*/
uint32_t nSteps = 0; //nur positive Zahlen!
uint16_t microsteps = 4; // bei 8 microsteps wird der erste und letzte "verschluckt"
const uint16_t maxSteps = 512;
const char EOL_CHAR = '\n';
const uint16_t umProFullstep = 589; // ein Fullstep hat ~0,655mm
uint32_t maxX = 420000;	// 42cm in x in um
uint32_t maxY = 300000; // 30cm in y in um

const uint16_t sinusTabelle[256] = { // nur positive Sinushalbwelle, Werte von 0 bis 1000
0,12,25,37,49,61,74,86,98,110,122,135,147,159,171,183,
195,207,219,231,243,255,267,279,290,302,314,325,337,348,360,371,
383,394,405,416,428,439,450,461,471,482,493,504,514,525,535,545,
556,566,576,586,596,606,615,625,634,644,653,662,672,681,690,698,
707,716,724,733,741,749,757,765,773,781,788,796,803,810,818,825,
831,838,845,851,858,864,870,876,882,888,893,899,904,909,914,919,
924,929,933,937,942,946,950,953,957,960,964,967,970,973,976,978,
981,983,985,987,989,991,992,994,995,996,997,998,999,999,1000,1000,
1000,1000,1000,999,999,998,997,996,995,994,992,991,989,987,985,983,
981,978,976,973,970,967,964,960,957,953,950,946,942,937,933,929,
924,919,914,909,904,899,893,888,882,876,870,864,858,851,845,838,
831,825,818,810,803,796,788,781,773,765,757,749,741,733,724,716,
707,698,690,681,672,662,653,644,634,625,615,606,596,586,576,566,
556,545,535,525,514,504,493,482,471,461,450,439,428,416,405,394,
383,371,360,348,337,325,314,302,290,279,267,255,243,231,219,207,
195,183,171,159,147,135,122,110,98,86,74,61,49,37,25,12
};
uint8_t amplitude = 30; // Amplitude f?r PWM-Signal von 0 bis 100, werden im TimerHandler gesetzt!!!!!!
uint8_t runAmplitude  = 40;
uint8_t holdAmplitude = 10;

volatile int16_t step_index_1 = 0;
volatile int16_t step_index_2 = 0;
int direction_1 = 1; // 1,-1 oder 0 f?r festen Stopp
int direction_2 = 1;

volatile uint8_t drawing = 0;

volatile Coord startCoord = {0,0};
volatile Coord endCoord = {0,0};
volatile int d1 = 0;
volatile int d2 = 0;
volatile int fehler = 0;
//gleiche Berechnung f?r n?chste Befehle im Voraus abspeichern?

int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  NVIC_SetPriority(SysTick_IRQn,
  NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15,0));

  /* System Clock */
  SystemClock_Config();

  
  gpio_Config();
  USART_Config();
  tim3und4_Config(); // beide PWMs
  tim6_Config();

  setMicrosteps(16);
	//nSteps = 48*microsteps*5; // 5 Umdrehungen
	setFrequency(100*microsteps); // damit man auf 200Hz pro Fullstep kommt
  // PWM und Steps trennen!!! PWM bei ~20kHz und steps bei so 1-5kHz laut Chat
  LL_TIM_EnableCounter(TIM4); // start PWM
  LL_TIM_EnableCounter(TIM3); // start PWM
  //LL_TIM_EnableCounter(TIM6); // start steps

	report('K');
	//reportString("Start");
	//homing();
	
//	draw_pattern(SQUARE);	
	//draw_pattern(STAR);
	
//	drawLetter('A', (Coord){0,0});
//	drawText("Mechatronik", (Coord){0,0});

	
  while (1)
  {
      __WFI();
  }
}


/* GPIO CONFIG ***************************************************************/
void gpio_Config()
{
    LL_GPIO_InitTypeDef GPIO_InitStructure;

    /* GPIOA Clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    /****************************************************************
     * PB6 = TIM4_CH1 (PWM)
     * PB7 = TIM4_CH2 (PWM)
     * PB8 = TIM4_CH3 (PWM)
     * PB9 = TIM4_CH4 (PWM)
     ****************************************************************/

    GPIO_InitStructure.Pin =
        LL_GPIO_PIN_6 |
        LL_GPIO_PIN_7 |
        LL_GPIO_PIN_8 |
        LL_GPIO_PIN_9;

    GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStructure.Alternate = LL_GPIO_AF_2;  // abh?ngig von Pin-Wahl!

    LL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* GPIOC Clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

    /****************************************************************
     * PC6 = TIM3_CH1 (PWM)
     * PC7 = TIM3_CH2 (PWM)
     * PC8 = TIM3_CH3 (PWM)
     * PC9 = TIM3_CH4 (PWM)
     ****************************************************************/

    GPIO_InitStructure.Pin =
        LL_GPIO_PIN_6 |
        LL_GPIO_PIN_7 |
        LL_GPIO_PIN_8 |
        LL_GPIO_PIN_9;

    GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStructure.Alternate = LL_GPIO_AF_2;

    LL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	  //----Elektromagnet zum Stiftanheben
  
	  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
	  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStructure.Pin = LL_GPIO_PIN_10;
	  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;   // Wir ben?tigen keine Pull-Up oder Pull-Down Widerst?nde
	  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;

	  LL_GPIO_Init(GPIOC,&GPIO_InitStructure);

    //----USART2

	  //GPIOA mit AHB1 Bus verbinden
	  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
	  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStructure.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3;
	  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
	  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	  GPIO_InitStructure.Alternate = LL_GPIO_AF_7;//verbindet A2 und A3 mit USART2
	  
	  LL_GPIO_Init(GPIOA,&GPIO_InitStructure);
		
		
		
		// Lichtschranken auf Pins A0 und A1
	  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
	  GPIO_InitStructure.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1;
	  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
	  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	  GPIO_InitStructure.Alternate = 0;
	  
	  LL_GPIO_Init(GPIOA,&GPIO_InitStructure);
		
		
		LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);
		LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE1);
		
		LL_EXTI_InitTypeDef EXTI_InitStruct;
		EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0 | LL_EXTI_LINE_1;
		EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
		EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
		
		EXTI_InitStruct.LineCommand = ENABLE;
		
		LL_EXTI_Init(&EXTI_InitStruct);
		
		NVIC_SetPriority(EXTI0_IRQn, 0);
		NVIC_SetPriority(EXTI1_IRQn, 0);
		NVIC_EnableIRQ(EXTI0_IRQn);
		NVIC_EnableIRQ(EXTI1_IRQn);
}

	 // --------------------------USART ---------------------------------
void USART_Config(){
	//USART2 mit AHB1 Bus verbinden
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  LL_USART_InitTypeDef    usart2;
	LL_USART_StructInit(&usart2);
	
	usart2.BaudRate = 115200;
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
	
	//int encoded_priority = NVIC_EncodePriority(priority_grouping,1,1);//Gruppe=1, Sub=1
	NVIC_SetPriority(USART2_IRQn, 2);//encoded_priority);
  NVIC_EnableIRQ(USART2_IRQn);
}

/* TIM4 PWM CONFIG ***********************************************************/
void tim3und4_Config()
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

    LL_TIM_InitTypeDef TIM_InitStruct;

    /****************************************************************
     * Timer Clock:
     * 16MHz / 1 = 16MHz
     *
     * PWM:
     * 16MHz / 800 = 20kHz
     ****************************************************************/

    TIM_InitStruct.Prescaler = 0;
    TIM_InitStruct.Autoreload = 799; // viele pwm stufen sind gut
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;

    LL_TIM_Init(TIM4, &TIM_InitStruct);
    LL_TIM_Init(TIM3, &TIM_InitStruct);

    /****************************************************************
     * PWM Channel Config
     ****************************************************************/

    LL_TIM_OC_InitTypeDef PWM_InitStruct;

    PWM_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    PWM_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
    PWM_InitStruct.CompareValue = 0;
    PWM_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;

    LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH1, &PWM_InitStruct);
    LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH2, &PWM_InitStruct);
    LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH3, &PWM_InitStruct);
    LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH4, &PWM_InitStruct);

    LL_TIM_OC_EnablePreload(TIM4, LL_TIM_CHANNEL_CH1);
    LL_TIM_OC_EnablePreload(TIM4, LL_TIM_CHANNEL_CH2);
    LL_TIM_OC_EnablePreload(TIM4, LL_TIM_CHANNEL_CH3);
    LL_TIM_OC_EnablePreload(TIM4, LL_TIM_CHANNEL_CH4);

    LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH1, &PWM_InitStruct);
    LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH2, &PWM_InitStruct);
    LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH3, &PWM_InitStruct);
    LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH4, &PWM_InitStruct);

    LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH1);
    LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH2);
    LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH3);
    LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH4);

    /****************************************************************
     * Kan?le aktivieren
     ****************************************************************/

    LL_TIM_CC_EnableChannel(TIM4,
          LL_TIM_CHANNEL_CH1 |
          LL_TIM_CHANNEL_CH2 |
          LL_TIM_CHANNEL_CH3 |
          LL_TIM_CHANNEL_CH4);

    LL_TIM_CC_EnableChannel(TIM3,
          LL_TIM_CHANNEL_CH1 |
          LL_TIM_CHANNEL_CH2 |
          LL_TIM_CHANNEL_CH3 |
          LL_TIM_CHANNEL_CH4);

    LL_TIM_EnableARRPreload(TIM4);//ARR l?sst sich nun direkt ?berschreiben
    LL_TIM_EnableARRPreload(TIM3);
}

void tim6_Config(){
	//Tim6 config
  // Enable the timer peripheral clock
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);
	
	LL_TIM_InitTypeDef tim6;
	
    /****************************************************************
     * Timer Clock:
     * 16MHz / 16 = 1MHz
     *
     * PWM:
     * 1MHz / 1000 = 1kHz
     ****************************************************************/
	tim6.Prescaler = 15;	//1MHz
	tim6.Autoreload = 1249; //624-> 1.6kHz, 1249->800Hz
	
	LL_TIM_Init(TIM6, &tim6);
	
	LL_TIM_EnableARRPreload(TIM6);//ARR l?sst sich nun direkt ?berschreiben
	
  LL_TIM_EnableIT_UPDATE(TIM6);
	
 // int encoded_priority = NVIC_EncodePriority(priority_grouping,1,3);//Gruppe=1, Sub=1
//	NVIC_SetPriority(TIM6_DAC_IRQn, encoded_priority);
	NVIC_SetPriority(TIM6_DAC_IRQn, 1);
	NVIC_EnableIRQ(TIM6_DAC_IRQn);	
}

void TIM6_DAC_IRQHandler(){
	if(nSteps > 0){
	  amplitude = runAmplitude;
	  setPWMs(step_index_1,1);
	  setPWMs(step_index_2,2);
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

    if(step_index_1 >= 4*microsteps){
	  	step_index_1 = 0;
	  }
	  else if(step_index_1 < 0){
	  	step_index_1 = 4*microsteps -1;
	  }
	  if(step_index_2 >= 4*microsteps){
	  	step_index_2 = 0;
	  }
	  else if(step_index_2 < 0){
	  	step_index_2 = 4*microsteps -1;
	  }
		nSteps--;
	}
	else{
		amplitude = holdAmplitude;
	  setPWMs(step_index_1,1);
	  setPWMs(step_index_2,2);
		LL_TIM_DisableCounter(TIM6);
		LL_TIM_SetCounter(TIM6, 0);
    startCoord = endCoord;
		drawing = 0;
		nSteps = 0;
		
		report('K');
	}

	// SEHR WICHTIG!!!
	LL_TIM_ClearFlag_UPDATE(TIM6);
}

void EXTI0_IRQHandler() {
	nSteps = 0;
  startCoord.x = 0;
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);	
}

void EXTI1_IRQHandler() {
	nSteps = 0;
  startCoord.y = 0;
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);	
}

void report(char symbol) {
	while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
	LL_USART_TransmitData8(USART2,symbol);
}

void reportString(char *string) {
	while (*string) {
		report(*string++);
	}
}

void USART2_IRQHandler(){ // parsed bis \n oder bis 32 chars
	static char cmd[32];
	static uint8_t i = 0;
	
	if(LL_USART_IsActiveFlag_RXNE(USART2)){
    int recvd = LL_USART_ReceiveData8(USART2);//empfangenes Datenwort wird ausgelesen -> pending-bit wird hardware-seitig resetted
	  
    if(recvd == EOL_CHAR){
      cmd[i] = '\0'; // String terminieren
      i = 0;
      befehl(cmd);
    }
    else if(i < 32){
      cmd[i++] = recvd;
    }
    else{
      i=0;
      cmd[0] = '\0';
      while(!LL_USART_IsActiveFlag_TXE(USART2)){};//warten bis Transmit register wieder frei ist
	    LL_USART_TransmitData8(USART2,'F');
    }
    
  }
}

void setPWMs(uint16_t step, uint8_t motor){
    uint16_t faktor = maxSteps/(4*microsteps); // ein Fullstep = 90? und dieser wird in Microsteps aufgeteilt
	
    uint16_t a = sinusTabelle[(faktor*step)% (maxSteps/2)]; // statt % lieber &0xFF ?
    uint16_t b = sinusTabelle[(faktor*step + maxSteps/4) % (maxSteps/2)]; // 90?-Verschiebung -> Kosinus

    TIM_TypeDef *tim;
    if(motor == 1){tim = TIM4;}
    else if(motor == 2){tim = TIM3;}

    uint32_t arr = LL_TIM_GetAutoReload(tim);

    switch(faktor * step / (maxSteps/4)){
    case 0: // erster Quadrant
        LL_TIM_OC_SetCompareCH1(tim, (arr*a*amplitude) / (1000*100));
        LL_TIM_OC_SetCompareCH2(tim, (arr*b*amplitude) / (1000*100));
        LL_TIM_OC_SetCompareCH3(tim, 0);
        LL_TIM_OC_SetCompareCH4(tim, 0);
        break;

    case 1: // zweiter Quadrant
    	LL_TIM_OC_SetCompareCH1(tim, (arr*a*amplitude) / (1000*100));
   	  LL_TIM_OC_SetCompareCH2(tim, 0);
    	LL_TIM_OC_SetCompareCH3(tim, 0);
    	LL_TIM_OC_SetCompareCH4(tim, (arr*b*amplitude) / (1000*100));
    	break;

    case 2: // dritter Quadrant
    	LL_TIM_OC_SetCompareCH1(tim, 0);
    	LL_TIM_OC_SetCompareCH2(tim, 0);
    	LL_TIM_OC_SetCompareCH3(tim, (arr*a*amplitude) / (1000*100));
    	LL_TIM_OC_SetCompareCH4(tim, (arr*b*amplitude) / (1000*100));
    	break;

    case 3: // vierter Quadrant
    	LL_TIM_OC_SetCompareCH1(tim, 0);
    	LL_TIM_OC_SetCompareCH2(tim, (arr*b*amplitude) / (1000*100));
    	LL_TIM_OC_SetCompareCH3(tim, (arr*a*amplitude) / (1000*100));
    	LL_TIM_OC_SetCompareCH4(tim, 0);
    	break;
    }
}

void setFrequency(int zahl){ // maximale Frequenz bei 500kHz, stepper schaffen eh nicht mehr als 500Hz/fullstep
	if(zahl <= 0){
		LL_TIM_DisableCounter(TIM6);
	}
	else{
		if(zahl > 500000){
			zahl = 500000;
		}else if(zahl < 16){
			zahl = 16;
		}
		LL_TIM_SetCounter(TIM6,0);
		LL_TIM_SetAutoReload(TIM6, (1000000/zahl) -1); // T = 1MHz/f
		LL_TIM_EnableCounter(TIM6);
	}
}

void setMicrosteps(uint16_t ms){
    microsteps = ms;
}

// parsed eine Zahl aus einem String s, beginnend an Index i. i wird auf das erste Zeichen nach der Zahl gesetzt.
static int parseInt(const char *s, int *i)
{
    int sign = 1;
    int result = 0;

    // optionales Vorzeichen
    if(s[*i] == '-')
    {
        sign = -1;
        (*i)++;
    }

    // Ziffern lesen
    while(s[*i] >= '0' && s[*i] <= '9')
    {
        result = result * 10 + (s[*i] - '0');
        (*i)++;
    }

    return sign * result;
}

static float parseFloat(const char *s, int *i)
{
    float sign = 1.0f;
    float result = 0.0f;

    if(s[*i] == '-'){
        sign = -1.0f;
        (*i)++;
    }

    // Ganzzahlteil
    while(s[*i] >= '0' && s[*i] <= '9'){
        result = result * 10.0f + (float)(s[*i] - '0');
        (*i)++;
    }

    // Nachkommastellen
    if(s[*i] == '.'){
        (*i)++;

        float factor = 0.1f;

        while(s[*i] >= '0' && s[*i] <= '9'){
            result += (float)(s[*i] - '0') * factor;
            factor *= 0.1f;
            (*i)++;
        }
    }

    return sign * result;
}

void befehl(char input[32]){	
	if(input[0] == 'M'){
		if(input[1] == '3'){
      stift(1);
			report('K');
      return;
    }else if(input[1] == '5'){
      stift(0);
			report('K');
      return;
    }
	}
	else if(input[0] == 'G' || input[0] == 'g'){
    if(input[1] == '2' && input[2] == '8'){
      homing();
      return;
	  }
    else if(input[1] == '0'){
      int i = 2;
      while(input[i] == ' ') i++;
      if(input[i] == 'F' || input[i] == 'f'){
				i++;
				int f = parseInt(input, &i);
        setFrequency(f);
				report('K');
				
				//report('f');
				//report(f);
        return;
      }
      else if(input[i] == 'X' || input[i] == 'x'){
        i++;
        int x = (int) (parseFloat(input, &i) * 1000.0f); // mm * 1000 = um
        while(input[i] == ' ') i++;
        if(input[i] == 'Y' || input[i] == 'y'){
          i++;
          int y = (int) (parseFloat(input, &i) * 1000.0f);
          move((Coord){x,y});
          return;
        }
      }
    }
  } 
  

	report('F');
	reportString(input);
}

void stift(uint8_t in){
	if(in == 0){ // Stift in der Luft
		LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_10);		
	}else if(in == 1){ // Stift auf Papier
		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_10);		
	}
}

void move(Coord input){ // Bresenham Algorythmus, Koordinaten in um
	if(input.x > maxX){input.x = maxX;}else if(input.x < 0){input.x = 0;}
	if(input.y > maxY){input.y = maxY;}else if(input.y < 0){input.y = 0;}
	endCoord = (Coord) {input.x /umProFullstep, input.y /umProFullstep}; // Umrechnung um zu steps	
	
	d1 = (endCoord.x - startCoord.x) + (endCoord.y - startCoord.y); // = dx + dy
	d2 = (endCoord.x - startCoord.x) - (endCoord.y - startCoord.y); // = dy - dy
	
	direction_1 = (d1 >= 0) ? 1 : -1;
	direction_2 = (d2 >= 0) ? 1 : -1;
	d1 = d1 * direction_1 * microsteps;
	d2 = d2 * direction_2 * microsteps;

	if(d1 >= d2){
		fehler = d1/2; // "/2", damit bei x:y=2:1 der y-Schritt in der Mitte passiert
		nSteps = d1;
	}
	else{
		fehler = d2/2; // runden nicht schlimm?
		nSteps = d2;
	}
	
	drawing = 1;
	LL_TIM_EnableCounter(TIM6);
  while(drawing){} // busy-wait, damit nicht schon der n?chste move ausgel?st wird
}

void homing(){	
	//reportString("homing");
	
	if(!LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0)){
		startCoord = (Coord) {90000,0};
		move((Coord) {0,0});
	}else{report('K');}
	if(!LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1)){
		startCoord = (Coord) {0,90000};
		move((Coord) {0,0});
	}else{report('K');}
	startCoord = (Coord) {0,0};
	
	//reportString("home");
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
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
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  }

  LL_Init1msTick(16000000);

  LL_SetSystemCoreClock(16000000);

  LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}


/**
  * @brief Error Handler
  */
void Error_Handler(void)
{
  __disable_irq();

  while (1)
  {
  }
}
