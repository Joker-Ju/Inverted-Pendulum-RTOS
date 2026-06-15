#ifndef __SPI_SOFT_H
#define __SPI_SOFT_H

#include "stm32f10x.h"                  // Device header

// ── CS 通用宏（需要传参） ──
#define SPI_Soft_Start(port, pin)     do { (port)->BRR  = (1 << (pin)); } while(0)
#define SPI_Soft_Stop(port, pin)     do { (port)->BSRR = (1 << (pin)); } while(0)

void SPI_Soft_Init(void);                       // 初始化引脚
uint8_t SPI_Soft_SwapByte(uint8_t data);    // 同时收发一字节

#endif
