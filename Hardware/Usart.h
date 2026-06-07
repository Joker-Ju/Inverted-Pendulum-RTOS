#ifndef __USART_H
#define __USART_H

#include <stdio.h>

uint8_t  Usart_IsDataReady(void);     // 数据收齐了吗？
uint8_t* Usart_GetData(void);         // 拿到 buffer 指针
uint8_t  Usart_GetSize(void);          // 拿长度
void     Usart_ClearFlag(void);       // 处理完复位 size+flag
void	 Usart_ClearSize(void);
uint8_t  Usart_MatchCmd(const uint8_t *cmd, uint8_t len);  // 匹配返回1，不匹配返回0

void 	 Usart_Init(void);
void 	 Usart_TransmitChar(uint8_t ch);
//uint8_t Usart_ReceiveChar(void);
void 	 Usart_TransmitString(uint8_t *str, uint8_t size);
//void Usart_ReceiveString(uint8_t buffer[], uint8_t* size);

#endif
