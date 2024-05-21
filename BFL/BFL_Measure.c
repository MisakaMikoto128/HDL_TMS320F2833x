/**
 * @file BFL_Measure.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-23
 * @last modified 2024-04-23
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "BFL_Measure.h"
#include "CPU_Define.h"
#include "DSP2833x_Device.h"
#include "average_filter.h"

#define USING_FFT 0
#define USING_RMSE 0

#if USING_FFT == 1
#include "dsp.h"
#include "fpu.h"
#include "fpu32/fpu_rfft.h"

//*****************************************************************************
// the defines
//*****************************************************************************
#define FFT_STAGES (7U)
#define FFT_SIZE (1 << FFT_STAGES)

//*****************************************************************************
// the globals
//*****************************************************************************
// The global pass, fail values
uint16_t pass = 0U, fail = 0U;
// The absolute error between the result and expected values
float tolerance = 1.0e-3;

// Object of the structure RFFT_F32_STRUCT
RFFT_F32_STRUCT rfft;
// Handle to the RFFT_F32_STRUCT object
RFFT_F32_STRUCT_Handle hnd_rfft = &rfft;

// Object of the structure RFFT_ADC_F32_STRUCT
RFFT_ADC_F32_STRUCT rfft_adc;
// Handle to the RFFT_ADC_F32_STRUCT object
RFFT_ADC_F32_STRUCT_Handle hnd_rfft_adc = &rfft_adc;

// #ifdef __cplusplus
// #pragma DATA_SECTION("FFT_buffer_2")
// #else
// #pragma DATA_SECTION(test_output, "FFT_buffer_2")
// #endif
float test_output[FFT_SIZE];
uint16_t test_input[FFT_SIZE];
float test_magnitude_phase[(FFT_SIZE >> 1) + 1];
float twiddleFactors[FFT_SIZE];
void FFT_Init();
#endif
//
// Defines for ADC start parameters
//
#if (CPU_FRQ_150MHZ) // Default - 150 MHz SYSCLKOUT
//
// HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 150/(2*3)   = 25.0 MHz
//
#define ADC_MODCLK 0x3
#endif
#if (CPU_FRQ_100MHZ)
//
// HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 100/(2*2)   = 25.0 MHz
//
#define ADC_MODCLK 0x2
#endif

//
// ADC module clock = HSPCLK/2*ADC_CKPS   = 25.0MHz/(1*2) = 12.5MHz
//
#define ADC_CKPS 0x1
#define ADC_SHCLK 0xf // S/H width in ADC module periods = 16 ADC clocks
#define AVG 10        // Average sample limit
#define ZOFFSET 0x00  // Average Zero offset

#define PIONTS_PER_GROUP 120 // 循环展开优化4层，必须是4的倍数，120/128
#define GROUP_NUM 8
#define BUF_SIZE (GROUP_NUM * PIONTS_PER_GROUP) // Sample buffer size

// 全波RMS
#if USING_RMSE == 1
static float AdcRMSE[GROUP_NUM];
#endif
static float AdcRMS[GROUP_NUM];
static float AdcAvg[GROUP_NUM];
#if USING_RMSE == 1
static float AdcVoltRMSE[GROUP_NUM];
#endif
static float AdcVoltRMS[GROUP_NUM];
static float AdcVoltAvg[GROUP_NUM];

static average_filter_t AdcVoltRMSFilter[GROUP_NUM];
static float AdcVoltRMSFilterBuf[GROUP_NUM][AVG];
static bool g_measure_ready = false;

//
// Globals
//
#pragma DATA_SECTION(DMABuf1, "DMARAML4");
volatile Uint16 DMABuf1[BUF_SIZE];

void config_ePWM4_to_generate_ADCSOCA(void);
void enable_ePWM4(void);

void config_ADC();
void config_DMA();
volatile Uint16 *DMADest;
volatile Uint16 *DMASource;
__interrupt void local_DINTCH1_ISR(void);

void BFL_Measure_Init()
{

  for (int i = 0; i < GROUP_NUM; i++)
  {
    average_filter_init(&AdcVoltRMSFilter[i], AdcVoltRMSFilterBuf[i], AVG);
  }

#if USING_FFT == 1
  FFT_Init();
#endif

  config_ADC();
  config_DMA();
  StartDMACH1();
  config_ePWM4_to_generate_ADCSOCA();
  enable_ePWM4();
}

bool BFL_Measure_ReadReady()
{
  bool ret = g_measure_ready;
  if (ret)
  {
    g_measure_ready = false;
  }
  return ret;
}

void BFL_Measure_Read(BFL_Measure_t *pMeasure)
{

  _disable_interrupts();
  for (int i = 0; i < GROUP_NUM; i++)
  {
    pMeasure->AdcVoltRMS[i] = AdcVoltRMS[i];
    pMeasure->AdcVoltAvg[i] = AdcVoltAvg[i];
    pMeasure->AdcVoltRMS_Filted[i] = AdcVoltRMSFilter[i].average;
    pMeasure->AdcVoltAvg_Filted[i] = AdcVoltRMSFilter[i].average;
  }
  _enable_interrupts();
}

//
// Defines that configure the period for each timer
//
#define EPWM4_TIMER_TBPRD \
  (75000000ULL / 2000UL - 1) // Period register 2kHz采样频率
#define EPWM4_MAX_CMPA 1950
#define EPWM4_MIN_CMPA 50
#define EPWM4_MAX_CMPB 1950
#define EPWM4_MIN_CMPB 50

//
// config_ePWM4_to_generate_ADCSOCA -
//
void config_ePWM4_to_generate_ADCSOCA_(void)
{
  //
  // Configure ePWM4 Timer
  // Interrupt triggers ADCSOCA
  //
  EALLOW;

  EPwm4Regs.ETSEL.bit.SOCAEN = 1;  // Enable SOC on A group
  EPwm4Regs.ETSEL.bit.SOCASEL = 4; // Select SOC on up-count
  EPwm4Regs.ETPS.bit.SOCAPRD = 1;  // Generate pulse on 1st event

  // PWM period = (TBPRD + 1 ) × TTBCLK Up-Count mode
  EPwm4Regs.TBPRD = EPWM4_TIMER_TBPRD; // Set EPwm1 Timer period （周期）

  // Freeze counter （冻结 ，不运行，配置为0则开始运行）
  EPwm4Regs.TBCTL.bit.CTRMODE = TB_FREEZE;

  //
  // Setup TBCLK
  //

  EPwm4Regs.TBPHS.half.TBPHS = 0x0000; // Phase is 0
  EPwm4Regs.TBCTR = 0x0000;            // Clear counter

  //
  // Setup counter mode TBCLK = SYSCLKOUT / (HSPCLKDIV × CLKDIV)
  //
  // TBCLK=SYSCLKOUT/(HSPCLKDIV*CLKDIV):150/(1*2)=75MHz
  EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV2;
  EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;

  //
  // Setup shadowing
  //
  EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
  EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
  EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // Load on Zero
  EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

  //
  // Interrupt where we will change the Compare Values
  //
  EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO; // Select INT on period event
  EPwm4Regs.ETSEL.bit.INTEN = 1;            // Enable INT
  EPwm4Regs.ETPS.bit.INTPRD = ET_1ST;       // Generate INT on every event

  EDIS;
}

void config_ePWM4_to_generate_ADCSOCA()
{
  //
  // For this example, only initialize the ePWM
  //
  EALLOW;
  /* Disable TBCLK within the ePWM要保证时基同步的话，
  首先在配置TB/CC寄存器时先把时钟关闭，即所有TBCLK停止，不产生。
  等全部配置后之后再打开，保证时钟同步
  */
  SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
  EDIS;

  config_ePWM4_to_generate_ADCSOCA_();

  EALLOW;
  SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
  EDIS;
}

