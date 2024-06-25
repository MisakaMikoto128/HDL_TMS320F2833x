/**
 * @file CHIP_W25Q128.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-27
 * @last modified 2024-04-27
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "CHIP_W25Q128.h"
#include "CPU_Define.h"
#include "HDL_SPI.h"
#include "HDL_CPU_Time.h"
#include <stddef.h>

int32_t w25q128_write_page_no_erase(uint32_t address, byte_t *buf, uint32_t size);

#define W25Q128_CHIP_ERASE_TIMEOUT_MAX 1000000U
// W25Q128默认的超时时长，单位ms。至少大于擦除一个扇区的时间50ms。
#define W25Q128_TIMEOUT_DEFAULT_VALUE 2000U
// W25Q128读取数据超时
#define W25Q128_RECEIVE_TIMEOUT 100U

// Command=0x05:  Read Status Register-1,返回寄存器SR1的值
uint16_t Flash_ReadSR1(void)
{
    uint16_t byte = 0;
    byte_t buf[2] = {0x05, 0xFF};
    byte_t buf_rev[2] = {0};
    HDL_SPI_CS_CLS();
    HDL_SPI_WriteRead(SPI_1, buf, buf_rev, 2, W25Q128_RECEIVE_TIMEOUT);
    HDL_SPI_CS_SET();
    byte = buf_rev[1];
    return byte;
}

// Command=0x35:  Read Status Register-2,返回寄存器SR2的值
uint16_t Flash_ReadSR2(void)
{
    uint16_t byte = 0;
    byte_t buf[2] = {0x35, 0xFF};
    byte_t buf_rev[2] = {0};
    HDL_SPI_CS_CLS();
    HDL_SPI_WriteRead(SPI_1, buf, buf_rev, 2, W25Q128_RECEIVE_TIMEOUT);
    HDL_SPI_CS_SET();
    byte = buf_rev[1];
    return byte;
}

// 检查寄存器SR1的BUSY位，直到BUSY位为0
uint32_t Flash_Wait_Busy(void)
{
    uint16_t SR1 = 0;
    uint32_t delay = 0;
    SR1 = Flash_ReadSR1(); // 读取状态寄存器SR1
    uint32_t start = HDL_CPU_Time_GetTick();
    while ((SR1 & 0x01) == 0x01)
    {
        if (HDL_CPU_Time_GetTick() - start > W25Q128_TIMEOUT_DEFAULT_VALUE)
        {
            return delay;
        }

        HDL_CPU_Time_DelayMs(1);
        delay++;
        SR1 = Flash_ReadSR1(); // 读取状态寄存器SR1
    }
    return delay;
}

/**
 * @brief Command=0x06: Write Enable,    使WEL=1
 *
 * @return true 读写成功。
 * @return false 读写失败。
 */
bool Flash_Write_Enable(void)
{
    byte_t buf[1] = {0x06};
    byte_t buf_rev[1] = {0};
    HDL_SPI_CS_CLS();
    HDL_SPI_WriteRead(SPI_1, buf, buf_rev, 1, W25Q128_RECEIVE_TIMEOUT);
    HDL_SPI_CS_SET();
    Flash_Wait_Busy(); // 等待操作完成
    return true;
}

uint16_t CHIP_W25Q128_Read_ID(void)
{
    uint16_t id = 0;

    // 发送0x90，读取厂商ID和设备ID
    byte_t buf[6] = {0x90, 0x00, 0x00, 0x00, 0xFF, 0xFF};
    byte_t buf_rev[6] = {0};
    HDL_SPI_CS_CLS();
    HDL_SPI_WriteRead(SPI_1, buf, buf_rev, 6, W25Q128_RECEIVE_TIMEOUT);
    HDL_SPI_CS_SET();
    // 随便发2个字节的数据
    id |= buf_rev[4] << 8; // id：0xEF17  厂商ID：0xEF
    id |= buf_rev[5];      // 设备ID：0x17

    return id;
}

int32_t CHIP_W25Q128_Init()
{
    HDL_SPI_Init(SPI_1, HDL_SPI_DATA_SIZE_8, HDL_SPI_CPOL_LOW, HDL_SPI_CPHA_2EDGE);
    return 0;
}

/**
 * @brief
 *
 * @param address
 * @param data
 * @param size
 * @return int32_t 实际读取的字节数
 */
int32_t CHIP_W25Q128_Read(uint32_t address, byte_t *data, uint32_t size)
{

    HDL_SPI_CS_CLS();
    byte_t buf[] = {0x03, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};
    byte_t buf_rev[sizeof(buf) / sizeof(buf[0])] = {0};
    HDL_SPI_WriteRead(SPI_1, buf, buf_rev, 4, W25Q128_RECEIVE_TIMEOUT);
    HDL_SPI_WriteRead(SPI_1, NULL, data, size, W25Q128_TIMEOUT_DEFAULT_VALUE);
    HDL_SPI_CS_SET();
    return 0;
}

static byte_t g_pageBuf[W25Q128_PAGE_SIZE] = {0};
/**
 * @brief 这个方法还不完善。
 *
 * @param address
 * @param data
 * @param size
 * @return int32_t
 */
