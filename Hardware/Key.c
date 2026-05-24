#include "stm32f10x.h"

static uint8_t Key_Num = 0; 
void Key_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //使能GPIOB时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //使能GPIOA时钟

    GPIOB->CRH &= ~GPIO_CRH_MODE10; //PB10清除原有设置
    GPIOB->CRH |= GPIO_CRH_CNF10_1; //PB10上拉输入
    GPIOB->CRH &= ~GPIO_CRH_CNF10_0; //PB10上拉输入
    GPIOB->ODR |= GPIO_ODR_ODR10; //PB10上拉，默认高电平

    GPIOB->CRH &= ~GPIO_CRH_MODE11; //PB11清除原有设置
    GPIOB->CRH |= GPIO_CRH_CNF11_1; //PB11上拉输入
    GPIOB->CRH &= ~GPIO_CRH_CNF11_0; //PB11上拉输入
    GPIOB->ODR |= GPIO_ODR_ODR11; //PB11上拉，默认高电平

    GPIOA->CRH &= ~GPIO_CRH_MODE11; //PA11清除原有设置
    GPIOA->CRH |= GPIO_CRH_CNF11_1; //PA11上拉输入
    GPIOA->CRH &= ~GPIO_CRH_CNF11_0; //PA11上拉输入
    GPIOA->ODR |= GPIO_ODR_ODR11; //PA11上拉，默认高电平

    GPIOA->CRH &= ~GPIO_CRH_MODE12; //PA12清除原有设置
    GPIOA->CRH |= GPIO_CRH_CNF12_1; //PA12上拉输入
    GPIOA->CRH &= ~GPIO_CRH_CNF12_0; //PA12上拉输入
    GPIOA->ODR |= GPIO_ODR_ODR12; //PA12上拉，默认高电平


}

uint8_t Key_GetNum(void)
{
    uint8_t temp;
    if (Key_Num)
    {
        temp = Key_Num;
        Key_Num = 0;
        return temp;
    }
    return 0;
}

uint8_t Key_GetState(void)
{
    if ((GPIOB->IDR & GPIO_IDR_IDR10) == 0) // PB10 按下
    {
        return 1;
    }
    else if ((GPIOB->IDR & GPIO_IDR_IDR11) == 0) // PB11 按下
    {
        return 2;
    }
    else if ((GPIOA->IDR & GPIO_IDR_IDR11) == 0) // PA11 按下
    {
        return 3;
    }
    else if ((GPIOA->IDR & GPIO_IDR_IDR12) == 0) // PA12 按下
    {
        return 4;
    }
    return 0; //无按键按下
}

void Key_Tick(void)
{

    static uint8_t PrevState,CurrState;
	PrevState = CurrState;
	CurrState = Key_GetState();
	if (CurrState == 0 && PrevState !=0)
	{
		Key_Num = PrevState; //记录按键编号
		// 使用Key_GetNum()函数获取按键编号后会自动清零，确保每次按键事件只处理一次
	}
    
}
