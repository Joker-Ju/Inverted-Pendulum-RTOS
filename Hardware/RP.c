#include "stm32f10x.h"

void RP_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC2EN; //使能ADC2时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //使能GPIOA时钟
    GPIOA->CRL &= ~GPIO_CRL_MODE2; //PA2清除原有设置
    GPIOA->CRL &= ~GPIO_CRL_CNF2; //PA2模拟输入

}

void RP_GetValue(uint16_t* value1, uint16_t* value2, uint16_t* value3, uint16_t* value4)
{
    ADC2->CR2 |= ADC_CR2_SWSTART; //软件触发开始转换
    while (!(ADC2->SR & ADC_SR_EOC)); //等待转换完成
    *value1 = ADC2->DR; //读取通道1的转换结果

    while (!(ADC2->SR & ADC_SR_EOC)); //等待转换完成
    *value2 = ADC2->DR; //读取通道2的转换结果

    while (!(ADC2->SR & ADC_SR_EOC)); //等待转换完成
    *value3 = ADC2->DR; //读取通道3的转换结果

    while (!(ADC2->SR & ADC_SR_EOC)); //等待转换完成
    *value4 = ADC2->DR; //读取通道4的转换结果
}

