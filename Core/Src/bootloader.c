/**
 * @file       bootloader.c
 * @copyright  Copyright (C) 2019 QuyLe Co., Ltd. All rights reserved.
 * @license    This project is released under the QuyLe License.
 * @version    1.0.0
 * @date       2023-08-14
 * @author     quy-itr-intern
 *
 * @brief      handle bootloader
 *
 * @note
 */

/* Includes ----------------------------------------------------------- */
#include "bootloader.h"

/* Private defines ---------------------------------------------------- */
#define ADDRESS_FIRMWARE_APPLICATION 0x08008000U
#define STM32F446xx                  1

#ifdef STM32F401xx
uint32_t max_size_flash = 245760;
#elif STM32F446xx
uint32_t max_size_flash = 507904;
#endif
/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

/* Private variables -------------------------------------------------- */
uint8_t flag_ota_update      = FALSE;
uint8_t flag_size_flash      = FALSE;
uint8_t flag_ota_complete    = FALSE;
uint8_t flag_earse_ok        = FALSE;
uint8_t flag_ota_check_infor = FALSE;
/* Private function prototypes ---------------------------------------- */

/* Function definitions ----------------------------------------------- */

void bootloader_handle_command(bootloader_command_data_t comand_data)
{
  switch (comand_data.cmd)
  {
  case COMMAND_READ:
  {
    /* code */
    char tx[20];
    bsp_uart1_printf((uint8_t *) "\r\nCOMMAND_READ\r\n");
    uint32_t address_value = 0;
    uint32_t address       = (comand_data.address[0] << 24) | (comand_data.address[1] << 16) | (comand_data.address[2] << 8) | (comand_data.address[3]);

    sprintf(tx, "Address: %lx\r\n", address);
    bsp_uart1_printf((uint8_t *) tx);

    for (uint8_t i = 0; i < comand_data.len; i++)
    {
      address_value = *(uint32_t *) (address + i * 4);
      sprintf(tx, "Value %d: %lx\r\n", i, address_value);
      bsp_uart1_printf((uint8_t *) tx);
    }
    break;
  }
  case COMMAND_WRITE:
  {
    /* code */
    bsp_uart1_printf((uint8_t *) "\r\nCOMMAND_WRITE\r\n");
    uint32_t address      = (comand_data.address[0] << 24) | (comand_data.address[1] << 16) | (comand_data.address[2] << 8) | (comand_data.address[3]);
    bsp_flash_write(address, (uint32_t*) &comand_data.data[0], comand_data.len / 4);
    bsp_uart1_printf((uint8_t *) "write flash memory ok !!!\r\n");
    break;
  }
  case COMMAND_ERASE:
  {
    /* code */
    bsp_uart1_printf((uint8_t *) "\r\nCOMMAND_ERASE\r\n");
    uint32_t address      = (comand_data.address[0] << 24) | (comand_data.address[1] << 16) | (comand_data.address[2] << 8) | (comand_data.address[3]);
    uint8_t  sector_erase = bsp_get_sector_from_address(address);
    bsp_flash_erase(bsp_get_address_from_sector(sector_erase));
    bsp_uart1_printf((uint8_t *) "Erase sector okok !!!\r\n");
    break;
  }
  case COMMAND_JUMP:
  {
    /* code */
    /* COMMAND_JUMP */
    bsp_uart1_printf((uint8_t *) "\r\nCOMMAND_JUMP\r\n");
    bsp_uart1_printf((uint8_t *) "Jump to application ok !!!\r\n");
    flag_ota_complete = TRUE;
    break;
  }
  case COMMAND_UPDATE:
  {
    /* code */
    if (comand_data.status_update == 1)
    {
      /* Update firmware */
      flag_ota_update = TRUE;
      bsp_uart1_printf(BOOTLOADER_CONFIRM_CHAR);
    }
    else if (comand_data.status_update == 2)
    {
      /* Begin Update firmware */
      bsp_uart1_printf(BOOTLOADER_CONFIRM_CHAR);
    }
    else if (comand_data.status_update == 3)
    {
      /* End Update firmware */
      bsp_uart1_printf(BOOTLOADER_CONFIRM_CHAR);
    }
    else if (comand_data.status_update == 4)
    {
      /* Check size flash */
      if (comand_data.size_flash < max_size_flash)
      {
        flag_size_flash = TRUE;
      }

      if ((flag_ota_update == TRUE) && (flag_size_flash == TRUE))
      {
        flag_earse_ok = FALSE;
        /* caculation number sector */
        uint8_t start_sector  = bsp_get_sector_from_address(ADDRESS_FIRMWARE_APPLICATION);
        uint8_t end_sector    = bsp_get_sector_from_address(ADDRESS_FIRMWARE_APPLICATION + comand_data.size_flash);
        uint8_t number_sector = end_sector - start_sector + 1;

        /* erase sector for write data */
        for (uint8_t i = 0; i < number_sector; i++)
        {
          bsp_flash_erase(bsp_get_address_from_sector(start_sector));
          start_sector++;
        }
        flag_earse_ok = TRUE;
      }
      bsp_uart1_printf(BOOTLOADER_CONFIRM_CHAR);
    }
    else if (comand_data.status_update == 5)
    {
      /* Handle data hex */
      bootloader_handle_hex_form_data_t hex_data;
      if ((bootloader_handle_data_receive(&hex_data, &comand_data.data[0], comand_data.len) == STATE_NO_ERRORS) && flag_ota_update && flag_size_flash)
      {
        /* write data receive to flash memory */
        if (!bootloader_handle_write_flash_memory(&hex_data))
          /* address write memory errors */
          bsp_uart1_printf(BOOTLOADER_CANCEL_UPDATE);
        else
          /* data write successful */
          bsp_uart1_printf(BOOTLOADER_CONFIRM_CHAR);
      }
      else
        /* data receive miss or error */
        bsp_uart1_printf(BOOTLOADER_RESEND_DATA);
      break;
    }
    break;
  }
  default: break;
  }
}

void bootloader_start_up(void)
{
  bsp_uart_set_callback_bootloader_command(bootloader_handle_command);
  bsp_uart_init();

  bsp_uart1_printf((uint8_t*) "Bootloader stm32!!!");

  while (1)
  {
    if (flag_ota_complete == TRUE)
    {
      bsp_uart_deinit_peripheral();
      HAL_RCC_DeInit();

      SCB->SHCSR &= ~(SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk);
      __set_MSP(*((volatile uint32_t *) ADDRESS_FIRMWARE_APPLICATION));
      uint32_t jump_address       = *((volatile uint32_t *) (ADDRESS_FIRMWARE_APPLICATION + 4));
      void (*reset_handler)(void) = (void *) jump_address;
      reset_handler();
    }
    /* code */
  }
}

/* End of file -------------------------------------------------------- */