int32_t CHIP_W25Q128_Write(uint32_t address, byte_t *data, uint32_t size)
{
    uint32_t sector = address / W25Q128_SECTOR_SIZE;
    int32_t ret = 0;
    if (sector >= W25Q128_SECTOR_COUNT)
    {
        return -1;
    }

    ret = CHIP_W25Q128_Erase_One_Sector(sector);

    uint32_t writePages = size / W25Q128_PAGE_SIZE;
    uint32_t remainBytes = size % W25Q128_PAGE_SIZE;
    for (uint32_t page_idx = 0; page_idx < writePages; page_idx++)
    {
        memcpy(g_pageBuf, data + page_idx * W25Q128_PAGE_SIZE, W25Q128_PAGE_SIZE);
        ret = w25q128_write_page_no_erase(sector * W25Q128_SECTOR_SIZE + page_idx * W25Q128_PAGE_SIZE, g_pageBuf, W25Q128_PAGE_SIZE);
    }

    if (remainBytes > 0)
    {
        memcpy(g_pageBuf, data + writePages * W25Q128_PAGE_SIZE, remainBytes);
        ret = w25q128_write_page_no_erase(sector * W25Q128_SECTOR_SIZE + writePages * W25Q128_PAGE_SIZE, g_pageBuf, remainBytes);
    }

    return ret;
}

/**
 * @brief The Sector Erase instruction sets all memory within
 * a specified sector (4K-bytes) to the erased state of all 1s (FFh).
 *
 * @param sector
 * @return int32_t 0: success, -1: fail
 */
int32_t CHIP_W25Q128_Erase_One_Sector(uint32_t sector)
{
    int32_t ret = 0;

    if (sector >= W25Q128_SECTOR_COUNT)
    {
        return -1;
    }

    uint32_t address = 0;
    address = sector * W25Q128_SECTOR_SIZE;

    Flash_Write_Enable(); // 使 WEL=1
    Flash_Wait_Busy();    // 等待空闲
    HDL_SPI_CS_CLS();
    byte_t buf[6] = {0x20, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};
    byte_t buf_rev[6] = {0};
    HDL_SPI_WriteRead(SPI_1, buf, buf_rev, 4, W25Q128_RECEIVE_TIMEOUT);
    HDL_SPI_CS_SET();
    Flash_Wait_Busy(); // 等待芯片擦除结束,大约25秒
    return ret;
}

/**
 * @brief W25Q128写入一个页的数据。_no_erase表示不进行擦除。
 * @note W25Q128有256字节的页缓冲区，写数据的起始地址可以是随机的，不需要考虑对齐，
 * 但是必须预先擦除，以确保被写入的字节单元写入数据前的值为0xFF。W25Q128一次最多写
 * 一页数据。
 *
 * @param buf 指向待写入数据的指针。
 * @param address 写入闪存的数据地址。
 * @param size 待写入数据的大小，单位字节。
 * @return int32_t 成功返回0，失败返回-1。
 */
int32_t w25q128_write_page_no_erase(uint32_t address, byte_t *buf, uint32_t size)
{
    int32_t status = 0;
    Flash_Write_Enable(); // SET WEL
    Flash_Wait_Busy();

    if(size > W25Q128_PAGE_SIZE)
    {
        size = W25Q128_PAGE_SIZE;
    }

    HDL_SPI_CS_CLS(); // CS=0
    byte_t buf1[] = {0x02, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};
    byte_t buf_rev[sizeof(buf1) / sizeof(buf1)] = {0};
    HDL_SPI_WriteRead(SPI_1, buf1, buf_rev, 4, W25Q128_RECEIVE_TIMEOUT);
    HDL_SPI_WriteRead(SPI_1, buf, NULL, size, W25Q128_TIMEOUT_DEFAULT_VALUE);
    HDL_SPI_CS_SET();  // CS=1
    Flash_Wait_Busy(); // 耗时大约3ms

    return status;
}

int32_t CHIP_W25Q128_Write_One_Sector_No_Erase(uint32_t sector, byte_t *buf)
{
    int32_t status = 0;

    for (uint32_t page_idx = 0; page_idx < W25Q128_SECTOR_SIZE / W25Q128_PAGE_SIZE; page_idx++)
    {
        status = w25q128_write_page_no_erase(sector * W25Q128_SECTOR_SIZE + page_idx * W25Q128_PAGE_SIZE, buf + page_idx * W25Q128_PAGE_SIZE, W25Q128_PAGE_SIZE);
    }
    return status;
}

/**
 * @brief 向指定扇区写入一个扇区大小的数据，会擦除原来的数据。
 *
 * @param sector 扇区编号。
 * @param buf 指向待写入数据的指针,其中至少包含一个W25Q512_SECTOR_SIZE大小的数据
 * @return int32_t 成功返回0，失败返回-1。
 */
int32_t CHIP_W25q128_Write_One_Sector(uint32_t sector, byte_t *buf)
{
    int32_t status = 0;
    CHIP_W25Q128_Erase_One_Sector(sector);
    status = CHIP_W25Q128_Write_One_Sector_No_Erase(sector, buf);
    return status;
}
