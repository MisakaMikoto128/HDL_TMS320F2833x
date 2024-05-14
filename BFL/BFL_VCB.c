/**
 * @file BFL_VCB.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-13
 * @last modified 2024-05-13
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "BFL_VCB.h"
#include "CPU_Define.h"
#include "ccommon.h"

const char *BFL_VCB_SW_To_String(BFL_VCB_SW_t vcb)
{
  switch (vcb)
  {
  case QF_SW:
    return "QF_SW";
  case QS1_SW:
    return "QS1_SW";
  case QS2_SW:
    return "QS2_SW";
  case KM1_SW:
    return "KM1_SW";
  case RES_SW:
    return "RES_SW";
  }
  return "Unknow";
}

const char *BFL_VCB_STATE_To_String(BFL_VCB_STATE_t state)
{
  switch (state)
  {
  case BFL_VCB_Opened:
    return "Opened";
  case BFL_VCB_Closed:
    return "Closed";
  case BFL_VCB_Unknow:
    return "Unknow";
  }
  return "Unknow";
}

const char *BFL_VCB_CTRL_STATE_To_String(BFL_VCB_CTRL_STATE_t state)
{
  switch (state)
  {
  case BFL_VCB_No_Ctrl:
    return "No_Ctrl";
  case BFL_VCB_Ctrl_Open:
    return "Ctrl_Open";
  case BFL_VCB_Ctrl_Close:
    return "Ctrl_Close";
  }
  return "Unknow";
}

struct BFL_BFL_VCB_t
{
  BFL_VCB_STATE_t setting_state;           // VCB设置完成的状态
  BFL_VCB_CTRL_STATE_t setting_ctrl_state; // VCB控制继电器设置完成的状态
};

struct BFL_BFL_VCB_t g_vcb_list[VCB_SW_NUM];

#define JK1_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO64 = 1
#define JK1_SET() GpioDataRegs.GPCSET.bit.GPIO64 = 1
#define JK1_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO64 = 1
#define JK1_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO64)

#define JK2_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO65 = 1
#define JK2_SET() GpioDataRegs.GPCSET.bit.GPIO65 = 1
#define JK2_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO65 = 1
#define JK2_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO65)

#define JK3_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO66 = 1
#define JK3_SET() GpioDataRegs.GPCSET.bit.GPIO66 = 1
#define JK3_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO66 = 1
#define JK3_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO66)

#define JK4_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO67 = 1
#define JK4_SET() GpioDataRegs.GPCSET.bit.GPIO67 = 1
#define JK4_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO67 = 1
#define JK4_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO67)

#define JK5_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO68 = 1
#define JK5_SET() GpioDataRegs.GPCSET.bit.GPIO68 = 1
#define JK5_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO68 = 1
#define JK5_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO68)

#define JK6_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO69 = 1
#define JK6_SET() GpioDataRegs.GPCSET.bit.GPIO69 = 1
#define JK6_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO69 = 1
#define JK6_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO69)

#define JK7_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO70 = 1
#define JK7_SET() GpioDataRegs.GPCSET.bit.GPIO70 = 1
#define JK7_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO70 = 1
#define JK7_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO70)

#define JK8_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO71 = 1
#define JK8_SET() GpioDataRegs.GPCSET.bit.GPIO71 = 1
#define JK8_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO71 = 1
#define JK8_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO71)

#define JK9_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO72 = 1
#define JK9_SET() GpioDataRegs.GPCSET.bit.GPIO72 = 1
#define JK9_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO72 = 1
#define JK9_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO72)

#define JK10_CLEAR() GpioDataRegs.GPCCLEAR.bit.GPIO73 = 1
#define JK10_SET() GpioDataRegs.GPCSET.bit.GPIO73 = 1
#define JK10_TOGGLE() GpioDataRegs.GPCTOGGLE.bit.GPIO73 = 1
#define JK10_IsSet() (GpioDataRegs.GPCDAT.bit.GPIO73)

#define XIN1_IsSet() (GpioDataRegs.GPBDAT.bit.GPIO40)
#define XIN2_IsSet() (GpioDataRegs.GPBDAT.bit.GPIO41)
#define XIN3_IsSet() (GpioDataRegs.GPBDAT.bit.GPIO42)
#define XIN4_IsSet() (GpioDataRegs.GPBDAT.bit.GPIO43)

#include "HDL_CPU_Time.h"
/**
 * @brief 初始化真空断路控制继电器，使其处于无控制的状态。
 *
 */
