/**
 * @file HDL_CAN.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-15
 * @last modified 2024-05-15
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "HDL_CAN.h"
#include "CPU_Define.h"
#include "ccommon.h"

/**********************   存储将要发送的数据8Byte  ****************************/
typedef struct _CAN_MSG_byte
{
    unsigned char data[8];
} CAN_MSG_byte;
typedef struct _CAN_MSG_BYTE
{
    unsigned char byte0;
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
    unsigned char byte4;
    unsigned char byte5;
    unsigned char byte6;
    unsigned char byte7;
} CAN_MSG_BYTE;
typedef union _CAN_MSG_DATA
{
    CAN_MSG_byte msg_byte;
    CAN_MSG_BYTE msg_Byte;
} CAN_MSG_DATA;

/**************************    将要发送的数据信息     **************************************/

typedef struct _CanTxMsg
{
    union
    {
        unsigned short int all;
        struct
        {
            unsigned short int StdId : 11;
            unsigned short int resved : 5;
        } bit;
    } StdId;                      // 标准帧ID
    unsigned char DLC;            // 数据长度，可为0到8;
    CAN_MSG_DATA CAN_Tx_msg_data; /*!< 帧消息内容,共8字节 */
} CanTxMsg;

/**************************    将要接收的数据信息     **************************************/

typedef struct _CanRxMsg
{
    union
    {
        unsigned short int all;
        struct
        {
            unsigned short int StdId : 11;
            unsigned short int resved : 5;
        } bit;
    } StdId;                      // 标准帧ID,值为0x000到0x7FFF;
    unsigned char DLC;            // 数据长度，可为0到8;
    CAN_MSG_DATA CAN_Rx_msg_data; /*!< 帧消息内容,共8字节 */
} CanRxMsg;

__interrupt void CANA_ISR(void);

/**
 * @brief CAN初始化。
 *
 */
