#include "Usart2.h"



void Usart2_Init(uint32_t baud) {
    // 时钟使能
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;   // USART2 在 APB1 上
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // PA2 = TX：复用推挽输出（CNF=10, MODE=11）
    GPIOA->CRL &= ~(GPIO_CRL_CNF2 | GPIO_CRL_MODE2);  // 先清零
    GPIOA->CRL |= GPIO_CRL_CNF2_1;  // CNF=10
    GPIOA->CRL |= GPIO_CRL_MODE2;  // MODE=11

    // PA3 = RX：浮空输入（CNF=01, MODE=00）
    GPIOA->CRL &= ~(GPIO_CRL_CNF3 | GPIO_CRL_MODE3);  // CNF=01, MODE=00
    GPIOA->CRL |= GPIO_CRL_CNF3_0;  // CNF=01

    // 波特率：USART2 挂在 APB1（36MHz），BRR = 36MHz / baud
    USART2->BRR = 36000000 / baud;
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

// 发 1 字节：等 TXE（发送缓冲空）→ 写 DR
void Usart2_SendByte(uint8_t c) {
    while (!(USART2->SR & USART_SR_TXE));   // TXE=1 表示上一个字节已送出
    USART2->DR = c;
}

// 发字符串：逐个字节发
void Usart2_SendStr(const char *s) {
    while (*s) Usart2_SendByte(*s++);
}
// ── 返回值：1=成功收到，0=超时 ──
// ── 收到的字节通过 byte 指针传回 ──
uint8_t Usart2_RecvByte(uint8_t *byte, uint32_t timeout_us) {
    while (timeout_us--) {
        if (USART2->SR & USART_SR_RXNE) {
            *byte = USART2->DR;        // ← 通过指针传回
            return 1;                  // ← 返回 1 表示成功
        }
        for (volatile uint32_t i = 0; i < 6; i++);
    }
    return 0;                          // ← 返回 0 表示超时
}

// 清空接收缓冲：丢掉残留数据，确保下一条指令不受干扰
void Usart2_Flush(void) {
    uint32_t t = 50000;  // 大约 50ms
    while (t--) {
        if (USART2->SR & USART_SR_RXNE) {
            (void)USART2->DR;   // 读走扔掉
            t = 50000;          // 还有数据，继续等
        }
        for (volatile uint32_t i = 0; i < 6; i++);
    }
}
