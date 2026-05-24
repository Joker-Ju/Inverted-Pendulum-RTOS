#include "stm32f10x.h"                  // Device header


void Encoder_Init(void)
{
	/*开启时钟*/
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //使能GPIOA时钟
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //使能TIM3时钟

	/*GPIO初始化*/
	GPIOA->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_CNF7); //PA6，7上拉输入
	GPIOA->CRL |= GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1; //PA6，7上拉输入
	GPIOA->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_MODE7); //PA6，7输入模式

	//将PA6和PA7引脚初始化为上拉输入
	
		
	/*时基单元初始化*/
	TIM3->ARR = 65536 - 1; //设置计数周期
	TIM3->PSC = 1 - 1;     //设置预分频器
	TIM3->CR1 &= ~TIM_CR1_DIR;        //向上计数模式
	TIM3->CR1 &= ~TIM_CR1_CKD;       //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	//重复计数器高级定时器才有，TIM3没有，不需要配置
	TIM3->EGR |= TIM_EGR_UG; //更新事件，立即加载预分频器和自动重装载寄存器的值
	TIM3->SR &= ~TIM_SR_UIF; //清除更新中断标志位，避免误触发更新中断

	// /*输入捕获初始化*/

	TIM3->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S); 
	TIM3->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0); //CC1S=01，CC1通道配置为输入；CC2S=01，CC2通道配置为输入
	TIM3->CCMR1 &= ~(TIM_CCMR1_IC1F | TIM_CCMR1_IC2F);
	TIM3->CCMR1 |= (0xF << 4) | (0xF << 12); //IC1F=1111，IC2F=1111，输入滤波器参数，可以过滤信号抖动
	TIM3->CCMR1 &= ~(TIM_CCMR1_IC1PSC | TIM_CCMR1_IC2PSC); //IC1PSC=00，IC2PSC=00，输入捕获预分频器，选择不分频
	
	TIM3->CCER &= ~TIM_CCER_CC1P; //CC1P=0，CC1通道不反相
	TIM3->CCER |= TIM_CCER_CC2P; //CC2P=1，CC2通道反相
	TIM3->EGR |= TIM_EGR_UG; //更新事件，立即加载前面的配置
	TIM3->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC2IF); //清除捕获比较通道1和通道2的中断标志位，避免误触发捕获比较中断
	
	TIM3->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;  // 编码器模式3，4个边沿计数

	TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E; //使能捕获比较通道1和通道2
	/*TIM使能*/
	TIM3->CR1 |= TIM_CR1_CEN; //使能TIM3，定时器开始运行
}


int16_t Encoder_Get(void)
{
	/*使用Temp变量作为中继，目的是返回CNT后将其清零*/
	int16_t Temp;
	Temp = TIM3->CNT; //读取编码器计数值
	TIM3->CNT = 0;   //读取后立即清零，准备下一次读取
	return Temp;
}
