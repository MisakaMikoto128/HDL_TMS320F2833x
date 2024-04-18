/**
* @file APP_Main.c
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-04-18
* @last modified 2024-04-18
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#include "APP_Main.h"
#include "CPU_Define.h"
#include "BFL_Buzz.h"
#include "HDL_Uart.h"
void APP_Main_Init()
{
    BFL_Buzz_Init();
    //Uart_Init(COM2,115200,UART_WORD_LEN_8,UART_STOP_BIT_1,UART_PARITY_NONE);
     Uart_Init(COM3,115200,UART_WORD_LEN_8,UART_STOP_BIT_1,UART_PARITY_NONE);
}

void APP_Main_Poll()
{
    // BFL_Buzz_Toggle();
    while (ScicRegs.SCICTL2.bit.TXRDY == 0) {}
        ScicRegs.SCITXBUF='a';
    DELAY_US(100000);
}