void enable_ePWM4(void)
{
  EALLOW;
  __asm("   NOP");
  EPwm4Regs.TBCTL.bit.CTRMODE = 0; // Up count mode
  EPwm4Regs.ETSEL.bit.SOCAEN = 1;
  EDIS;
}

void config_ADC()
{

  //
  // Specific clock setting for this example
  //
  EALLOW;
  SysCtrlRegs.HISPCP.all = ADC_MODCLK; // HSPCLK = SYSCLKOUT/ADC_MODCLK
  EDIS;

  //
  // Step 4. Initialize all the Device Peripherals:
  // This function is found in DSP2833x_InitPeripherals.c
  //
  // InitPeripherals(); // Not required for this example
  InitAdc(); // For this example, init the ADC

  //
  // Specific ADC setup for this example:
  //
  AdcRegs.ADCTRL1.bit.ACQ_PS = ADC_SHCLK;
  AdcRegs.ADCTRL3.bit.ADCCLKPS = ADC_CKPS;
  AdcRegs.ADCTRL1.bit.SEQ_CASC = 0; // 0 Non-Cascaded Mode
  AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 0x1;
  AdcRegs.ADCTRL2.bit.RST_SEQ1 = 0x1;

  AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0;
  AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 0x1;
  AdcRegs.ADCCHSELSEQ1.bit.CONV02 = 0x2;
  AdcRegs.ADCCHSELSEQ1.bit.CONV03 = 0x3;
  AdcRegs.ADCCHSELSEQ2.bit.CONV04 = 0x4;
  AdcRegs.ADCCHSELSEQ2.bit.CONV05 = 0x5;
  AdcRegs.ADCCHSELSEQ2.bit.CONV06 = 0x6;
  AdcRegs.ADCCHSELSEQ2.bit.CONV07 = 0x7;

  //
  // Enable ADC to accept ePWM_SOCA trigger
  //
  AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = 1;

  //
  // Set up ADC to perform 4 conversions for every SOC
  //
  AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = GROUP_NUM - 1;
}

