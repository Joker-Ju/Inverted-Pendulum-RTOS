#ifndef __USART2_H
#define __USART2_H

#include "stm32f10x.h"


void Usart2_Init(uint32_t baud);
void Usart2_SendByte(uint8_t c);
void Usart2_SendStr(const char *s);
uint8_t Usart2_RecvByte(uint8_t *byte, uint32_t timeout_us);
void Usart2_Flush(void);

#endif
