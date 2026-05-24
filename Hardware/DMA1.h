#ifndef __DMA1_H
#define __DMA1_H

void DMA1_Init(void);
void DMA1_StartConvert(uint32_t *dstAddr, uint8_t Len);
void DMA1_ADC1_StartConvert(uint32_t *dstAddr, uint8_t Len);
void DMA1_Reset_CNDTR(uint16_t number);

#endif
