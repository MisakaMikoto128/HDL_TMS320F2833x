/**
* @file cobject.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2023-09-13
* @last modified 2023-09-13
*
* @copyright Copyright (c) 2023 Liu Yuanlin Personal.
*
*/
#ifndef COBJECT_H
#define COBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void* CObject_t;

// 定义一个函数来返回结构体的大小
#define STRUCT_SIZE(type) (unsigned long)(&((type *)0)->end)

#ifdef __cplusplus
}
#endif
#endif //!COBJECT_H
