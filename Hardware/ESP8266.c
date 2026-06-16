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

// ── ESP8266_RecvData：解析 +IPD,<len>:<data> ──
//
// 收到的数据格式（ESP8266 固件自动加的）：
//   +IPD,512:[512bytes binary data...]
//    ↑     ↑   ↑
//    1     2   3
//
// 步骤：
//   1. 等 '+'
//   2. 确认 "IPD,"
//   3. 读数字直到 ':'
//   4. 读指定长度的二进制数据

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
// ── +IPD 分包状态 ──
static uint32_t total_ipd_size = 0;   // 这次 +IPD 的总大小
static uint32_t ipd_remaining = 0;  // 还剩多少字节没收完
static uint8_t  ipd_closed_pending = 0;

uint8_t ESP8266_RecvData(uint8_t *buf, uint32_t *len, uint32_t timeout_ms) {
    uint8_t c;
    // 如果上次检测到 CLOSED，这次再报出来
    if (ipd_closed_pending) {
        ipd_closed_pending = 0;
        return ESP8266_CLOSED;
    }
    if (ipd_remaining == 0) {
        uint8_t wait_result = wait_ipd_or_closed(timeout_ms);
        // 如果检测到 CLOSED，先延迟、返回 TIMEOUT（让 caller 先退出）
        if (wait_result == ESP8266_CLOSED) {
            ipd_closed_pending = 1;
            return ESP8266_TIMEOUT;
        }
        if (wait_result != ESP8266_OK)
            return wait_result;

    // ═══ 第 2 步：确认 "IPD," ═══
        // 预期的 4 个字符：I P D ,
        if (!Usart2_RecvByte(&c, 5000) ||  c != 'I') return ESP8266_FAIL;
        if (!Usart2_RecvByte(&c, 5000) ||  c != 'P') return ESP8266_FAIL;
        if (!Usart2_RecvByte(&c, 5000) ||  c != 'D') return ESP8266_FAIL;
        if (!Usart2_RecvByte(&c, 5000) ||  c != ',') return ESP8266_FAIL;

    // ═══ 第 3 步：读数字直到 ':' ═══
        // 比如 "+IPD,512:" → 连续读 '5','1','2',':'
        // 转换成整数：0→5→51→512
        uint32_t data_len = 0;
        while (1) {
            if (!Usart2_RecvByte(&c, 5000))
                return ESP8266_FAIL;
            if (c == ':') break;              // 冒号表示长度结束
            if (c < '0' || c > '9') return ESP8266_FAIL;  // 非数字，异常
            data_len = data_len * 10 + (c - '0');
        }
        total_ipd_size += data_len; // 记录总大小，GetTotalSize 就能查
        ipd_remaining = data_len;  // 剩余字节数增加，等会收 data_len 字节数据
    }
// ═══ 第 4 步：收 data_len 字节数据 ═══
    // 从 ipd_remaining 里取最多 *len 字节给 caller
    uint32_t want = *len;
    uint32_t chunk_len = (ipd_remaining < want) ? ipd_remaining : want;
    *len = chunk_len;                      // 通过指针告诉调用者实际长度

    for (uint32_t i = 0; i < chunk_len; i++) {
        if (!Usart2_RecvByte(&buf[i], 5000)) // 每字节留足 UART 间隔
            return ESP8266_FAIL;
    }
    ipd_remaining -= chunk_len;
    return ESP8266_OK;
}

uint32_t ESP8266_GetTotalSize(void)
{
    return total_ipd_size;
}

void ESP8266_ClearSize(void)
{
    total_ipd_size = 0;
    ipd_remaining = 0;
}
