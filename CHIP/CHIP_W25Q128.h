/**
 * @file CHIP_W25Q128.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-27
 * @last modified 2024-04-27
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef CHIP_W25Q128_H
#define CHIP_W25Q128_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"
#define W25Q128_CHIP_ID 0xEF17
#define W25Q128_FLASH_SIZE 0x1000000UL // 128Mbit,单位字节
#define W25Q128_PAGE_SIZE 256UL
#define W25Q128_SECTOR_SIZE 4096UL
#define W25Q128_SECTOR_COUNT (W25Q128_FLASH_SIZE / W25Q128_SECTOR_SIZE) // 16384UL

    int32_t CHIP_W25Q128_Init();
    uint16_t CHIP_W25Q128_Read_ID(void);
    int32_t CHIP_W25Q128_Read(uint32_t address, byte_t *data, uint32_t size);
    int32_t CHIP_W25Q128_Write(uint32_t address, byte_t *data, uint32_t size);
    int32_t CHIP_W25q128_Write_One_Sector(uint32_t sector, byte_t *buf);
    int32_t CHIP_W25Q128_Erase_One_Sector(uint32_t sector);
    int32_t CHIP_W25q128_Write_One_Sector_No_Erase(uint32_t sector, byte_t *buf);
#ifdef __cplusplus
}
#endif
#endif //! CHIP_W25Q128_H
