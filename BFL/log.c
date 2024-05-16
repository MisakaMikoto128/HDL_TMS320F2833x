/**
 * @file log.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "log.h"
#include <stdio.h>
#include "HDL_Uart.h"

// 调试输出数据包最大长度
#define MAXDEBUGSEND 256
static char buffer[MAXDEBUGSEND + 1];
#define DEBUG_COM COM2

/**
 * @brief 使用串口调试的格式化输出方法
 *
 * @param format
 * @param ...
 */
void Debug_Printf(const void *format, ...)
{
    uint32_t uLen;
    va_list vArgs;
    va_start(vArgs, format);
    uLen = vsnprintf(buffer, MAXDEBUGSEND, (char const *)format, vArgs);
    va_end(vArgs);
    if (uLen > MAXDEBUGSEND)
        uLen = MAXDEBUGSEND;
    Uart_Write(DEBUG_COM, (uint16_t *)buffer, uLen);
}

