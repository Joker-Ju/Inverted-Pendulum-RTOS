#include "ESP8266.h"



// ═══════════════════════════════════════════════════
//  AT 指令收发
// ═════════════════════════════════════════════════──
// 等响应中出现 expect 字符串
// 内部原理：逐字节跟 expect 比对
// 比如 expect="OK"，收到 'O' 就等 'K'，收到 'K' 就返回成功
// 如果中间出现不匹配的字符，从头开始重新匹配
static uint8_t wait_response(const char *expect, uint32_t timeout_ms) {
    const char *p = expect;      // p 指向 expect 中下一个要匹配的字符
    uint32_t t = timeout_ms * 20;  // 转成 ~μs

    while (t--) {
        uint8_t c;
        if (!Usart2_RecvByte(&c, 50))
            continue;            // 超时，继续等

        if (*p == c) {           // 匹配上了
            p++;                 // 指针移到下一个期待字符
            if (*p == '\0')      // 匹配完了！
                return ESP8266_OK;
        } 
        else {
            p = expect;          // 匹配失败，从头开始
            if (c == expect[0])  // 但这个字节可能正好是开头
                p = expect + 1;
        }
    }
    return ESP8266_TIMEOUT;
}

// 发一条 AT 指令 + 等预期回复
// cmd: "AT+CWMODE=1"
// expect: "OK"
static uint8_t at_cmd(const char *cmd, const char *expect, uint32_t timeout_ms) {
    Usart2_Flush();               // 清掉上次残留数据
    Usart2_SendStr(cmd);
    Usart2_SendStr("\r\n");      // AT 指令以 \r\n 结尾
    return wait_response(expect, timeout_ms);
}


// ═══════════════════════════════════════════════════
//  公开 API
// ══════════════════════════════════════════════════

uint8_t ESP8266_IsAlive(void) {
    for (int i = 0; i < 10; i++) {
        Usart2_Flush();
        Usart2_SendStr("AT\r\n");
        if (wait_response("OK", 1000) == ESP8266_OK)
            return 1;                       // 在线！
    }
    return 0;                               // 10 次都超时
}

uint8_t ESP8266_Init(void) {
    Usart2_Init(115200);

    // ── 直接调 IsAlive ──
    if (!ESP8266_IsAlive())
        return ESP8266_TIMEOUT;

    at_cmd("AT+CWMODE=1", "OK", 2000);
    at_cmd("ATE0", "OK", 1000);
    at_cmd("AT+CIPRECVMODE=1", "OK", 1000);  // ← 加这行
    return ESP8266_OK;
}

uint8_t ESP8266_ConnectAP(const char *ssid, const char *pwd) {
    Usart2_Flush();
    Usart2_SendStr("AT+CWJAP=\"");
    Usart2_SendStr(ssid);
    Usart2_SendStr("\",\"");
    Usart2_SendStr(pwd);
    Usart2_SendStr("\"\r\n");

    // 连 WiFi 涉及 DHCP，给 15 秒
    return wait_response("OK", 15000);
}

uint8_t ESP8266_TCPConnect(const char *ip, uint16_t port) {
    Usart2_Flush();
    Usart2_SendStr("AT+CIPSTART=\"TCP\",\"");
    Usart2_SendStr(ip);
    Usart2_SendStr("\",");

    // 端口转成字符串发出去（itoa 的简化版）
    char port_str[6];
    int idx = 0;
    uint32_t p = port;
    do { port_str[idx++] = '0' + (p % 10); p /= 10; } while (p);

    // 数字是反的（个位在前），倒着发
    for (int i = idx - 1; i >= 0; i--)
        Usart2_SendByte(port_str[i]);

    Usart2_SendStr("\r\n");
    return wait_response("CONNECT", 10000);  // TCP 连接成功回 CONNECT
}



static uint8_t wait_ipd_or_closed(uint32_t timeout_ms) {
    const char closed_word[] = "CLOSED";
    uint8_t closed_match = 0;
    uint32_t t = timeout_ms * 20;

    while (t--) {
        uint8_t c;
        if (!Usart2_RecvByte(&c, 50))
            continue;

        if (c == '+')
            return ESP8266_OK;

        if (c == closed_word[closed_match]) {
            closed_match++;
            if (closed_match == sizeof(closed_word) - 1)
                return ESP8266_CLOSED;
        } else {
            closed_match = (c == 'C') ? 1 : 0;
        }
    }

    return ESP8266_TIMEOUT;
}


