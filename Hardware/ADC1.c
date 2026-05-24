#include "STM32f10x.h"
#include "ADC1.h"


void ADC1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //使能GPIOA时钟
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; //使能ADC1时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //使能GPIOB时钟

    //配置ADCPRE寄存器，PCLK2分频6，ADCCLK=72MHz/6=12MHz，满足ADC时钟不超过14MHz的要求
    RCC->CFGR |= RCC_CFGR_ADCPRE_1; //PCLK2分频6
    RCC->CFGR &= ~RCC_CFGR_ADCPRE_0; //PCLK2分频6
    
    

    //配置GPIOA2，3，4，5为模拟输入
    GPIOA->CRL &= ~GPIO_CRL_CNF2; //PA2模拟输入
    GPIOA->CRL &= ~GPIO_CRL_MODE2; //PA2输入模式
    GPIOA->CRL &= ~GPIO_CRL_CNF3; //PA3模拟输入
    GPIOA->CRL &= ~GPIO_CRL_MODE3; //PA3输入模式
    GPIOA->CRL &= ~GPIO_CRL_CNF4; //PA4模拟输入
    GPIOA->CRL &= ~GPIO_CRL_MODE4; //PA4输入模式
    GPIOA->CRL &= ~GPIO_CRL_CNF5; //PA5模拟输入
    GPIOA->CRL &= ~GPIO_CRL_MODE5; //PA5输入模式
    //配置GPIOB0，为模拟输入
    GPIOB->CRL &= ~GPIO_CRL_CNF0; //PB0模拟输入
    GPIOB->CRL &= ~GPIO_CRL_MODE0; //PB0输入模式
        
    //开启扫描模式
    ADC1->CR1 |= ADC_CR1_SCAN; //使能扫描模式
    //去掉连续转换模式，使用外部触发单次转换，这样才是通过外部TRGO事件触发ADC转换
    ADC1->CR2 &= ~ADC_CR2_CONT; //禁止连续转换模式
    // //开启连续转换模式
    // ADC1->CR2 |= ADC_CR2_CONT; //使能连续转换模式
    //数据右对齐
    ADC1->CR2 &= ~ADC_CR2_ALIGN; //右对齐


    //设置采样时间为7.5周期 设置寄存器为001
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP2; //PA2采样时间为7.5周期
    ADC1->SMPR2 |= ADC_SMPR2_SMP2_0; //PA2采样时间为7.5周期
    
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP3; //PA3采样时间为7.5周期
    ADC1->SMPR2 |= ADC_SMPR2_SMP3_0; //PA3采样时间为7.5周期

    ADC1->SMPR2 &= ~ADC_SMPR2_SMP4; //PA4采样时间为7.5周期
    ADC1->SMPR2 |= ADC_SMPR2_SMP4_0; //PA4采样时间为7.5周期

    ADC1->SMPR2 &= ~ADC_SMPR2_SMP5; //PA5采样时间为7.5周期
    ADC1->SMPR2 |= ADC_SMPR2_SMP5_0; //PA5采样时间为7.5周期

    ADC1->SMPR2 &= ~ADC_SMPR2_SMP8; //PB0采样时间为7.5周期
    ADC1->SMPR2 |= ADC_SMPR2_SMP8_0; //PB0采样时间为7.5周期

    //规则组通道序列配置，长度为5，依次转换通道为2、3、4、5、8
    ADC1->SQR1 &= ~ADC_SQR1_L; //清除转换序列长度位
    ADC1->SQR1 |= ADC_SQR1_L_2; //设置转换序列长度为5，即转换5个通道


    ADC1->SQR3 = 0;
    ADC1->SQR3 |= 2 << 0; //第1个转换通道为PA2(ADC_IN2)
    ADC1->SQR3 |= 3 << 5; //第2个转换通道为PA3(ADC_IN3)
    ADC1->SQR3 |= 4 << 10; //第3个转换通道为PA4(ADC_IN4)
    ADC1->SQR3 |= 5 << 15; //第4个转换通道为PA5(ADC_IN5)
    ADC1->SQR3 |= 8 << 20; //第5个转换通道为PB8(ADC_IN8)

    //使用外部事件启动转换
    ADC1->CR2 |= ADC_CR2_EXTTRIG; //使能外部事件触发
    //选择外部事件为SWSTART，这样我们就可以通过软件设置SWSTART位来触发ADC转换
    ADC1->CR2 |= ADC_CR2_EXTSEL;


    
    
    //打开ADC1的DMA请求
    ADC1->CR2 |= ADC_CR2_DMA; //使能ADC1的DMA请求   

}

void ADC1_StartConvert(void)
{
    //上电并校准ADC1，先上电后再打开DMA请求，确保DMA配置完成后再启动ADC转换
    ADC1->CR2 |= ADC_CR2_ADON; //上电ADC1    


    ADC1->CR2 |= ADC_CR2_RSTCAL; //复位校准寄存器
    while (ADC1->CR2 & ADC_CR2_RSTCAL) {};
    ADC1->CR2 |= ADC_CR2_CAL; //校准
    while (ADC1->CR2 & ADC_CR2_CAL){}; //等待校准完成
    //先不启动转换，等 PID 任务通知后再启动，这样可以确保第一次转换时 DMA 已经准备好了
    // ADC1->CR2 |= ADC_CR2_ADON; //再次上电启动转换
}

// erratum：STM32F103 的 ADC 在某些条件下可能会出现 DMA 无法正确触发的情况，复位 ADC 内部状态可以解决这个问题
void ADC1_DMA_toggle(void)
{
    ADC1->CR2 &= ~ADC_CR2_DMA;   // 断开 DMA 请求
    ADC1->CR2 |= ADC_CR2_DMA;    // 重新连接，内部状态复位
}

void ADC1_SWSTART(void)
{
    ADC1->CR2 |= ADC_CR2_SWSTART; //软件触发ADC转换
}


void ADC1_Stop(void)
{
    ADC1->CR2 &= ~ADC_CR2_ADON; //关闭ADC1
}