void HDL_CAN_Init(HDL_CAN_t can)
{
    UNUSED(can);
    //
    // For this example, configure CAN pins using GPIO regs here
    // This function is found in DSP2833x_ECan.c
    //
    InitECanGpio();

    EALLOW;
    PieVectTable.ECAN1INTA = &CANA_ISR;
    EDIS;

    //
    // Initialize eCAN-A module
    //
    InitECana();

    //
    // Mailboxes can be written to 16-bits or 32-bits at a time
    // Write to the MSGID field of TRANSMIT mailboxes MBOX0 - 15
    //
    ECanaMboxes.MBOX0.MSGID.all = 0x9555AAA0;
    ECanaMboxes.MBOX1.MSGID.all = 0x9555AAA1;
    ECanaMboxes.MBOX2.MSGID.all = 0x9555AAA2;
    ECanaMboxes.MBOX3.MSGID.all = 0x9555AAA3;
    ECanaMboxes.MBOX4.MSGID.all = 0x9555AAA4;
    ECanaMboxes.MBOX5.MSGID.all = 0x9555AAA5;
    ECanaMboxes.MBOX6.MSGID.all = 0x9555AAA6;
    ECanaMboxes.MBOX7.MSGID.all = 0x9555AAA7;
    ECanaMboxes.MBOX8.MSGID.all = 0x9555AAA8;
    ECanaMboxes.MBOX9.MSGID.all = 0x9555AAA9;
    ECanaMboxes.MBOX10.MSGID.all = 0x9555AAAA;
    ECanaMboxes.MBOX11.MSGID.all = 0x9555AAAB;
    ECanaMboxes.MBOX12.MSGID.all = 0x9555AAAC;
    ECanaMboxes.MBOX13.MSGID.all = 0x9555AAAD;
    ECanaMboxes.MBOX14.MSGID.all = 0x9555AAAE;
    ECanaMboxes.MBOX15.MSGID.all = 0x9555AAAF;

    //
    // Write to the MSGID field of RECEIVE mailboxes MBOX16 - 31
    //
    ECanaMboxes.MBOX16.MSGID.all = 0x9555AAA0;
    ECanaMboxes.MBOX17.MSGID.all = 0x9555AAA1;
    ECanaMboxes.MBOX18.MSGID.all = 0x9555AAA2;
    ECanaMboxes.MBOX19.MSGID.all = 0x9555AAA3;
    ECanaMboxes.MBOX20.MSGID.all = 0x9555AAA4;
    ECanaMboxes.MBOX21.MSGID.all = 0x9555AAA5;
    ECanaMboxes.MBOX22.MSGID.all = 0x9555AAA6;
    ECanaMboxes.MBOX23.MSGID.all = 0x9555AAA7;
    ECanaMboxes.MBOX24.MSGID.all = 0x9555AAA8;
    ECanaMboxes.MBOX25.MSGID.all = 0x9555AAA9;
    ECanaMboxes.MBOX26.MSGID.all = 0x9555AAAA;
    ECanaMboxes.MBOX27.MSGID.all = 0x9555AAAB;
    ECanaMboxes.MBOX28.MSGID.all = 0x9555AAAC;
    ECanaMboxes.MBOX29.MSGID.all = 0x9555AAAD;
    ECanaMboxes.MBOX30.MSGID.all = 0x9555AAAE;
    ECanaMboxes.MBOX31.MSGID.all = 0x9555AAAF;

    //
    // Configure Mailboxes 0-15 as Tx, 16-31 as Rx
    // Since this write is to the entire register (instead of a bit field)
    // a shadow register is not required.
    //
    ECanaRegs.CANMD.all = 0xFFFF0000;

    //
    // Enable all Mailboxes
    // Since this write is to the entire register (instead of a bit field)
    // a shadow register is not required.
    //
    ECanaRegs.CANME.all = 0xFFFFFFFF;

    //
    // Specify that 8 bits will be sent/received
    //
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX6.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX7.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX8.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX9.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX10.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX11.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX12.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX13.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX14.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX15.MSGCTRL.bit.DLC = 8;

    //
    // Write to the mailbox RAM field of MBOX0 - 15
    //
    ECanaMboxes.MBOX0.MDL.all = 0x9555AAA0;
    ECanaMboxes.MBOX0.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX1.MDL.all = 0x9555AAA1;
    ECanaMboxes.MBOX1.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX2.MDL.all = 0x9555AAA2;
    ECanaMboxes.MBOX2.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX3.MDL.all = 0x9555AAA3;
    ECanaMboxes.MBOX3.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX4.MDL.all = 0x9555AAA4;
    ECanaMboxes.MBOX4.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX5.MDL.all = 0x9555AAA5;
    ECanaMboxes.MBOX5.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX6.MDL.all = 0x9555AAA6;
    ECanaMboxes.MBOX6.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX7.MDL.all = 0x9555AAA7;
    ECanaMboxes.MBOX7.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX8.MDL.all = 0x9555AAA8;
    ECanaMboxes.MBOX8.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX9.MDL.all = 0x9555AAA9;
    ECanaMboxes.MBOX9.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX10.MDL.all = 0x9555AAAA;
    ECanaMboxes.MBOX10.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX11.MDL.all = 0x9555AAAB;
    ECanaMboxes.MBOX11.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX12.MDL.all = 0x9555AAAC;
    ECanaMboxes.MBOX12.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX13.MDL.all = 0x9555AAAD;
    ECanaMboxes.MBOX13.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX14.MDL.all = 0x9555AAAE;
    ECanaMboxes.MBOX14.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX15.MDL.all = 0x9555AAAF;
    ECanaMboxes.MBOX15.MDH.all = 0x89ABCDEF;

    //
    // Since this write is to the entire register (instead of a bit field)
    // a shadow register is not required.
    //
    EALLOW;
    ECanaRegs.CANMIM.all = 0xFFFFFFFF;

    //
    // eCAN control registers require read/write access using 32-bits.  Thus we
    // will create a set of shadow registers for this example.  These shadow
    // registers will be used to make sure the access is 32-bits and not 16.
    //
    // 因为无法对CAN寄存器进行直接按位修改，此处借助一个影子寄存器来进行修改。
    struct ECAN_REGS ECanaShadow;

    ECanaShadow.CANME.all = ECanaRegs.CANME.all; // 保存旧有的寄存器值
    ECanaShadow.CANME.bit.ME1 = 0;               // 修改系统寄存器前需关闭要配置的邮箱，此处不使能邮箱1
    ECanaShadow.CANME.bit.ME2 = 0;               // 不使能邮箱2

    //
    // Configure the eCAN for self test mode
    // Enable the enhanced features of the eCAN.
    //
    EALLOW;
    // 将修改完毕的值写入系统寄存器内
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    EDIS;

    /*----------------------	对发送邮箱MailBox1进行配置	---------------------*/
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;       // 配置数据长度为8;
    ECanaMboxes.MBOX1.MSGID.bit.STDMSGID = 0x21; // 设置发送消息的ID；
    ECanaMboxes.MBOX1.MSGID.bit.AME = 0;         // 因为是发送邮箱所以屏蔽使能位,如果需要使用屏蔽,必须将该位置1
    ECanaMboxes.MBOX1.MSGID.bit.IDE = 0;         // 不使用扩展ID，使用11bit的标志ID（数据手册Page829）

    /*----------------------	对接收邮箱MailBox2进行配置	---------------------*/
    ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;       // 配置数据长度;
    ECanaMboxes.MBOX2.MSGID.bit.STDMSGID = 0x21; // 设置接收消息的有效ID
    ECanaMboxes.MBOX2.MSGID.bit.AME = 1;         // 接收消息ID过滤器使能
    ECanaMboxes.MBOX2.MSGID.bit.IDE = 0;         // 不使用扩展ID，使用11bit的标志ID（数据手册Page829）

    // ECanaLAMRegs.LAM2.bit.LAMI = 0;//当为0时只接受标准帧
    ECanaLAMRegs.LAM2.all = 0x00; // 设置只接收标准帧的ID=0x21消息

    ECanaRegs.CANRMP.all = 0xFFFFFFFF;

    ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
    ECanaShadow.CANMD.bit.MD1 = 0; // 配置邮箱1用于发送
    ECanaShadow.CANMD.bit.MD2 = 1; // 邮箱2用于接收
    ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;

    ECanaShadow.CANME.all = ECanaRegs.CANME.all; // 配置完毕使能邮箱1、2
    ECanaShadow.CANME.bit.ME1 = 1;               // 使能邮箱1
    ECanaShadow.CANME.bit.ME2 = 1;               // 使能邮箱2
    ECanaRegs.CANME.all = ECanaShadow.CANME.all;
    /*----------------------	对接收中断进行配置	---------------------*/
    EALLOW;
    ECanaRegs.CANMIM.bit.MIM2 = 1; // 使能中断邮箱2的中断;
    ECanaRegs.CANMIL.bit.MIL2 = 1; // 将中断1连接至中断INT9.6;
    ECanaRegs.CANGIM.bit.I1EN = 1; // 使能中断1;
    EDIS;

    // 下列两行最好写在main函数中，和其他中断一起使能。
    IER |= M_INT9;                     // 开启系统中断线
    PieCtrlRegs.PIEIER9.bit.INTx6 = 1; // 使能CAN线
}

