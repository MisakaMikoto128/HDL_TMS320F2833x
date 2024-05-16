/**
* @file log.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-05-16
* @last modified 2024-05-16
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

void Debug_Printf(const void *format, ...);
#ifdef __cplusplus
}
#endif
#endif //!LOG_H
