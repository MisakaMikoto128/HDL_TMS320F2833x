/**
* @file async_delay.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-05-17
* @last modified 2024-05-17
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef ASYNC_DELAY_H
#define ASYNC_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void async_delay(uint32_t ms, void (*callback)(void *), void *arg);
#ifdef __cplusplus
}
#endif
#endif //!ASYNC_DELAY_H
