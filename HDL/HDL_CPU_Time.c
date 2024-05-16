/**
 * @file HDL_CPU_TIme.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-26
 * @last modified 2024-04-26
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "HDL_CPU_TIme.h"
#include "CPU_Define.h"
#include <limits.h>
#include <stddef.h>

//
// Function Prototype statements
//
__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void cpu_timer2_isr(void);

static CPU_Time_Callback_t _gCPUTickCallback =
    NULL; // CPU tick定时器的每次中断回调的函数指针。
/* 保存 TIM定时中断到后执行的回调函数指针 */
static CPU_Time_Callback_t s_TIM_CallBack1;
static bool s_TIM1Busy = false;
static bool g_TimerInited = false;
//
// ConfigCpuTimer - This function initializes the selected timer to the period
// specified by the "Freq" and "Period" parameters. The "Freq" is entered as
// "MHz" and the period in "uSeconds". The timer is held in the stopped state
// after configuration.
//
static void ConfigTheCpuTimer(struct CPUTIMER_VARS *Timer, uint16_t TDDR,
                              uint16_t PSC, Uint32 PRD, int enableInt)
{
  //
  // Initialize timer period
  //
  Timer->CPUFreqInMHz = 150;
  Timer->PeriodInUSec = PRD + 1;

  //
  // Set pre-scale counter to divide by Freq (SYSCLKOUT)
  //
  Timer->RegsAddr->TPRH.bit.PSCH = PSC >> 8; // 1MHz
  Timer->RegsAddr->TPRH.bit.TDDRH = TDDR >> 8;

  Timer->RegsAddr->TPR.bit.PSC = PSC; // 1MHz
  Timer->RegsAddr->TPR.bit.TDDR = TDDR;

  // Ftimer = SYSCLKOUT / (PRD + 1) / (TPR + 1)
  //
  // Counter decrements PRD+1 times each period
  //
  Timer->RegsAddr->PRD.all = PRD;

  //
  // Initialize timer control register
  //

  //
  // 1 = Stop timer, 0 = Start/Restart Timer
  //
  Timer->RegsAddr->TCR.bit.TSS = 1;

  Timer->RegsAddr->TCR.bit.TRB = 1; // 1 = reload timer
  Timer->RegsAddr->TCR.bit.SOFT = 1;
  Timer->RegsAddr->TCR.bit.FREE = 1; // Timer Free Run

  //
  // 0 = Disable/ 1 = Enable Timer Interrupt
  //
  Timer->RegsAddr->TCR.bit.TIE = enableInt > 0 ? 1 : 0;

  //
  // Reset interrupt counter
  //
  // Timer->InterruptCount = 0;
}

