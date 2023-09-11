/**
 * @file       bootloader_handle.h
 * @copyright  Copyright (C) 2023 QuyLe Co., Ltd. All rights reserved.
 * @license    This project is released under the QuyLe License.
 * @version    1.0.0
 * @date       2023-08-14
 * @author     quyle-itr-intern
 *
 * @brief      handle data hex
 *
 * @note
 */

/* Includes ----------------------------------------------------------- */
#include "bootloader_handle.h"

#include <stdlib.h>

/* Private defines ---------------------------------------------------- */
#define USART_UD                     &huart1

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */
extern UART_HandleTypeDef huart1;

/* Private variables -------------------------------------------------- */
uint32_t          size_current;
volatile uint32_t size_data_firmware = 0;
volatile uint32_t address_write_flash;
/* Private function prototypes ---------------------------------------- */

/* Function definitions ----------------------------------------------- */
uint8_t bootloader_handle_write_flash_memory(bootloader_handle_hex_form_data_t *hex_data)
{
  uint8_t  data_count, i;
  uint8_t  counter_index       = 0;
  uint16_t data_write_flash[8] = { 0 };

  switch (hex_data->record_type)
  {
  case 4: /* Extended Linear Address Record: used to identify the extended linear address  */
  {
    uint32_t address = 0;
    address |= hex_data->data[0] << 8 | hex_data->data[1];
    /* Address save firmware update */
    address_write_flash = 0x08000000 | (address << 16);
    break;
  }
  case 5: /* Start Linear Address Record: the address where the program starts to run      */
  {
    break;
  }
  case 0: /* Data Rrecord: used to record data, most records of HEX files are data records */
  {
    /* get character count */
    data_count = hex_data->byte_count;
    size_data_firmware += data_count;

    /* set full address */
    address_write_flash = address_write_flash & 0xFFFF0000;
    address_write_flash = address_write_flash | (hex_data->address[0] << 8) | (hex_data->address[1]);

    /* check address wite flash data */
    if (address_write_flash < 0x08008000U)
    {
      address_write_flash = 0;
      return 0;
    }

    /* get the data of the record */
    for (i = 0; i < data_count; i += 2)
    {
      data_write_flash[counter_index] = (hex_data->data[i + 1] << 8) | (hex_data->data[i]);
      counter_index++;
    }

    /* count data old or even */
    if (data_count % 2 != 0)
    {
      data_write_flash[counter_index] = 0xFF00 | (hex_data->data[data_count - 1]);
      bsp_flash_write(address_write_flash, (uint32_t *) data_write_flash, data_count / 4 + 1);
    }
    else
      bsp_flash_write(address_write_flash, (uint32_t *) data_write_flash, data_count / 4);
    break;
  }
  default: break;
  }
  return 1;
}

void bootloader_handle_parse_element(uint8_t *des, uint8_t *buff, uint8_t length)
{
  uint8_t count_array = 0U;
  uint8_t buffer_temp[32];

  /* convert data string to data hex and save to des buffer*/
  for (uint8_t count = 0U; count < length; count++)
  {
    if (buff[count] >= '0' && buff[count] <= '9')
      buffer_temp[count] = buff[count] - 48;
    else if (buff[count] >= 'A' && buff[count] <= 'F')
      buffer_temp[count] = buff[count] - 65 + 10;
    else
      return;
    if (count % 2 != 0)
    {
      des[count_array] = ((buffer_temp[count - 1] << 4) | (buffer_temp[count]));
      count_array++;
    }
  }
}

uint8_t bootloader_handle_data_receive(bootloader_handle_hex_form_data_t *hex_data, uint8_t *data, uint16_t size)
{
  uint8_t                             cal_check_sum = 0;
  bootloader_handle_field_hex_files_t state         = START_CODE;
  uint8_t                             buffer_save_data_hex[16];

  /* parse string to hex form data hex */
  for (uint8_t count = 0; count < size; count++)
  {
    switch (state)
    {
    case START_CODE:
    {
      if (data[count] != ':')
        return STATE_ERRORS;
      else
        state = BYTE_COUNT;
      break;
    }
    case BYTE_COUNT:
    {
      bootloader_handle_parse_element(buffer_save_data_hex, &data[count], 2U);
      hex_data->byte_count = buffer_save_data_hex[0];
      count                = 2U;
      state                = ADDRESS;
      break;
    }
    case ADDRESS:
    {
      bootloader_handle_parse_element(buffer_save_data_hex, &data[count], 4U);
      hex_data->address[0] = buffer_save_data_hex[0];
      hex_data->address[1] = buffer_save_data_hex[1];
      count                = 6U;
      state                = RECORD_TYPE;
      break;
    }
    case RECORD_TYPE:
    {
      bootloader_handle_parse_element(buffer_save_data_hex, &data[count], 2U);
      hex_data->record_type = buffer_save_data_hex[0];
      count                 = 8U;
      state                 = DATA;
      break;
    }
    case DATA:
    {
      bootloader_handle_parse_element(buffer_save_data_hex, &data[count], (hex_data->byte_count * 2U));
      for (count = 0; count < hex_data->byte_count; count++)
      {
        hex_data->data[count] = buffer_save_data_hex[count];
      }
      count = 8U + (hex_data->byte_count * 2U);
      state = CHECKSUM;
      break;
    }
    case CHECKSUM:
    {
      bootloader_handle_parse_element(buffer_save_data_hex, &data[count], 2U);
      hex_data->check_sum = buffer_save_data_hex[0];
      state               = HEX_DONE;
      break;
    }
    case HEX_DONE:
    {
      /* check sum data hex */
      cal_check_sum = hex_data->byte_count + hex_data->address[0] + hex_data->address[1] + hex_data->record_type;
      for (count = 0; count < hex_data->byte_count; count++)
      {
        cal_check_sum += hex_data->data[count];
      }
      cal_check_sum = ~cal_check_sum + 1U;
      if (cal_check_sum == hex_data->check_sum)
        return STATE_NO_ERRORS;
      else
        return STATE_ERRORS;
    }
    default: return STATE_ERRORS;
    }
  }
  return STATE_ERRORS;
}

/* End of file -------------------------------------------------------- */
