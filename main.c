#include "stm32f10x.h"

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

int TimerPeriod = 0, n = 0;
int Channel1Pulse = 0, Channel2Pulse = 0, Channel3Pulse = 0;
uint16_t pulse_width = 0;

void RCC_Configuration(void);
void GPIO_Configuration(void);
void PWM_SetDC(uint16_t dutycycle);
void Delay(__IO uint32_t nCount);

int main(void)
{
	/* System Clocks Configuration */
	SystemInit();
	RCC_Configuration();

  /* GPIO Configuration */
  GPIO_Configuration();

  /* TIM1 Configuration --------------------------------------------------- */
	
	/* Compute the value to be set in ARR regiter to generate signal frequency at 10 Khz */
	TimerPeriod = (SystemCoreClock / 22500);
  /* Compute CCR1 value to generate a duty cycle at 50% for channel 1 and 1N */
  Channel1Pulse = (TimerPeriod / 6);
  /* Compute CCR2 value to generate a duty cycle at 50%  for channel 2 and 2N with 60 degree phase shift*/
  Channel2Pulse = (TimerPeriod / 6) + (TimerPeriod / 3);
  /* Compute CCR3 value to generate a duty cycle at 50%  for channel 3 and 3N with 120 degree phase shift*/
  Channel3Pulse = (TimerPeriod / 6) + ((TimerPeriod * 2) / 3);

	/* Enable the TIM2 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
 
	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 5000 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 5 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	/* TIM IT enable */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);
	
	/* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = TimerPeriod - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* Channel 1, 2,3 and 4 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_Pulse = Channel2Pulse;
  TIM_OC2Init(TIM1, &TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;
  TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	
	/* Dead time configuration (3.7 us) */
  TIM_BDTRInitStructure.TIM_DeadTime = 183;
  TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);
	
	/* TIM1 IT Enable */
// 	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	
	/* TIM1 counter enable */
  TIM_Cmd(TIM1, ENABLE);

  /* TIM1 Main Output Enable */
  TIM_CtrlPWMOutputs(TIM1, ENABLE);

  while (1)
  {	
	}
}

void RCC_Configuration(void)
{
  /* TIM1, GPIOA, GPIOB and AFIO clocks enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM1,DISABLE);
}

/* Configure the TIM1 Pins. */

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
  /* GPIOA Configuration: Channel 1, 2 and 3 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* GPIOB Configuration: Channel 1N, 2N and 3N as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		if (pulse_width <= 100)
    {
      PWM_SetDC(pulse_width++);
    }
		else if (pulse_width > 100)
		{
			PWM_SetDC(pulse_width++);
		}
    Delay(100);
	}
}

void PWM_SetDC(uint16_t dutycycle)
{
    TIM1->CCR1 = dutycycle;
    TIM1->CCR2 = dutycycle;
    TIM1->CCR3 = dutycycle;
}

void Delay(__IO uint32_t nCount)
{
  while(nCount--)
  {
  }
}
