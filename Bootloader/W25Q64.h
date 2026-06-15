#ifndef __W25Q64_H
#define __W25Q64_H

#include "stm32f10x.h"                  // Device header
#include "W25Q64_Ins.h"
#include "SPI_Hard.h"

void    W25Q64_Init(void);
uint8_t W25Q64_ReadID(void);
void    W25Q64_Read(uint32_t addr, uint8_t *buf, uint32_t len);
void    W25Q64_Write(uint32_t addr, uint8_t *buf, uint32_t len);
void    W25Q64_SectorErase(uint32_t addr);
void    W25Q64_Safe_Write(uint32_t addr, uint8_t *buf, uint32_t len) ; // Read-Modify-Write 安全写

#endif
