/**
* @file HDL_IWDG.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-04-20
* @last modified 2024-04-20
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef HDL_IWDG_H
#define HDL_IWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void HDL_IWDG_Init(uint32_t timeout_ms);

void HDL_IWDG_Feed(void);
#ifdef __cplusplus
}
#endif
#endif //!HDL_IWDG_H
