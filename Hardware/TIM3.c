#include "stm32f10x.h"

void TIM3_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //使能TIM3时钟

    //配置TIM3为基本定时器模式
    TIM3->CR1 &= ~TIM_CR1_CEN;//先关闭TIM3
    TIM3->PSC = 7200 - 1; //预分频器，计数频率为72MHz/7200=10kHz
    TIM3->ARR = 10 - 1; //自动重装载值，定时周期为10kHz/10=1kHz，即1ms
    //选择TIM3的TRGO事件为更新事件
    TIM3->CR2 &= ~TIM_CR2_MMS;
    TIM3->CR2 |= TIM_CR2_MMS_1; //MMS=010, TRGO=Update Event

    TIM3->EGR |= TIM_EGR_UG; //更新事件，装载PSC/ARR

    TIM3->CR1 |= TIM_CR1_CEN; //使能TIM3
}
