#include "stm32f10x.h"                  // Device header
void Led_Init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN ; //使能GPIOA时钟
	GPIOC->CRH &= ~GPIO_CRH_MODE13 ; //PC13清除原有设置
	GPIOC->CRH |= GPIO_CRH_MODE13 ; //PC13输出速度50MHz
	GPIOC->CRH &= ~GPIO_CRH_CNF13 ; //PC13推挽输出

	GPIOC->ODR |= GPIO_ODR_ODR13 ; //PC13置1，LED灭
}

void Led_On(void)
{
	GPIOC->ODR &= ~GPIO_ODR_ODR13 ; //PC13置0，LED亮
}                                    
                                     
void Led_Off(void)                   
{                                    
	GPIOC->ODR |= GPIO_ODR_ODR13 ; //PC13置1，LED灭
}

void Led_Toggle(void)
{
	//LED状态取反
	GPIOC->ODR ^= GPIO_ODR_ODR13 ; //PC13取反
}
