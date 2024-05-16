
/*
byte_t flash_sector_buf[W25Q128_SECTOR_SIZE] = {0};
void CHIP_W25Q128_Test() {
  // Debug_Printf("111\r\n");
  uint16_t id = CHIP_W25Q128_Read_ID();
  Debug_Printf("CHIP_W25Q128_Read_ID:%x\r\n", id);

  for (int i = 0; i < W25Q128_SECTOR_SIZE; i++) {
    flash_sector_buf[i] = i & 0xFF;
  }

  CHIP_W25q128_Write_One_Sector(1, flash_sector_buf);

  for (int i = 0; i < W25Q128_SECTOR_SIZE; i++) {
    flash_sector_buf[i] = 0;
  }

  CHIP_W25Q128_Read(W25Q128_SECTOR_SIZE * 1, flash_sector_buf,
                    W25Q128_SECTOR_SIZE);

  int cnt = 0;
  for (int i = 0; i < W25Q128_SECTOR_SIZE; i++) {
    if (flash_sector_buf[i] != (i & 0xFF)) {
      cnt++;
    }
  }

  if (cnt == 0) {
    Debug_Printf("CHIP_W25q128_Write_One_Sector success\r\n");
  } else {
    Debug_Printf("CHIP_W25q128_Write_One_Sector fail, cnt:%d\r\n", cnt);
  }

  cnt = 0;
}
*/
// BFL_Buzz_Toggle();

  // for_Each_VCB_SW_t(vcb)
  // {
  //   if (BFL_VCB_Get_Setting_State(vcb) == BFL_VCB_Opened)
  //   {
  //     BFL_VCB_Set_As_Switch_Closed(vcb);
  //   }
  //   else if (BFL_VCB_Get_Setting_State(vcb) == BFL_VCB_Closed)
  //   {
  //     BFL_VCB_Set_As_Switch_Opened(vcb);
  //   }
  //   else
  //   {
  //     BFL_VCB_Set_As_Switch_Closed(vcb);
  //   }
  // }

  //   {

  //     HDL_CPU_Time_DelayMs(1000);

  //     for_Each_VCB_SW_t(vcb)
  //     {
  //       BFL_VCB_STATE_t state = BFL_VCB_Get_Actual_State(vcb);
  //       BFL_VCB_STATE_t settingState = BFL_VCB_Get_Setting_State(vcb);
  //       Debug_Printf("VCB:%s, fb: %s, set:%s\n", BFL_VCB_SW_To_String(vcb),
  //                    BFL_VCB_STATE_To_String(state),
  //                    BFL_VCB_STATE_To_String(settingState));
  //     }

  //     if (BFL_Button_IsPressed(MODE_BTN))
  //     {
  //       if (g_app_main_stack.modeBtnPressed == false)
  //       {
  //         g_app_main_stack.modeBtnPressed = true;
  //         Debug_Printf("MODE_BTN is pressed\n");
  //       }
  //     }
  //     else
  //     {
  //       if (g_app_main_stack.modeBtnPressed == true)
  //       {
  //         g_app_main_stack.modeBtnPressed = false;
  //         Debug_Printf("MODE_BTN is released\n");
  //       }
  //     }

  //     Debug_Printf("Hello World\n");
  //     HDL_CPU_Time_StartHardTimer(1, 1500000U, timer_callback);

  //     HDL_CPU_Time_DelayMs(1000);
  //     Debug_Printf("bb Hello World\n");
  //   }

  // {
  //   HDL_CPU_Time_DelayMs(1000ULL);
  //   // BFL_SCRT_Pluse_Transmit(SCRT_ALL, 4, 2000);

  //   // signal = BFL_SCRR_Have_Signal(SCRR_ALL);

  //   RS485C_Release_Bus();
  //   RS485A_Release_Bus();

  //   // RS485C_Take_Bus();
  //   // RS485A_Take_Bus();

  //   Uint32 tickA = HDL_CPU_Time_GetUsTick();

  //   Uart_Write(COM1, (const uint16_t *)"123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789",
  //              cnt);
  //   uint32_t readLen = Uart_Read(COM2, (uint16_t *)buffer, sizeof(buffer));
  //   if (readLen > 0)
  //   {
  //     Uart_Write(COM2, (const uint16_t *)buffer, readLen);
  //   }

  //   // for (uint32_t i = 0; i < cnt; i++) {
  //   //   //            cnt);
  //   //   SciaRegs.SCITXBUF = 'a';
  //   //   // 在此做判断，如果发送FIFO缓冲中数据  >=
  //   //   // 16字节，要等待下直到FIFO小于16才能再次向FIFO中存数据
  //   //   while (SciaRegs.SCICTL2.bit.TXRDY == 0) {
  //   //   }
  //   // }
  //   // //  TX FIFO Interrupt Enable
  //   // SciaRegs.SCIFFTX.bit.TXFFIENA = 1;

  //   Uint32 tickB = HDL_CPU_Time_GetUsTick();

  //   t = (tickB - tickA);
  //   // Uart_Write(COM2, (const uint16_t *)"123456789123456789", 18);
  //   // Uart_Write(COM3, (const uint16_t *)"123456789123456789", 18);
  // }
