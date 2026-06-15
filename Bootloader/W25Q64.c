#include "W25Q64.h"

#define SPI_Start_pa1()     do { SPI_Hard_Start(GPIOA, 1); } while(0)
#define SPI_Stop_pa1()      do { SPI_Hard_Stop(GPIOA, 1); } while(0)

// ── 读状态寄存器 ──
static uint8_t W25Q64_ReadSR(void) {
	SPI_Start_pa1();
	SPI_Hard_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
	uint8_t sr = SPI_Hard_SwapByte(W25Q64_DUMMY_BYTE);
	SPI_Stop_pa1();
	return sr;
}

// ── 等待忙 ──
static void W25Q64_WaitBusy(void) {
	while (W25Q64_ReadSR() & 0x01);
}

// ── 写使能 ──
static void W25Q64_WriteEnable(void) {
	SPI_Start_pa1();
	SPI_Hard_SwapByte(W25Q64_WRITE_ENABLE);
	SPI_Stop_pa1();
}


void W25Q64_Init(void)
{
	SPI_Hard_Init();
	for (volatile uint32_t i = 0; i < 72000; i++);
}

// ── 读设备 ID ──
// 用 Release Power Down / Device ID 指令（0xAB）
// 正常返回 0x16（W25Q64）
uint8_t W25Q64_ReadID(void) {
	
	SPI_Start_pa1();
	SPI_Hard_SwapByte(W25Q64_RELEASE_POWER_DOWN_HPM_DEVICE_ID);
	SPI_Hard_SwapByte(W25Q64_DUMMY_BYTE);       // dummy × 3
	SPI_Hard_SwapByte(W25Q64_DUMMY_BYTE);
	SPI_Hard_SwapByte(W25Q64_DUMMY_BYTE);
	uint8_t id = SPI_Hard_SwapByte(W25Q64_DUMMY_BYTE); // 第 4 个 dummy 时读到 ID
	SPI_Stop_pa1();

	return id;
}

void W25Q64_Read(uint32_t addr, uint8_t *buf, uint32_t len) {
	SPI_Start_pa1();
	SPI_Hard_SwapByte(W25Q64_READ_DATA);
	SPI_Hard_SwapByte((addr >> 16) & 0xFF);
	SPI_Hard_SwapByte((addr >> 8)  & 0xFF);
	SPI_Hard_SwapByte(addr        & 0xFF);
	for (uint32_t i = 0; i < len; i++) {
		buf[i] = SPI_Hard_SwapByte(W25Q64_DUMMY_BYTE);
	}
	SPI_Stop_pa1();
}

static void W25Q64_PageProgram(uint32_t addr, uint8_t *buf, uint32_t len) {
	W25Q64_WriteEnable();
	SPI_Start_pa1();
	SPI_Hard_SwapByte(W25Q64_PAGE_PROGRAM);
	SPI_Hard_SwapByte((addr >> 16) & 0xFF);
	SPI_Hard_SwapByte((addr >> 8)  & 0xFF);
	SPI_Hard_SwapByte(addr        & 0xFF);
	for (uint32_t i = 0; i < len; i++) {
		SPI_Hard_SwapByte(buf[i]);
	}
	SPI_Stop_pa1();
	W25Q64_WaitBusy();
}

void W25Q64_SectorErase(uint32_t addr) {
	W25Q64_WriteEnable();
	SPI_Start_pa1();
	SPI_Hard_SwapByte(W25Q64_SECTOR_ERASE_4KB);
	SPI_Hard_SwapByte((addr >> 16) & 0xFF);
	SPI_Hard_SwapByte((addr >> 8)  & 0xFF);
	SPI_Hard_SwapByte(addr        & 0xFF);
	SPI_Stop_pa1();
	W25Q64_WaitBusy();
}

void W25Q64_Write(uint32_t addr, uint8_t *buf, uint32_t len) {
	uint32_t remain = len;
	uint32_t offset = 0;

	while (remain > 0) {
		uint32_t page_remain = 256 - (addr & 0xFF);
		uint32_t write_len = (remain < page_remain) ? remain : page_remain;

		W25Q64_PageProgram(addr, buf + offset, write_len);

		addr   += write_len;
		offset += write_len;
		remain -= write_len;
	}
}

static uint8_t w25q64_buf[4096];

void W25Q64_Safe_Write(uint32_t addr, uint8_t *buf, uint32_t len) {
	uint32_t remain = len;
	uint32_t offset = 0;

	while (remain > 0) {
		// 当前地址属于哪个扇区
		uint32_t sector = addr / 4096;
		uint32_t sector_start = sector * 4096;

		// 这次写会跨扇区吗？不跨就写到 len，跨就写到扇区末尾
		uint32_t sector_end = sector_start + 4096;
		uint32_t chunk_end = addr + remain;
		if (chunk_end > sector_end) chunk_end = sector_end;
		uint32_t chunk_len = chunk_end - addr;

		// ── Read-Modify-Write：读整个扇区到缓冲 ──
		W25Q64_Read(sector_start, w25q64_buf, 4096);

		// 在内存里覆盖要写的内容
		for (uint32_t i = 0; i < chunk_len; i++) {
			w25q64_buf[(addr - sector_start) + i] = buf[offset + i];
		}

		// 擦除扇区
		W25Q64_SectorErase(sector_start);

		// 逐页写回（用回 PageProgram）
		for (uint32_t page_off = 0; page_off < 4096; page_off += 256) {
			W25Q64_PageProgram(sector_start + page_off,
							w25q64_buf + page_off, 256);
		}

		addr   += chunk_len;
		offset += chunk_len;
		remain -= chunk_len;
	}
}
