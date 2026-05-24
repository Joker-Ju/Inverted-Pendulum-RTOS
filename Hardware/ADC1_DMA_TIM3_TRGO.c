#include "stm32f10x.h"                 // Device header
#include "TIM3.h"
#include "DMA1.h"
#include "ADC1.h"
extern uint32_t *dstAddr;
extern uint8_t Len;

void ADC1_DMA_TIM3_TRGO_Init(uint32_t *dstAddr, uint8_t Len)
{
    ADC1_Init(); //初始化ADC1
    DMA1_Init(); //初始化DMA1
    //配置DMA1通道1，将ADC1的数据寄存器地址作为外设地址
    ADC1_DMA1_StartConvert(dstAddr, Len); //配置并启动DMA1传输

    TIM3_Init(); //最后开启TIM3，开始周期性TRGO触发

}

              