void BFL_VCB_Seurity_Init()
{
  for_Each_VCB_SW_t(vcb)
  {
    BFL_VCB_Relay_Set_As_Switch_No_Ctrl(vcb);
  }

  EALLOW;
  // VCB控制信号外部电路上无上下拉
  //  General purpose I/O
  GpioCtrlRegs.GPCMUX1.bit.GPIO64 = 0x00;
  // Configures the GPIO pin as an output
  GpioCtrlRegs.GPCDIR.bit.GPIO64 = 1;
  // Enable the internal pullup on the specified pin.
  GpioCtrlRegs.GPCPUD.bit.GPIO64 = 0;

  // General purpose I/O
  GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 0x00;
  // Configures the GPIO pin as an output
  GpioCtrlRegs.GPCDIR.bit.GPIO65 = 1;
  // Enable the internal pullup on the specified pin.
  GpioCtrlRegs.GPCPUD.bit.GPIO65 = 0;

  GpioCtrlRegs.GPCMUX1.bit.GPIO66 = 0x00;
  GpioCtrlRegs.GPCDIR.bit.GPIO66 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO66 = 0;

  GpioCtrlRegs.GPCMUX1.bit.GPIO67 = 0x00;
  GpioCtrlRegs.GPCDIR.bit.GPIO67 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO67 = 0;

  GpioCtrlRegs.GPCMUX1.bit.GPIO68 = 0x00;
  GpioCtrlRegs.GPCDIR.bit.GPIO68 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO68 = 0;

  GpioCtrlRegs.GPCMUX1.bit.GPIO69 = 0x00;
  GpioCtrlRegs.GPCDIR.bit.GPIO69 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO69 = 0;

  GpioCtrlRegs.GPCMUX1.bit.GPIO70 = 0x00;
  GpioCtrlRegs.GPCDIR.bit.GPIO70 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO70 = 0;

  GpioCtrlRegs.GPCMUX1.bit.GPIO71 = 0x00;
  GpioCtrlRegs.GPCDIR.bit.GPIO71 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO71 = 0;

  GpioCtrlRegs.GPCMUX1.bit.GPIO72 = 0x00;
  GpioCtrlRegs.GPCDIR.bit.GPIO72 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO72 = 0;

  GpioCtrlRegs.GPCMUX1.bit.GPIO73 = 0x00;
  GpioCtrlRegs.GPCDIR.bit.GPIO73 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO73 = 0;
  EDIS;

  for_Each_VCB_SW_t(vcb)
  {
    g_vcb_list[vcb].setting_state = BFL_VCB_Unknow;
    BFL_VCB_Relay_Set_As_Switch_No_Ctrl(vcb);
  }

  EALLOW;
  // Specifies the sampling period for pins GPIO40 to GPIO47 with Sampling Period = TSYSCLKOUT
  GpioCtrlRegs.GPBCTRL.bit.QUALPRD1 = 0x00;

  // IO输入口电路上被下拉
  // General purpose I/O
  GpioCtrlRegs.GPBMUX1.bit.GPIO43 = 0x00;
  // Configures the GPIO pin as an input
  GpioCtrlRegs.GPBDIR.bit.GPIO43 = 0;
  // Disable the internal pullup on the specified pin.
  GpioCtrlRegs.GPBPUD.bit.GPIO43 = 1;
  // Qualification using 3 samples
  GpioCtrlRegs.GPBQSEL1.bit.GPIO43 = 0x01;

  GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 0x00;
  GpioCtrlRegs.GPBDIR.bit.GPIO42 = 0;
  GpioCtrlRegs.GPBPUD.bit.GPIO42 = 1;
  GpioCtrlRegs.GPBQSEL1.bit.GPIO42 = 0x01;

  GpioCtrlRegs.GPBMUX1.bit.GPIO41 = 0x00;
  GpioCtrlRegs.GPBDIR.bit.GPIO41 = 0;
  GpioCtrlRegs.GPBPUD.bit.GPIO41 = 1;
  GpioCtrlRegs.GPBQSEL1.bit.GPIO41 = 0x01;

  GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0x00;
  GpioCtrlRegs.GPBDIR.bit.GPIO40 = 0;
  GpioCtrlRegs.GPBPUD.bit.GPIO40 = 1;
  GpioCtrlRegs.GPBQSEL1.bit.GPIO40 = 0x01;
  EDIS;
}

/**
 * @brief 设置真空断路器的控制继电器为使真空断路器断开状态。
 *
 * @param vcb
 */
void BFL_VCB_Relay_Set_As_Switch_Opened(BFL_VCB_SW_t vcb)
{
  switch (vcb)
  {
  case QF_SW:
    // 先使得需要处于无控制状态的继电器处于无控制状态
    JK1_SET();
    // 然后再控制需要闭合的继电器闭合
    JK2_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Open;
    break;
  case QS1_SW:
    JK3_SET();
    JK4_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Open;
    break;
  case QS2_SW:
    JK5_SET();
    JK6_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Open;
    break;
  case KM1_SW:
    JK7_SET();
    JK8_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Open;
    break;
  case RES_SW:
    JK9_SET();
    JK10_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Open;
    break;
  default:
    break;
  }
}

