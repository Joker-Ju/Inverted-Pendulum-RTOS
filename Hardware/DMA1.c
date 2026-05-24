#include "stm32f10x.h"
#include "ADC1.h"

void DMA1_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN; //使能DMA1时钟
    DMA1_Channel1->CCR &= ~DMA_CCR1_EN; //配置前先关闭通道

    //配置DMA通道1
    //数据传输方向设置
    // DMA1_Channel1->CCR &= ~DMA_CCR1_MEM2MEM; //外设到内存
    DMA1_Channel1->CCR &= ~DMA_CCR1_DIR; //从外设到内存
    //数据宽度设置
    DMA1_Channel1->CCR &= ~DMA_CCR1_MSIZE;
    DMA1_Channel1->CCR |= DMA_CCR1_MSIZE_0; //内存数据宽度为16位   
    DMA1_Channel1->CCR &= ~DMA_CCR1_PSIZE;
    DMA1_Channel1->CCR |= DMA_CCR1_PSIZE_0; //外设数据宽度为16位

    //循环模式设置，内存地址自增，外设不自增

    //改成非循环模式，主循环直接读缓冲即可
    DMA1_Channel1->CCR &= ~DMA_CCR1_CIRC; 


    DMA1_Channel1->CCR |= DMA_CCR1_MINC; //内存地址自动递增
    DMA1_Channel1->CCR &= ~DMA_CCR1_PINC; //外设地址固定


    //启用DMA中断，传输完成中断使能
    // DMA1_Channel1->CCR |= DMA_CCR1_EN;
    DMA1_Channel1->CCR |= DMA_CCR1_TCIE;
    //配置NVIC
    NVIC_SetPriorityGrouping(3); //设置优先级分组为3
    NVIC_SetPriority(DMA1_Channel1_IRQn, 5); //设置DMA1中断优先级(必须 >= configMAX_SYSCALL_INTERRUPT_PRIORITY(5))
    NVIC_EnableIRQ(DMA1_Channel1_IRQn); //使能DMA1中断

}

void DMA1_StartConvert(uint32_t *dstAddr, uint8_t Len)
{
    DMA1_Channel1->CPAR = (uint32_t)&(ADC1->DR); //外设地址固定为ADC1数据寄存器
    DMA1_Channel1->CMAR = (uint32_t)dstAddr; //目标缓存地址
    DMA1_Channel1->CNDTR = Len; //按半字计数
    DMA1_Channel1->CCR |= DMA_CCR1_EN; //开启DMA
}

void DMA1_ADC1_StartConvert(uint32_t *dstAddr, uint8_t Len)
{
    ADC1_StartConvert(); //启动ADC1转换
    DMA1_StartConvert(dstAddr, Len); //启动DMA1传输
}


void DMA1_Reset_CNDTR(uint16_t number)
{
        ADC1_DMA_toggle(); // 通过切换 DMA 请求来复位 ADC 内部状态，确保下一次转换能正确触发 DMA
        DMA1_Channel1->CCR &= ~DMA_CCR1_EN;     // 关 DMA 通道
    	DMA1->IFCR |= DMA_IFCR_CTCIF1; //清除DMA的TCIF 
        DMA1_Channel1->CNDTR = number; //重装载计数器
        DMA1_Channel1->CCR |= DMA_CCR1_EN;
}



