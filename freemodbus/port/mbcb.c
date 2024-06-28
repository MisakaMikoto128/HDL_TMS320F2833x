#include "mb.h"
#include "mbcb.h"
#include "CPU_Define.h"
#include "ccommon.h"
#include "port.h"
#include <stdlib.h>

/* Private variables ---------------------------------------------------------*/
// 输入寄存器内容
uint16_t usRegInputBuf[REG_INPUT_NREGS] = {0};
// 输入寄存器起始地址
uint16_t usRegInputStart = REG_INPUT_START;

// 保持寄存器内容
uint16_t usRegHoldingBuf[REG_HOLDING_NREGS] = {0};
// 保持寄存器起始地址
uint16_t usRegHoldingStart = REG_HOLDING_START;
bool boolRegHoldingChanged = false;

// 线圈状态
byte_t ucRegCoilsBuf[REG_COILS_SIZE / 8] = {0};
// 开关输入状态
byte_t ucRegDiscreteBuf[REG_DISCRETE_SIZE / 8] = {0};

/**
 * @brief 保持寄存器是否发生变化
 *
 */
bool eMBRegHoldingChanged(void)
{
    return boolRegHoldingChanged;
}

#define eMBRegHoldingSetChanged() (boolRegHoldingChanged = true)
/**
 * @brief 清除保持寄存器变化标志。
 *
 */
void eMBRegHoldingClearChanged(void)
{
    boolRegHoldingChanged = false;
}

#include "B2_EventRecord.h"

/**
 * @brief 读取输入寄存器，对应功能码是 04 eMBFuncReadInputRegister.
 * @note 上位机发来的 帧格式是:
 * SlaveAddr(1 Byte)+FuncCode(1 Byte)
 * +StartAddrHiByte(1 Byte)+StartAddrLoByte(1 Byte)
 * +LenAddrHiByte(1 Byte)+LenAddrLoByte(1 Byte)+
 * +CRCAddrHiByte(1 Byte)+CRCAddrLoByte(1 Byte)
 * @param pucRegBuffer 数据缓存区，用于响应主机
 * @param usAddress 寄存器地址
 * @param usNRegs 要读取的寄存器个数
 * @return eMBErrorCode
 */
