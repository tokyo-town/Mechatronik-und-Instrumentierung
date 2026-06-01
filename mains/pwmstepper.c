/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : PWM Motorsteuerung mit Richtungssteuerung
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

void gpio_Config(void);
void tim2_Config(void);
void tim6_Config(void);

void setPWM(uint8_t duty);
void setDirection(uint8_t dir);
void setPWMs(uint16_t step);
void setFrequency(int zahl);
void setMicrosteps(uint16_t ms);

/* Private variables ---------------------------------------------------------*/
uint16_t step_index = 0; // 
uint32_t nSteps = 0; //nur positive Zahlen!
uint16_t microsteps = 4; // bei 8 microsteps wird der erste und letzte "verschluckt"
const uint16_t maxSteps = 512;

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
uint8_t amplitude = 30; // Amplitude für PWM-Signal von 0 bis 100, werden im TimerHandler gesetzt!!!!!!
uint8_t runAmplitude  = 20;
uint8_t holdAmplitude = 10;


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

  /* GPIO + PWM */
  gpio_Config();
  tim2_Config();
  tim6_Config();

	//setArr!!!!
    setMicrosteps(1);
		nSteps = 48*microsteps*5; // 5 Umdrehungen
		setFrequency(200*microsteps); // damit man auf 200Hz pro Fullstep kommt
    //setFrequency(400*512); // 400Hz/FS = 400*512Hz/Microstep

    // PWM und Steps trennen!!! PWM bei ~20kHz und steps bei so 1-5kHz laut Chat
    LL_TIM_EnableCounter(TIM2); // start PWM
    LL_TIM_EnableCounter(TIM6); // start steps

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
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    /* GPIOC Clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

    /****************************************************************
     * PA0 = TIM2_CH1 (PWM)
     * PA1 = TIM2_CH2 (PWM)
     ****************************************************************/

    GPIO_InitStructure.Pin =
        LL_GPIO_PIN_0 |
        LL_GPIO_PIN_1;

    GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStructure.Alternate = LL_GPIO_AF_1;

    LL_GPIO_Init(GPIOA, &GPIO_InitStructure);

    /****************************************************************
     * PC2 + PC3 = Richtung
     ****************************************************************/

    GPIO_InitStructure.Pin =
        LL_GPIO_PIN_2 |
        LL_GPIO_PIN_3;

    GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;

    LL_GPIO_Init(GPIOC, &GPIO_InitStructure);
}


/* TIM2 PWM CONFIG ***********************************************************/
void tim2_Config()
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

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

    LL_TIM_Init(TIM2, &TIM_InitStruct);

    /****************************************************************
     * PWM Channel Config
     ****************************************************************/

    LL_TIM_OC_InitTypeDef PWM_InitStruct;

    PWM_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    PWM_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
    PWM_InitStruct.CompareValue = 0;
    PWM_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;

    LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH1, &PWM_InitStruct);
    LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH2, &PWM_InitStruct);

    LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH2);

    /****************************************************************
     * Kanäle aktivieren
     ****************************************************************/

    LL_TIM_CC_EnableChannel(TIM2,
          LL_TIM_CHANNEL_CH1 |
          LL_TIM_CHANNEL_CH2);

    LL_TIM_EnableARRPreload(TIM2);//ARR lässt sich nun direkt überschreiben
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
	
	LL_TIM_EnableARRPreload(TIM6);//ARR lässt sich nun direkt überschreiben
	
  LL_TIM_EnableIT_UPDATE(TIM6);
	
 // int encoded_priority = NVIC_EncodePriority(priority_grouping,1,3);//Gruppe=1, Sub=1
//	NVIC_SetPriority(TIM6_DAC_IRQn, encoded_priority);
	NVIC_SetPriority(TIM6_DAC_IRQn, 0);
	NVIC_EnableIRQ(TIM6_DAC_IRQn);	
}

void TIM6_DAC_IRQHandler(){	
	if(nSteps > 0){
		amplitude = runAmplitude;
		setPWMs(step_index);
		step_index++;
		if(step_index >= 4*microsteps){ // eine ganze (elektrische) Umdrehung besteht aus 4 Fullsteps
			step_index = 0;
		}
		nSteps--;
	}
	else if(nSteps == 0){
		amplitude = holdAmplitude;
		setPWMs(step_index);
		LL_TIM_DisableCounter(TIM6);
		LL_TIM_SetCounter(TIM6, 0);
	}
	
	// SEHR WICHTIG!!!
	LL_TIM_ClearFlag_UPDATE(TIM6);
}

void setPWMs(uint16_t step){
    uint16_t faktor = maxSteps/(4*microsteps); // ein Fullstep = 90° und dieser wird in Microsteps aufgeteilt
	
    uint16_t a = sinusTabelle[(faktor*step)% (maxSteps/2)]; // statt % lieber &0xFF ?
    uint16_t b = sinusTabelle[(faktor*step + maxSteps/4) % (maxSteps/2)]; // 90°-Verschiebung -> Kosinus

    uint32_t arr = LL_TIM_GetAutoReload(TIM2);

    switch(faktor * step / (maxSteps/4)){
    case 0: // erster Quadrant
        LL_TIM_OC_SetCompareCH1(TIM2, (arr*a*amplitude) / (1000*100));
        LL_TIM_OC_SetCompareCH2(TIM2, (arr*b*amplitude) / (1000*100));
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_2);
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_3);
        break;
    case 1: // zweiter Quadrant
        LL_TIM_OC_SetCompareCH1(TIM2, (arr*a*amplitude) / (1000*100));
        LL_TIM_OC_SetCompareCH2(TIM2, ((100000- b* amplitude) * arr) / 100000);
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_2);
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_3);
        break;
    case 2: // dritter Quadrant
        LL_TIM_OC_SetCompareCH1(TIM2, ((100000- a* amplitude) * arr) / 100000);
        LL_TIM_OC_SetCompareCH2(TIM2, ((100000- b* amplitude) * arr) / 100000);
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_2);
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_3);
        break;
    case 3: // vierter Quadrant
        LL_TIM_OC_SetCompareCH1(TIM2, ((100000- a* amplitude) * arr) / 100000);
        LL_TIM_OC_SetCompareCH2(TIM2, (arr*b*amplitude) / (1000*100));
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_2);
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_3);
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
		}
		LL_TIM_SetCounter(TIM6,0);
		LL_TIM_SetAutoReload(TIM6, (1000000/zahl) -1); // T = 1MHz/f
		LL_TIM_EnableCounter(TIM6);
	}
}

void setMicrosteps(uint16_t ms){
    microsteps = ms;
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