void config_DMA()
{
  //
  // Interrupts that are used in this example are re-mapped to
  // ISR functions found within this file.
  //
  EALLOW; // Allow access to EALLOW protected registers
  PieVectTable.DINTCH1 = &local_DINTCH1_ISR;
  EDIS; // Disable access to EALLOW protected registers

  IER |= M_INT7; // Enable INT7 (7.1 DMA Ch1)
  //
  // Initialize DMA
  //
  DMAInitialize();

  //
  // Clear Table
  //
  for (int i = 0; i < BUF_SIZE; i++)
  {
    DMABuf1[i] = 0;
  }

  //
  // Configure DMA Channel
  //

  //
  // Point DMA destination to the beginning of the array
  //
  DMADest = &DMABuf1[0];
  //
  // Point DMA source to ADC result register base
  //
  DMASource = &AdcMirror.ADCRESULT0;

  DMACH1AddrConfig(DMADest, DMASource);

  DMACH1BurstConfig(GROUP_NUM - 1, 1, PIONTS_PER_GROUP);
  DMACH1TransferConfig(PIONTS_PER_GROUP - 1, 0,
                       -((GROUP_NUM - 1) * PIONTS_PER_GROUP - 1));
  DMACH1WrapConfig(0, 0, PIONTS_PER_GROUP - 1, 0);
  DMACH1ModeConfig(DMA_SEQ1INT, PERINT_ENABLE, ONESHOT_DISABLE, CONT_ENABLE,
                   SYNC_DISABLE, SYNC_SRC, OVRFLOW_DISABLE, SIXTEEN_BIT,
                   CHINT_END, CHINT_ENABLE);
}

