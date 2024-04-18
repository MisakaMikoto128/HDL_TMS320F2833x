/**
 * @file HDL_Uart.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-18
 * @last modified 2024-04-18
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef HDL_UART_H
#define HDL_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

    // 串口号定义
    typedef enum
    {
        COM1 = 0, // USART1
        COM2 = 1, // USART3
        COM_NUM,
    } COMID_t;

#ifdef AA
        typedef void (*UartWriteOverCallback_t)(void *args);
    
#endif


#ifdef __cplusplus
}
#endif
#endif //! HDL_UART_H
