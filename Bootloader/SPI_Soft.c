#include "SPI_Soft.h"

// ── 引脚定义 ──
#define SPI_SCK_PIN     (1 << 5)    // PA5
#define SPI_MISO_PIN    (1 << 6)    // PA6
#define SPI_MOSI_PIN    (1 << 7)    // PA7
#define SPI_CS_PIN      (1 << 1)    // PA1

// ── 宏操作 ──
#define SCK_H()     GPIOA->BSRR = SPI_SCK_PIN
#define SCK_L()     GPIOA->BRR  = SPI_SCK_PIN

#define MOSI_H()    GPIOA->BSRR = SPI_MOSI_PIN
#define MOSI_L()    GPIOA->BRR  = SPI_MOSI_PIN

#define MISO_GET()  ((GPIOA->IDR >> 6) & 1)

#define CS_H()      GPIOA->BSRR = SPI_CS_PIN
#define CS_L()      GPIOA->BRR  = SPI_CS_PIN

// ── 延时（~1μs @ 72MHz） ──
static void spi_delay(void) {
    for (volatile uint32_t i = 0; i < 72; i++);
}

void SPI_Soft_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;     // GPIOA 时钟

    // PA1(CS):  推挽输出 50MHz
    // PA5(SCK): 推挽输出 50MHz
    // PA6(MISO): 浮空输入
    // PA7(MOSI): 推挽输出 50MHz
    GPIOA->CRL &= ~(GPIO_CRL_CNF1 | GPIO_CRL_CNF5 | GPIO_CRL_CNF7); // 清除 PA1~PA7 的配置
    GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_MODE5 | GPIO_CRL_MODE7); // 50MHz
    GPIOA->CRL |= (GPIO_CRL_MODE1 |GPIO_CRL_MODE5 | GPIO_CRL_MODE7); // 50MHz
    GPIOA->CRL |= (GPIO_CRL_CNF6_0); // PA6 浮空输入
    // 默认状态：空闲
    CS_H();         // 不选中
    SCK_L();        // 空闲低（CPOL=0）
}

// ── SPI Mode 0 收发一字节 ──
// CPOL=0（空闲低），CPHA=0（第一个边沿采样）
// SCK 时序：低 → 高（上升沿采样）→ 低（下降沿变化）
// MOSI  Bit7  Bit6  Bit5  Bit4  Bit3  Bit2  Bit1  Bit0
uint8_t SPI_Soft_SwapByte(uint8_t data) {
    uint8_t rByte = 0;
    for (int i = 0; i < 8; i++) {
        // 在下降沿设置 MOSI（数据变化）
        if (data & 0x80)
			MOSI_H();
        else
            MOSI_L();
		data <<= 1; 
        rByte <<= 1;         // 左移，准备下一个 bit
        spi_delay();

        SCK_H();            // 上升沿 → 从机采样 MOSI，同时从机把 MISO 输出
        spi_delay();

        
        // ⭐ 在 SCK 高电平时读 MISO（此时从机数据已稳定）
        rByte |= MISO_GET(); // 读 MISO 放到最低位（后续左移会到高位）


        SCK_L();            // 拉低 SCK，准备下一个 bit
        spi_delay();
    }

    return rByte;            // 返回收到的字节
}
