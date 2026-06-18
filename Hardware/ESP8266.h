#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"
#include "Usart2.h"

// 返回值
#define ESP8266_OK      0
#define ESP8266_TIMEOUT 1
#define ESP8266_FAIL    2
#define ESP8266_CLOSED  3


// ── 公开接口 ──
uint8_t     ESP8266_IsAlive(void);   // 发 AT 等 OK，返回 1=在线 0=离线
uint8_t     ESP8266_Init(void);                              // 初始化 UART2 + 检测模块
uint8_t     ESP8266_ConnectAP(const char *ssid, const char *pwd);  // 连 WiFi 热点
uint8_t     ESP8266_TCPConnect(const char *ip, uint16_t port);     // 连 TCP 服务器
// ── 被动模式（CIPRECVMODE=1）新接口 ──
uint32_t    ESP8266_WaitData(uint32_t timeout_ms);
uint32_t    ESP8266_ReadData(uint8_t *buf, uint32_t len);
void        ESP8266_CloseConnection(void);   // 关闭连接，返回残留
uint8_t     ESP8266_IsTCPConnected(void);  // 检查 TCP 是否还连着，返回 1=已连接，0=已断开
void        ESP8266_ResetClosedFlag(void);

#endif
