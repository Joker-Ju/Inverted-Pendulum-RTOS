#include "My_RTOS.h"
#include "stm32f10x.h"

int main(void)
{

	// ── 调试：看看 App 跑起来了没 ──
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	GPIOC->CRH &= ~(0xF << 20);
	GPIOC->CRH |= (0x3 << 20);
	GPIOC->ODR ^= (1 << 13);     // LED 亮
	for (volatile uint32_t i = 0; i < 5000000; i++);
	GPIOC->ODR ^= (1 << 13);     // LED 灭
	// ── 调试结束 ──
	My_RTOS_Start();
    //后面代码没意义了
    while (1) 
	{
		
	}
}