/**
 * @brief 设置真空断路器的控制继电器为使真空断路器闭合状态。
 *
 * @param vcb
 */
void BFL_VCB_Relay_Set_As_Switch_Closed(BFL_VCB_SW_t vcb)
{
  switch (vcb)
  {
  case QF_SW:
    JK2_SET();
    JK1_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Close;
    break;
  case QS1_SW:
    JK4_SET();
    JK3_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Close;
    break;
  case QS2_SW:
    JK6_SET();
    JK5_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Close;
    break;
  case KM1_SW:
    JK8_SET();
    JK7_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Close;
    break;
  case RES_SW:
    JK10_SET();
    JK9_CLEAR();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_Ctrl_Close;
    break;
  default:
    break;
  }
}

/**
 * @brief 设置真空断路器的控制继电器为使真空断路器处于无控制状态。
 *
 * @param vcb
 */
void BFL_VCB_Relay_Set_As_Switch_No_Ctrl(BFL_VCB_SW_t vcb)
{
  switch (vcb)
  {
  case QF_SW:
    JK1_SET();
    JK2_SET();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_No_Ctrl;
    break;
  case QS1_SW:
    JK3_SET();
    JK4_SET();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_No_Ctrl;
    break;
  case QS2_SW:
    JK5_SET();
    JK6_SET();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_No_Ctrl;
    break;
  case KM1_SW:
    JK7_SET();
    JK8_SET();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_No_Ctrl;
    break;
  case RES_SW:
    JK9_SET();
    JK10_SET();
    g_vcb_list[vcb].setting_ctrl_state = BFL_VCB_No_Ctrl;
    break;
  default:
    break;
  }
}

/**
 * @brief 获取真空断路器的控制继电器的已经执行完成的状态。
 *
 * @param vcb
 * @return BFL_VCB_STATE_t
 */
BFL_VCB_CTRL_STATE_t BFL_VCB_Relay_Get_Setting_State(BFL_VCB_SW_t vcb)
{
  if (vcb >= VCB_SW_NUM)
  {
    return BFL_VCB_No_Ctrl;
  }
  return g_vcb_list[vcb].setting_ctrl_state;
}

/**
 * @brief 设置真空断路器为使真空断路器闭合。
 *
 * @param vcb
 */
void BFL_VCB_Set_As_Switch_Closed(BFL_VCB_SW_t vcb)
{
  if (vcb >= VCB_SW_NUM)
  {
    return;
  }

  BFL_VCB_Relay_Set_As_Switch_Closed(vcb);
  g_vcb_list[vcb].setting_state = BFL_VCB_Closed;
}

/**
 * @brief 设置真空断路器为使真空断路器断开。
 *
 * @param vcb
 */
void BFL_VCB_Set_As_Switch_Opened(BFL_VCB_SW_t vcb)
{
  if (vcb >= VCB_SW_NUM)
  {
    return;
  }

  BFL_VCB_Relay_Set_As_Switch_Opened(vcb);
  g_vcb_list[vcb].setting_state = BFL_VCB_Opened;
}

/**
 * @brief 设置真空断路器为使真空断路器处于无控制状态。
 *
 * @param vcb
 */
void BFL_VCB_Set_As_Switch_No_Ctrl(BFL_VCB_SW_t vcb)
{
  if (vcb >= VCB_SW_NUM)
  {
    return;
  }

  BFL_VCB_Relay_Set_As_Switch_No_Ctrl(vcb);
}

/**
 * @brief 获取真空断路器的已经执行完成的设置状态。
 *
 * @param vcb
 * @return BFL_VCB_STATE_t
 */
BFL_VCB_STATE_t BFL_VCB_Get_Setting_State(BFL_VCB_SW_t vcb)
{
  if (vcb >= VCB_SW_NUM)
  {
    return BFL_VCB_Unknow;
  }
  return g_vcb_list[vcb].setting_state;
}

/**
 * @brief 获取真空断路器的实际状态。
 *
 * @param vcb
 * @return BFL_VCB_STATE_t
 */
BFL_VCB_STATE_t BFL_VCB_Get_Actual_State(BFL_VCB_SW_t vcb)
{
  BFL_VCB_STATE_t state = BFL_VCB_Unknow;
  // TODO:闭合电平这里假设为高电平
  switch (vcb)
  {
  case QF_SW:
    state = XIN1_IsSet() ? BFL_VCB_Closed : BFL_VCB_Opened;
    break;
  case QS1_SW:
    state = XIN2_IsSet() ? BFL_VCB_Closed : BFL_VCB_Opened;
    break;
  case QS2_SW:
    state = XIN3_IsSet() ? BFL_VCB_Closed : BFL_VCB_Opened;
    break;
  case KM1_SW:
    state = XIN4_IsSet() ? BFL_VCB_Closed : BFL_VCB_Opened;
    break;
  case RES_SW:
    break;
  default:
    break;
  }
  return state;
}
