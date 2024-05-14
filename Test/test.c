
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
