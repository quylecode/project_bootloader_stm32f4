/**
 * @file       bootloader_command.h
 * @copyright  Copyright (C) 2019 QuyLe Co., Ltd. All rights reserved.
 * @license    This project is released under the QuyLe License.
 * @version    1.0.0
 * @date       2023-08-16
 * @author     quy-itr-intern
 *
 * @brief      handle bootloader command
 *
 * @note
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef BOOTLOADER_COMAMD_H
#define BOOTLOADER_COMAMD_H

/* Includes ----------------------------------------------------------- */
#include "bsp_utility.h"
#include "main.h"
/* Public defines ----------------------------------------------------- */

/* Public enumerate/structure ----------------------------------------- */
enum
{
  START        = 0x55,
  COMMAND_READ = 0,
  COMMAND_WRITE,
  COMMAND_ERASE,
  COMMAND_JUMP,
  COMMAND_UPDATE,
};

typedef struct
{
  uint8_t  cmd;
  uint8_t  address[4];
  uint8_t  status_update;
  uint32_t size_flash;
  uint8_t  len;
  uint8_t  data[256];
  uint16_t crc;
} bootloader_command_data_t;

typedef enum
{
  bootloader_command_error,
  bootloader_comand_ok,
} bootloader_command_status_t;

typedef enum
{
  START_FRAME = 0U,
  CMD         = 1,
  CMD_READ    = 2U,
  CMD_WRITE   = 3U,
  CMD_EARSE   = 4U,
  CMD_JUMP    = 5U,
  CMD_UPDATE  = 6U,
  CHECK_CRC   = 7U,
  DONE        = 8U,
} bootloader_command_field_t;

/* Public macros ------------------------------------------------------ */

/* Public variables --------------------------------------------------- */

/* Public function prototypes ----------------------------------------- */
/**
 * @brief  <function description>
 *
 * @param[in]     <param_name>  <param_despcription>
 * @param[out]    <param_name>  <param_despcription>
 * @param[inout]  <param_name>  <param_despcription>
 *
 * @attention  <API attention note>
 *
 * @return
 *  - 0: Success
 *  - 1: Error
 */
bootloader_command_status_t bootloader_command_handle_data(bootloader_command_data_t *comand_data, uint8_t *data, uint16_t size);

#endif

/* Read:  55 01 08008000 FF FFAA */
/* Write: 55 02 08008000 04 AACCBBDD 00AA */
/* Erase: 55 03 08008000 00AA */
/* Jump : 55 03 08008000 00AA */

/* End of file -------------------------------------------------------- */