// ══════════════════════════════════════════════════════
//  被动模式：等 +IPD,<len>（不带数据，只通知长度）
//  返回缓存中的字节数，0=超时/CLOSED
// ══════════════════════════════════════════════════════
static uint32_t check_buffered_len(void) {
    uint8_t c;

    Usart2_Flush();
    Usart2_SendStr("AT+CIPRECVLEN?\r\n");

    // 等 '+'
    for (uint32_t t = 10000; t; t--) {
        if (!Usart2_RecvByte(&c, 50)) continue;
        if (c == '+') break;
        if (t == 1) return 0;  // 超时
    }

    // 确认 "CIPRECVLEN:"
    const char *hdr = "CIPRECVLEN:";
    while (*hdr) {
        if (!Usart2_RecvByte(&c, 5000) || c != *hdr) return 0;
        hdr++;
    }

    // 读第一个数字（link 0 的缓存量）
    uint32_t val = 0;
    while (1) {
        if (!Usart2_RecvByte(&c, 5000)) return 0;
        if (c >= '0' && c <= '9')
            val = val * 10 + (c - '0');
        else
            break;  // 遇到逗号或 \r 都停下
    }
    return val;
}


uint32_t ESP8266_WaitData(uint32_t timeout_ms) {
    static uint8_t closed_already = 0;    // ← 记住上次已经 CLOSED 了
    static uint32_t idle_cnt = 0;         // 连续超时次数
    if (closed_already) {
        closed_already = 0;
        idle_cnt = 0;
        return 0xFFFFFFFF;                // ← 直接结束
    }
    uint8_t ret = wait_ipd_or_closed(timeout_ms);
    if (ret == ESP8266_CLOSED) {
        uint32_t remaining = check_buffered_len();
        if (remaining > 0) {
            // 把残留数据当正常 +IPD 返回
            return remaining;
        }
        closed_already = 1;
        return 0xFFFFFFFF;
    }
    if (ret != ESP8266_OK){
        if (++idle_cnt >= 3) {             // 连续 3 次超时 → 结束
            idle_cnt = 0;
            return 0xFFFFFFFF;
        }
        return 0;
    }
    // 确认 "IPD,"
    uint8_t c;
    if (!Usart2_RecvByte(&c, 5000) || c != 'I') return 0;
    if (!Usart2_RecvByte(&c, 5000) || c != 'P') return 0;
    if (!Usart2_RecvByte(&c, 5000) || c != 'D') return 0;
    if (!Usart2_RecvByte(&c, 5000) || c != ',') return 0;

    // ── 读数字直到遇到 ',' 或 '\r' ──
    // +IPD,<len>          ← 单连接
    // +IPD,<linkID>,<len> ← 多连接
    // 两种情况都处理：读到 ',' 说明还在读 linkID，继续
    //                  读到 \r 说明这就是 len
    uint32_t val = 0;
    while (1) {
        if (!Usart2_RecvByte(&c, 5000)) return 0;
        if (c == '\r' || c == '\n') break;   // 数字结束
        if (c >= '0' && c <= '9') {
            val = val * 10 + (c - '0');
        } else if (c == ',') {
            // 遇到逗号，说明刚读完的是 linkID，不是 len
            // 重置 val，读真正 len
            val = 0;
        } else {
            return 0;  // 异常字符
        }
    }

    return val;
}
// ══════════════════════════════════════════════════════
//  被动模式：发 AT+CIPRECVDATA=<len>，读数据
//  返回实际读到的字节数，0=失败
// ══════════════════════════════════════════════════════
uint32_t ESP8266_ReadData(uint8_t *buf, uint32_t len) {
    uint8_t c;

    // ── 发 AT+CIPRECVDATA=<len> ──

    Usart2_SendStr("AT+CIPRECVDATA=");
    char num[12];
    int i = 0;
    uint32_t n = len;
    do { num[i++] = '0' + (n % 10); n /= 10; } while (n);
    while (i--) Usart2_SendByte(num[i]);
    Usart2_SendStr("\r\n");

    // ── 等 "+CIPRECVDATA,<len>:" ──
    uint32_t timeout = 20000;  // ~1s
    while (timeout--) {
        if (!Usart2_RecvByte(&c, 50)) continue;
        if (c == '+') break;
    }
    if (timeout == 0) return 0;

    const char *hdr1 = "CIPRECVDATA,";
    while (*hdr1) {
        if (!Usart2_RecvByte(&c, 5000) || c != *hdr1) return 0;
        hdr1++;
    }
    // 跳过数字（长度）
    while (1) {
        if (!Usart2_RecvByte(&c, 5000)) return 0;
        if (c == ':') break;
    }

    // ── 读 len 字节数据 ──
    uint32_t read_len = len;
    for (uint32_t i = 0; i < read_len; i++) {
        if (!Usart2_RecvByte(&buf[i], 5000)) {
            read_len = i;
            break;
        }
    }
    return read_len;
}