eMBErrorCode
eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
    eMBErrorCode eStatus = MB_ENOERR;
    int iRegIndex;

    if ((usAddress >= usRegInputStart) && (usAddress + usNRegs <= REG_INPUT_END + 1))
    {
        iRegIndex = (int)(usAddress - 1 - usRegInputStart);
        while (usNRegs > 0)
        {
            *pucRegBuffer++ = (UCHAR)(usRegInputBuf[iRegIndex] >> 8);
            *pucRegBuffer++ = (UCHAR)(usRegInputBuf[iRegIndex] & 0xFF);
            iRegIndex++;
            usNRegs--;
        }
    }
    else if ((usAddress >= REG_INPUT_START_SEC2) && (usAddress + usNRegs <= REG_INPUT_END_SEC2 + 1))
    {
        // iRegIndex = (int)(usAddress - 1 - usRegInputStart);
        uint32_t readSize = B2_EventRecord_Read_RwaData_Circular_Generic((byte_t*)pucRegBuffer, REG_INPUT_NREGS_SEC2 * 2);
        if (readSize == 0)
        {
            eStatus = MB_ENOREG;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

void SyncModbusRegToSysinfo();
/**
 * @brief 对应功能码有：
 * 06 写保持寄存器 eMBFuncWriteHoldingRegister
 * 16 写多个保持寄存器 eMBFuncWriteMultipleHoldingRegister
 * 03 读保持寄存器 eMBFuncReadHoldingRegister
 * 23 读写多个保持寄存器 eMBFuncReadWriteMultipleHoldingRegister
 *
 * @param pucRegBuffer 数据缓存区，用于响应主机
 * @param usAddress 寄存器地址
 * @param usNRegs 要读写的寄存器个数
 * @param eMode 功能码
 * @return eMBErrorCode
 */
eMBErrorCode
eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode eStatus = MB_ENOERR;
    int iRegIndex = 0;
    if ((usAddress >= REG_HOLDING_START) &&
        ((usAddress + usNRegs) <= (REG_HOLDING_END + 1)))
    {
        iRegIndex = (int)(usAddress - 1 - usRegHoldingStart);
        switch (eMode)
        {
        case MB_REG_READ: // 读 MB_REG_READ = 0
            while (usNRegs > 0)
            {
                *pucRegBuffer++ = (byte_t)(usRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (byte_t)(usRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;
        case MB_REG_WRITE: // 写 MB_REG_WRITE = 0
            while (usNRegs > 0)
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }

            SyncModbusRegToSysinfo();
            eMBRegHoldingSetChanged();
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

extern void xMBUtilSetBits(UCHAR *ucByteBuf, USHORT usBitOffset, UCHAR ucNBits,
                           UCHAR ucValue);
extern UCHAR xMBUtilGetBits(UCHAR *ucByteBuf, USHORT usBitOffset, UCHAR ucNBits);
/**
 * @brief 对应功能码有：01 读线圈 eMBFuncReadCoils
 * 05 写线圈 eMBFuncWriteCoil
 * 15 写多个线圈 eMBFuncWriteMultipleCoils
 *
 * @note 如继电器
 *
 * @param pucRegBuffer 数据缓存区，用于响应主机
 * @param usAddress 线圈地址
 * @param usNCoils 要读写的线圈个数
 * @param eMode 功能码
 * @return eMBErrorCode
 */
eMBErrorCode
eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils,
              eMBRegisterMode eMode)
{
    // 错误状态
    eMBErrorCode eStatus = MB_ENOERR;
    // 寄存器个数
    int16_t iNCoils = (int16_t)usNCoils;
    // 寄存器偏移量
    int16_t usBitOffset;

    // 检查寄存器是否在指定范围内
    if (((int16_t)usAddress >= REG_COILS_START) &&
        (usAddress + usNCoils <= REG_COILS_END + 1))
    {
        // 计算寄存器偏移量
        usBitOffset = (int16_t)(usAddress - 1 - REG_COILS_START);
        switch (eMode)
        {
            // 读操作
        case MB_REG_READ:
            while (iNCoils > 0)
            {
                *pucRegBuffer++ = xMBUtilGetBits((UCHAR *)ucRegCoilsBuf, usBitOffset,
                                                 (byte_t)(iNCoils > 8 ? 8 : iNCoils));
                iNCoils -= 8;
                usBitOffset += 8;
            }
            break;

            // 写操作
        case MB_REG_WRITE:
            while (iNCoils > 0)
            {
                xMBUtilSetBits((UCHAR *)ucRegCoilsBuf, usBitOffset,
                               (byte_t)(iNCoils > 8 ? 8 : iNCoils),
                               *pucRegBuffer++);
                iNCoils -= 8;
            }
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
 * @brief 读取离散寄存器，对应功能码有：
 *  02 读离散寄存器 eMBFuncReadDiscreteInputs
 *
 * @param pucRegBuffer 数据缓存区，用于响应主机
 *
 * @param usAddress 寄存器地址
 * @param usNDiscrete 要读取的寄存器个数
 * @return eMBErrorCode
 */
eMBErrorCode
eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete)
{
    // 错误状态
    eMBErrorCode eStatus = MB_ENOERR;
    // 操作寄存器个数
    int16_t iNDiscrete = (int16_t)usNDiscrete;
    // 偏移量
    uint16_t usBitOffset;

    // 判断寄存器时候再制定范围内
    if (((int16_t)usAddress >= REG_DISCRETE_START) &&
        (usAddress + usNDiscrete <= REG_DISCRETE_END + 1))
    {
        // 获得偏移量
        usBitOffset = (uint16_t)(usAddress - 1 - REG_DISCRETE_START);

        while (iNDiscrete > 0)
        {
            *pucRegBuffer++ = xMBUtilGetBits((UCHAR *)ucRegDiscreteBuf, usBitOffset,
                                             (byte_t)(iNDiscrete > 8 ? 8 : iNDiscrete));
            iNDiscrete -= 8;
            usBitOffset += 8;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