/**
 * @brief CAN发送数据。
 *
 * @param can
 * @param _uiID
 * @param _uiData
 * @param _uiDataLen 数据长度，最大为8。
 */
void HDL_CAN_Send(HDL_CAN_t can, uint32_t _uiID, byte_t *_uiData, byte_t _uiDataLen);

void CAN_Send_Msg(CanTxMsg *can_tx_msg) // 发送一帧数据
{
    Uint16 time_cnt = 0;

    struct ECAN_REGS ECanaShadow;
    volatile struct MBOX *Mailbox = &ECanaMboxes.MBOX1;

    Mailbox->MSGID.bit.STDMSGID = can_tx_msg->StdId.bit.StdId; // 发送标准帧ID

    Mailbox->MSGCTRL.bit.DLC = can_tx_msg->DLC; // 数据长度

    Mailbox->MDL.byte.BYTE0 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte0;
    Mailbox->MDL.byte.BYTE1 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte1;
    Mailbox->MDL.byte.BYTE2 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte2;
    Mailbox->MDL.byte.BYTE3 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte3;
    Mailbox->MDH.byte.BYTE4 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte4;
    Mailbox->MDH.byte.BYTE5 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte5;
    Mailbox->MDH.byte.BYTE6 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte6;
    Mailbox->MDH.byte.BYTE7 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte7;

    ECanaShadow.CANTRS.all = 0;
    ECanaShadow.CANTRS.bit.TRS1 = 1;
    ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
    do
    {
        ECanaShadow.CANTA.all = ECanaRegs.CANTA.all;
        time_cnt++;
    } while (((ECanaShadow.CANTA.bit.TA1) == 0) && (time_cnt < 100));

    ECanaShadow.CANTA.all = 0;
    ECanaShadow.CANTA.bit.TA1 = 1;
    ECanaRegs.CANTA.all = ECanaShadow.CANTA.all;
}

CanRxMsg can_rx_msg; // 接收一帧数据
// 接收中断处理函数：
__interrupt void CANA_ISR(void)
{
    if ((ECanaRegs.CANGIF1.bit.GMIF1 == 1) && (ECanaRegs.CANRMP.bit.RMP2 == 1))
    {
        can_rx_msg.DLC = ECanaMboxes.MBOX2.MSGCTRL.bit.DLC;

        can_rx_msg.StdId.bit.StdId = ECanaMboxes.MBOX2.MSGID.bit.STDMSGID;

        can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte0 = ECanaMboxes.MBOX2.MDL.byte.BYTE0;
        can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte1 = ECanaMboxes.MBOX2.MDL.byte.BYTE1;
        can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte2 = ECanaMboxes.MBOX2.MDL.byte.BYTE2;
        can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte3 = ECanaMboxes.MBOX2.MDL.byte.BYTE3;
        can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte4 = ECanaMboxes.MBOX2.MDH.byte.BYTE4;
        can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte5 = ECanaMboxes.MBOX2.MDH.byte.BYTE5;
        can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte6 = ECanaMboxes.MBOX2.MDH.byte.BYTE6;
        can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte7 = ECanaMboxes.MBOX2.MDH.byte.BYTE7;

        ECanaRegs.CANRMP.bit.RMP2 = 1;
    }
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;
}