void HDL_CPU_Time_Init()
{

  if (g_TimerInited)
  {
    return;
  }
  //
  // Interrupts that are used in this example are re-mapped to
  // ISR functions found within this file.
  //
  EALLOW; // This is needed to write to EALLOW protected registers
  PieVectTable.TINT0 = &cpu_timer0_isr;
  PieVectTable.XINT13 = &cpu_timer1_isr;
  PieVectTable.TINT2 = &cpu_timer2_isr;
  EDIS; // This is needed to disable write to EALLOW protected registers

  //
  // Step 4. Initialize the Device Peripheral. This function can be
  //         found in DSP2833x_CpuTimers.c
  //
  InitCpuTimers(); // For this example, only initialize the Cpu Timers

  CpuTimer0.InterruptCount = 0;
  CpuTimer1.InterruptCount = 0;
  CpuTimer2.InterruptCount = 0;

#if (CPU_FRQ_150MHZ)
  //
  // Configure CPU-Timer 0, 1, and 2 to interrupt every second:
  // 150MHz CPU Freq, 1 second Period (in uSeconds)
  //
  ConfigTheCpuTimer(&CpuTimer0, 150 - 1, 0, 1000 - 1, 1);     // 1ms period
  ConfigTheCpuTimer(&CpuTimer1, 150 - 1, 0, 0xFFFFFFFFUL, 0); // 1us
  ConfigTheCpuTimer(&CpuTimer2, 150 - 1, 0, 1000 - 1, 0);
#endif

#if (CPU_FRQ_100MHZ)
  //
  // Configure CPU-Timer 0, 1, and 2 to interrupt every second:
  // 100MHz CPU Freq, 1 second Period (in uSeconds)
  //
  ConfigTheCpuTimer(&CpuTimer0, 100 - 1, 0, 1000, 1); // 1ms period
  ConfigTheCpuTimer(&CpuTimer1, 100 - 1, 0, 1, 0);    // 1us
  ConfigTheCpuTimer(&CpuTimer2, 100 - 1, 0, 1000000, 0);
#endif

  //
  // To ensure precise timing, use write-only instructions to write to the
  // entire register. Therefore, if any of the configuration bits are changed
  // in ConfigCpuTimer and InitCpuTimers (in DSP2833x_CpuTimers.h), the
  // below settings must also be updated.
  //
  // To start or restart the CPU-timer, set TSS to 0. At reset, TSS is cleared
  // to 0 and the CPU-timer immediately starts
  CpuTimer0Regs.TCR.bit.TSS = 0; // write-only instruction to set TSS bit = 0
  CpuTimer1Regs.TCR.bit.TSS = 0; // write-only instruction to set TSS bit = 0
  CpuTimer2Regs.TCR.bit.TSS = 0; // write-only instruction to set TSS bit = 0

  //
  // Step 5. User specific code, enable interrupts
  //

  //
  // Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
  // which is connected to CPU-Timer 1, and CPU int 14, which is connected
  // to CPU-Timer 2:
  //
  IER |= M_INT1;  // Timer0 (TIMER 0)INT1.7
  IER |= M_INT13; // External Interrupt 13 (XINT13) or CPUTime
  IER |= M_INT14; // CPU-Timer2 (for TI/RTOS use)

  //
  // Enable TINT0 in the PIE: Group 1 interrupt 7
  //
  PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

  g_TimerInited = true;
}

uint32_t HDL_CPU_Time_GetTick()
{
  uint32_t count;

  // 禁止中断
  _disable_interrupts();
  //
  // 0 = Disable/ 1 = Enable Timer Interrupt
  //
//   CpuTimer0.RegsAddr->TCR.bit.TIE = 0; 
  //TODO:这样做可以提高性能，避免影响其他的中断，也可以在中断中调用，因为只是获取值，而不是等待
  //但是实际在其他的中断中循环调用是否会造成问题还有待考证，还有就是

  // 读取计数器值
  count = CpuTimer0.InterruptCount;

  // 恢复中断
  _enable_interrupts();
//   CpuTimer0.RegsAddr->TCR.bit.TIE = 1;

  return count;
}

void HDL_CPU_Time_ResetTick() { CpuTimer0.InterruptCount = 0; }

uint32_t HDL_CPU_Time_GetUsTick()
{
  uint32_t ret = 0xFFFFFFFFUL - CpuTimer1.RegsAddr->TIM.all;
  return ret;
}

void HDL_CPU_Time_ResetUsTick()
{
  //
  // Reload all counter register with period value
  //
  CpuTimer1.RegsAddr->TCR.bit.TRB = 1;
}

void HDL_CPU_Time_SetCPUTickCallback(CPU_Time_Callback_t _pCallBack);

/**
 * @brief
 * 微妙演示函数，使用硬件定时器的寄存器，需要注意的是Debug模式下即使停止在断点，这个
 * 定时器还是在运行。另外需要确定所使用的定时器计数寄存器的位宽，课参考@US_TIMER_BITWIDE，移
 * 植程序时需要修改这个宏定义。
 *
 * @param DelayUs
 */
void HDL_CPU_Time_DelayUs(UsTimer_t DelayUs)
{
  UsTimer_t tickstart = HDL_CPU_Time_GetUsTick();
  UsTimer_t wait = DelayUs;

  while ((HDL_CPU_Time_GetUsTick() - tickstart) < wait)
  {
  }
}

/**
 * @brief This function provides minimum delay (in millisecond) based
 *       on variable incremented.
 *  @note
 * 这个函数在中断中使用时必须保证调用这个函数的中断优先级低于CPU毫秒定时器中断的优先级。
 * @param DelayMs
 */
