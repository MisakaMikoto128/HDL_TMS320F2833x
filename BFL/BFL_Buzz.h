/**
 * @file BFL_Buzz.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-18
 * @last modified 2024-04-18
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef BFL_BUZZ_H
#define BFL_BUZZ_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "HDL_GPIO.h"
    void BFL_Buzz_Off();

    void BFL_Buzz_Toggle();

    void BFL_Buzz_On();

    void BFL_Buzz_Init();

    bool BFL_Buzz_IsOff();

    void BFL_Buzz_SetTrigLevel(HDL_GPIO_PinState_t level);
#ifdef __cplusplus
}
#endif
#endif //! BFL_BUZZ_H
