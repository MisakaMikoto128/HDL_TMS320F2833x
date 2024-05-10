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
#define AVG 1000      // Average sample limit
#define ZOFFSET 0x00  // Average Zero offset

#define PIONTS_PER_GROUP 128
#define GROUP_NUM 8
#define BUF_SIZE (GROUP_NUM * PIONTS_PER_GROUP) // Sample buffer size

// 全波RMS
float AdcRMSE[GROUP_NUM];
float AdcRMS[GROUP_NUM];
float AdcAvg[GROUP_NUM];

float AdcVoltRMSE[GROUP_NUM];
float AdcVoltRMS[GROUP_NUM];
float AdcVoltAvg[GROUP_NUM];
// 半波RMS
float AdcHalfWaveRMS[GROUP_NUM];

//
// Globals
//
#pragma DATA_SECTION(DMABuf1, "DMARAML4");
volatile Uint16 DMABuf1[BUF_SIZE];

void config_ePWM1_to_generate_ADCSOCA(void);
void enable_ePWM1(void);
void config_ePWM2_to_generate_ADCSOCB(void);

void config_ADC();
void config_DMA();
volatile Uint16 *DMADest;
volatile Uint16 *DMASource;
__interrupt void local_DINTCH1_ISR(void);

void BFL_Measure_Init()
{
  config_ADC();
  config_DMA();
  StartDMACH1();
  config_ePWM1_to_generate_ADCSOCA();
  enable_ePWM1();
}

//
// Defines that configure the period for each timer
//
#define EPWM1_TIMER_TBPRD (75000000ULL / 2000UL - 1) // Period register
#define EPWM1_MAX_CMPA 1950
#define EPWM1_MIN_CMPA 50
#define EPWM1_MAX_CMPB 1950
#define EPWM1_MIN_CMPB 50

//
// config_ePWM1_to_generate_ADCSOCA -
//
void config_ePWM1_to_generate_ADCSOCA_(void)
{
  //
  // Configure ePWM1 Timer
  // Interrupt triggers ADCSOCA
  //
  EALLOW;

  EPwm1Regs.ETSEL.bit.SOCAEN = 1;  // Enable SOC on A group
  EPwm1Regs.ETSEL.bit.SOCASEL = 4; // Select SOC on up-count
  EPwm1Regs.ETPS.bit.SOCAPRD = 1;  // Generate pulse on 1st event

  // PWM period = (TBPRD + 1 ) × TTBCLK Up-Count mode
  EPwm1Regs.TBPRD = EPWM1_TIMER_TBPRD; // Set EPwm1 Timer period （周期）

  // Freeze counter （冻结 ，不运行，配置为0则开始运行）
  EPwm1Regs.TBCTL.bit.CTRMODE = TB_FREEZE;

  //
  // Setup TBCLK
  //

  EPwm1Regs.TBPHS.half.TBPHS = 0x0000; // Phase is 0
  EPwm1Regs.TBCTR = 0x0000;            // Clear counter

  //
  // Setup counter mode TBCLK = SYSCLKOUT / (HSPCLKDIV × CLKDIV)
  //
  // TBCLK=SYSCLKOUT/(HSPCLKDIV*CLKDIV):150/(1*2)=75MHz
  EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV2;
  EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;

  //
  // Setup shadowing
  //
  EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
  EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
  EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // Load on Zero
  EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

  //
  // Interrupt where we will change the Compare Values
  //
  EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO; // Select INT on period event
  EPwm1Regs.ETSEL.bit.INTEN = 1;            // Enable INT
  EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;       // Generate INT on every event

  EDIS;
}

void config_ePWM1_to_generate_ADCSOCA()
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

  config_ePWM1_to_generate_ADCSOCA_();

  EALLOW;
  SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
  EDIS;
}

void enable_ePWM1(void)
{
  EALLOW;
  __asm("   NOP");
  EPwm1Regs.TBCTL.bit.CTRMODE = 0; // Up count mode
  EPwm1Regs.ETSEL.bit.SOCAEN = 1;
  EDIS;
}

//
// config_ePWM2_to_generate_ADCSOCB -
//
void config_ePWM2_to_generate_ADCSOCB(void)
{
  //
  // Configure ePWM2 Timer
  // Interrupt triggers ADCSOCB
  //
  EALLOW;
  EPwm2Regs.TBPRD = 150; // Setup periodSetup period
  EPwm2Regs.CMPA.all = 0x200000;
  EPwm2Regs.ETSEL.bit.SOCBSEL = 2;   // ADCSOCB on TBCTR=TBPRD
  EPwm2Regs.ETPS.bit.SOCBPRD = 1;    // Generate SOCB on 1st event
  EPwm2Regs.ETSEL.bit.SOCBEN = 1;    // Enable SOCB generation
  EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0; // /1 clock mode
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
  DMACH1TransferConfig(PIONTS_PER_GROUP - 1, 0, -((GROUP_NUM - 1) * PIONTS_PER_GROUP - 1));
  DMACH1WrapConfig(0, 0, PIONTS_PER_GROUP - 1, 0);
  DMACH1ModeConfig(DMA_SEQ1INT, PERINT_ENABLE, ONESHOT_DISABLE, CONT_ENABLE,
                   SYNC_DISABLE, SYNC_SRC, OVRFLOW_DISABLE, SIXTEEN_BIT,
                   CHINT_END, CHINT_ENABLE);
}

#include <math.h>
#include <stdint.h>
//
// local_DINTCH1_ISR - INT7.1(DMA Channel 1)
//
__interrupt void local_DINTCH1_ISR(void)
{
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
    for (uint32_t j = 0; j < PIONTS_PER_GROUP; j++)
    {
      sum += pData[j];
    }
    AdcAvg[i] = sum * 1.0f / PIONTS_PER_GROUP;

    uint32_t pow2_sum_e = 0;
    uint32_t pow2_sum = 0;
    for (uint32_t j = 0; j < PIONTS_PER_GROUP; j++)
    {
      pow2_sum_e += powf(pData[j] - AdcAvg[i], 2);
      pow2_sum += powf(pData[j], 2);
    }
    AdcRMSE[i] = sqrtf(pow2_sum_e * 1.0f / PIONTS_PER_GROUP);
    AdcRMS[i] = sqrtf(pow2_sum * 1.0f / PIONTS_PER_GROUP);

    AdcVoltAvg[i] = (AdcAvg[i] - ZOFFSET) * 3.0f / 4096;
    AdcVoltRMSE[i] = AdcRMSE[i] * 3.0f / 4096;
    AdcVoltRMS[i] = AdcRMS[i] * 3.0f / 4096;
  }
}

//
// End of File
//