void HDL_CPU_Time_DelayMs(uint32_t DelayMs)
{
  uint32_t tickstart = HDL_CPU_Time_GetTick();
  uint32_t wait = DelayMs;

  while ((HDL_CPU_Time_GetTick() - tickstart) < wait)
  {
  }
}

/**
 * @brief This function handles CPU_US_TIM global interrupt.设置CPU
 * tick定时器的每次中断回调 的函数。
 * @retval None
 */
void HDL_CPU_Time_SetCPUTickCallback(CPU_Time_Callback_t _pCallBack)
{
  _gCPUTickCallback = _pCallBack;
}

/**
 * @brief 使用TIM2-5做单次定时器使用,
 * 定时时间到后执行回调函数。可以同时启动4个定时器通道，互不干扰。
 *          定时精度正负1us （主要耗费在调用本函数的执行时间）
 *          TIM2和TIM5 是32位定时器。定时范围很大
 *          TIM3和TIM4 是16位定时器。
 *
 * @param _CC : 捕获比较通道几，1
 * @param _uiTimeOut : 超时时间, 单位 1us. 对于16位定时器，最大 65.5ms;
 * 对于32位定时器，最大 4294秒
 * @param _pCallBack : 定时时间到后，被执行的函数
 * @param _pArg : 定时时间到后，被执行的函数所需要参数的地址。
 * @retval true : 启动成功, false : 定时器忙
 */
bool HDL_CPU_Time_StartHardTimer(uint16_t _CC, UsTimer_t _uiTimeOut,
                                 void *_pCallBack)
{

  if (s_TIM1Busy)
  {
    return false;
  }

  if (_CC == 1)
  {
    s_TIM_CallBack1 = (CPU_Time_Callback_t)_pCallBack;

    s_TIM1Busy = true;

#if (CPU_FRQ_150MHZ)
    ConfigTheCpuTimer(&CpuTimer2, 150 - 1, 0, _uiTimeOut - 1, 1);
    CpuTimer2Regs.TCR.bit.TSS = 0; // write-only instruction to set TSS bit = 0
#endif
#if (CPU_FRQ_100MHZ)
    ConfigTheCpuTimer(&CpuTimer2, 100 - 1, 0, _uiTimeOut - 1, 0);
#endif
  }

  return true;
}

/**
 * @brief 关闭硬件定时器。实际上就是关中断。
 * 如果定时器已经执行完，那么不会改变任何寄存器。
 * 如果定时器正在执行，且关闭的时刻不再临界条件，那么会关闭中断。
 * 如果刚好CCx中断标志置位但是还没有进入中断，且此时清楚了CCx中断标志,那么会进入中断
 * 但是却不能执行回调函数。
 *
 * 这个函数主要还是用于启动定时器后不需要了，要在定时中间关闭，不打算定时器去执行回调函数的情况。
 * @param _CC : 捕获比较通道几，1
 */
void HDL_CPU_Time_StopHardTimer(uint16_t _CC)
{
  if (_CC == 1)
  {
    CpuTimer2.RegsAddr->TCR.bit.TSS = 1;
  }
}

//
// cpu_timer0_isr -
//
__interrupt void cpu_timer0_isr(void)
{
  CpuTimer0.InterruptCount++;
  if (_gCPUTickCallback != NULL)
  {
    _gCPUTickCallback();
  }
  //
  // Acknowledge this interrupt to receive more interrupts from group 1
  //
  PieCtrlRegs.PIEACK.all |= PIEACK_GROUP1;
}

//
// cpu_timer1_isr -
//
__interrupt void cpu_timer1_isr(void)
{
  CpuTimer1.InterruptCount++;

  //
  // The CPU acknowledges the interrupt.
  //
  EDIS;
}

//
// cpu_timer2_isr -
//
__interrupt void cpu_timer2_isr(void)
{
  EALLOW;
  CpuTimer2.InterruptCount++;

  // Disable this interrupt
  CpuTimer2.RegsAddr->TCR.bit.TIE = 0;

  /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
  if (s_TIM_CallBack1 != NULL)
  {
    s_TIM_CallBack1();
  }

  s_TIM1Busy = false;

  //
  // The CPU acknowledges the interrupt.
  //
  EDIS;
}
