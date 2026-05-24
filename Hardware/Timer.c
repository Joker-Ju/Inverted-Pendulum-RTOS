#include "stm32f10x.h"                  // Device header

void Timer_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; //使能TIM1时钟
    TIM1->PSC = 72 - 1; //预分频器，72MHz/72=1MHz
    TIM1->ARR = 1000 - 1; //自动重装载寄存器，1MHz/1000=1kHz，即每1ms更新一次

    //ADC1规则组只能由TIM3的TRGO触发
    // TIM1->CR2 &= ~TIM_CR2_MMS; //选择TIM1的TRGO事件为更新事件
    // TIM1->CR2 |= TIM_CR2_MMS_1; //MMS=010, TRGO=Update Event
    // TIM1->EGR |= TIM_EGR_UG; //更新事件，装载PSC/ARR寄存器值
    TIM1->DIER |= TIM_DIER_UIE; //使能更新中断


    NVIC_SetPriorityGrouping(3); //设置优先级分组为0
    NVIC_SetPriority(TIM1_UP_IRQn, 0); //设置TIM1更新中断优先级为0
    NVIC_SetPriority(TIM1_TRG_COM_IRQn, 0); //设置TIM1触发/通信中断优先级为0
    NVIC_EnableIRQ(TIM1_UP_IRQn); //使能TIM1更新中断


    TIM1->CR1 &= ~TIM_CR1_DIR; //向上计数
    TIM1->CR1 |= TIM_CR1_ARPE; //使能自动重装载预装载

    TIM1->CR1 |= TIM_CR1_CEN; //使能TIM1
}
