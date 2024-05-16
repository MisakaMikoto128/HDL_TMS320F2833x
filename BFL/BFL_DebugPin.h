/**
* @file BFL_DebugPin.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-05-16
* @last modified 2024-05-16
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef BFL_DEBUGPIN_H
#define BFL_DEBUGPIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    DEBUG_PIN_1 = 0,
    DEBUG_PIN_2,
    DEBUG_PIN_NUM,
    DEBUG_PIN_NONE,
} BFL_DebugPin_t;

void BFL_DebugPin_Init();
void BFL_DebugPin_Set(BFL_DebugPin_t pin);
void BFL_DebugPin_Reset(BFL_DebugPin_t pin);
void BFL_DebugPin_Toggle(BFL_DebugPin_t pin);
#ifdef __cplusplus
}
#endif
#endif //!BFL_DEBUGPIN_H
