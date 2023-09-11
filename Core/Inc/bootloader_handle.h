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

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef BOOTLOADER_HANDLE_H
#define BOOTLOADER_HANDLE_H

/* Includes ----------------------------------------------------------- */
#include "bsp_flash.h"
#include "bsp_uart.h"
#include "math.h"
#include "stdio.h"
#include "string.h"

/* Public defines ----------------------------------------------------- */

/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief field hex files
 */
typedef enum
{
  START_CODE  = 0U, /*  Start code 	  */
  BYTE_COUNT  = 1U, /*  Byte count 	  */
  ADDRESS     = 2U, /*  Address 		  */
  RECORD_TYPE = 3U, /*  Record type 	*/
  DATA        = 4U, /*  Data 			    */
  CHECKSUM    = 5U, /*  Checksum 		  */
  HEX_DONE    = 6U  /*  work is done 	*/
} bootloader_handle_field_hex_files_t;

/**
 * @brief status process
 */
typedef enum
{
  STATE_ERRORS = 0U,   /* There is a error */
  STATE_NO_ERRORS,     /* There is no errors */
  STATE_PENDING,       /* Working is pending  */
  STATE_ERRORS_TIMEOUT /* There is a error due to timeout */
} bootloader_handle_status_process_t;

/**
 * @brief struct hex form data
 */
typedef struct
{
  uint8_t byte_count;
  uint8_t address[2];
  uint8_t record_type;
  uint8_t data[16];
  uint8_t check_sum;
} bootloader_handle_hex_form_data_t;

/* Public macros ------------------------------------------------------ */

/* Public function prototypes ----------------------------------------- */

uint8_t bootloader_handle_write_flash_memory(bootloader_handle_hex_form_data_t *hex_data);
uint8_t bootloader_handle_data_receive(bootloader_handle_hex_form_data_t *hex_data, uint8_t *data, uint16_t size);

#endif

/* End of file -------------------------------------------------------- */
