#include "stm32f10x.h"

void PWM_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //使能TIM2时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //使能GPIOA时钟
    TIM2->PSC = 36 - 1; //预分频器，72MHz/36=2MHz
    TIM2->ARR = 100 - 1; //自动重装载寄存器，2MHz/100=20kHz，即每50us更新一次

    GPIOA->CRL &= ~GPIO_CRL_MODE0; //PA0清除原有设置
    GPIOA->CRL |= GPIO_CRL_MODE0; //PA0输出速度50MHz
    GPIOA->CRL |= GPIO_CRL_CNF0_1; //PA0复用推挽输出
    GPIOA->CRL &= ~GPIO_CRL_CNF0_0; //PA0复用推挽输出

    TIM2->CR1 &= ~TIM_CR1_DIR; //向上计数
    
    TIM2->CCR1 = 0; //初始占空比为0%
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_2; //选择PWM模式1
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1; //选择PWM模式1
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M_0; //选择PWM模式1
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE; //使能CCR1预装载



    TIM2->CCER |= TIM_CCER_CC1E; //使能CH1输出


    TIM2->CR1 |= TIM_CR1_CEN; //使能TIM2
}

void PWM_SetDutyCycle(uint8_t duty)
{
    if (duty > 100) 
	{
		duty = 100; //限制占空比在0-100%
	}
    TIM2->CCR1 = duty; //设置占空比
}