#include <math.h>
#include <stdint.h>
#include "BFL_DebugPin.h"
//
// local_DINTCH1_ISR - INT7.1(DMA Channel 1)
//
__interrupt void local_DINTCH1_ISR(void)
{
  // 测试当前中断中计算耗时2ms 优化类型5-speed 优化等级O0
  // 优化后548us 优化类型5-speed 优化等级O0
  // 数据预先取优化后493us 优化类型5-speed 优化等级O0
  // 2层循环展开后208us 优化类型5-speed 优化等级O0 无法使用,结果错误 写错了
  // 4层循环展开后92.4us 优化类型5-speed 优化等级O0 无法使用,结果错误 写错了
  // 4层循环展开后25.1us 优化类型5-speed 优化等级：整个程序优化
  // 无法使用,结果错误 写错了

  // 4层循环展开后405us 优化类型5-speed 优化等级O0
  BFL_DebugPin_Set(DEBUG_PIN_1);
  //
  // To receive more interrupts from this PIE group, acknowledge this
  // interrupt
  //
  PieCtrlRegs.PIEACK.all |= PIEACK_GROUP7;
  //
  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
  //
  // Calculate RMS
  for (uint32_t i = 0; i < GROUP_NUM; i++)
  {
    volatile uint16_t *pData = &DMABuf1[i * PIONTS_PER_GROUP];
    uint32_t sum = 0;
    uint32_t pow2_sum = 0;
    uint32_t data = 0;
    for (int j = 0; j < PIONTS_PER_GROUP; j += 4)
    {
      data = pData[j];
      sum += data;
      pow2_sum += (data * data);

      data = pData[j + 1];
      sum += data;
      pow2_sum += (data * data);

      data = pData[j + 2];
      sum += data;
      pow2_sum += (data * data);

      data = pData[j + 3];
      sum += data;
      pow2_sum += (data * data);
    }
    AdcAvg[i] = (sum * (1.0f / PIONTS_PER_GROUP));
    
    AdcRMS[i] = sqrtf((pow2_sum * (1.0f / PIONTS_PER_GROUP)));
    AdcVoltRMS[i] = (AdcRMS[i] * (3.0f / 4096));
    AdcVoltAvg[i] = ((AdcAvg[i] - ZOFFSET) * 3.0f / 4096);
    average_filter_update(&AdcVoltRMSFilter[i], AdcVoltRMS[i]);

#if USING_RMSE == 1
    uint32_t pow2_sum_e = 0;
    int32_t temp = 0;
    int32_t avgTemp = AdcAvg[i];
    for (int j = 0; j < PIONTS_PER_GROUP; j += 4)
    {
      temp = pData[j] - avgTemp;
      pow2_sum_e += temp * temp;

      temp = pData[j + 1] - avgTemp;
      pow2_sum_e += temp * temp;

      temp = pData[j + 2] - avgTemp;
      pow2_sum_e += temp * temp;

      temp = pData[j + 3] - avgTemp;
      pow2_sum_e += temp * temp;
    }
    AdcRMSE[i] = sqrtf((pow2_sum_e * 1.0f / PIONTS_PER_GROUP));
    AdcVoltRMSE[i] = (AdcRMSE[i] * 3.0f / 4096);
#endif
  }

#if USING_FFT == 1
  for (int i = 0; i < FFT_SIZE; i++)
  {
    test_input[i] = DMABuf1[i];
  }
  RFFT_adc_f32(hnd_rfft_adc); // Calculate real FFT with 12-bit
  RFFT_f32_mag(hnd_rfft);
#endif

  g_measure_ready = true;
  BFL_DebugPin_Reset(DEBUG_PIN_1);
}

#if USING_FFT == 1
void FFT_Init()
{

#if defined(_FLASH)
  //
  // Setup the FLASH Banks
  //
  DSP_Example_setupFlash();
#endif // defined(_FLASH)

  // Configure the object
  RFFT_f32_setInputPtr(hnd_rfft, (float *)test_input);
  RFFT_f32_setOutputPtr(hnd_rfft, test_output);
  RFFT_f32_setMagnitudePtr(hnd_rfft, test_magnitude_phase);
  RFFT_f32_setPhasePtr(hnd_rfft, test_magnitude_phase);
  RFFT_f32_setStages(hnd_rfft, FFT_STAGES);
  RFFT_f32_setFFTSize(hnd_rfft, FFT_SIZE);

  // Twiddle factor pointer
#ifdef USE_TABLES
  RFFT_f32_setTwiddlesPtr(hnd_rfft, RFFT_f32_twiddleFactors);
#else
  // Calculate twiddle factor
  RFFT_f32_setTwiddlesPtr(hnd_rfft, twiddleFactors);
  RFFT_f32_sincostable(hnd_rfft);
#endif // USE_TABLES

  // Link the RFFT_ADC_F32_STRUCT to RFFT_F32_STRUCT. Tail pointer
  // of RFFT_ADC_F32_STRUCT must point to the OutBuf pointer of
  // RFFT_F32_STRUCT
  RFFT_ADC_f32_setTailPtr(hnd_rfft_adc, &(hnd_rfft->OutBuf));
  RFFT_ADC_f32_setInBufPtr(hnd_rfft_adc, test_input);
}
#endif
//
// End of File
//
