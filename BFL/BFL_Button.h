/**
* @file BFL_Button.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-05-14
* @last modified 2024-05-14
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef BFL_BUTTON_H
#define BFL_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MODE_BTN = 0,
} BFL_Button_t;
void BFL_Button_Init();

bool BFL_Button_IsPressed(BFL_Button_t btn);
#ifdef __cplusplus
}
#endif
#endif //!BFL_BUTTON_H
