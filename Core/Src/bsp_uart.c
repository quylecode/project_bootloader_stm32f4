/**
 * @file       bsp_uart.c
 * @copyright  Copyright (C) 2023 QuyLe Co., Ltd. All rights reserved.
 * @license    This project is released under the QuyLe License.
 * @version    1.0.0
 * @date       2023-08-14
 * @author     quyle-itr-intern
 *
 * @brief     bsp uart
 *
 * @note
 */

/* Includes ----------------------------------------------------------- */
#include "bsp_uart.h"

#include <string.h>
/* Private defines ---------------------------------------------------- */
#define DEBUG 1
/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef  hdma_usart1_rx;
uint8_t                   g_flag_cplt_dma = FALSE;

/* Private variables -------------------------------------------------- */
#define RX_BUFFER_SIZE 2000
uint8_t                      data_receive_dma[RX_BUFFER_SIZE];
uint8_t                      rx_buffer[RX_BUFFER_SIZE];
uint8_t                     *buffer_save_data_handle;
bsp_uart_handle_rx_data_t    bsp_uart_handle_rx_data_callback;
bsp_uart_bootloader_comand_t bsp_uart_bootloader_comand_callback;

/* Private function prototypes ---------------------------------------- */
void bsp_uart_set_callback_handle_rx_data(void *cb)
{
  bsp_uart_handle_rx_data_callback = cb;
}

void bsp_uart_set_callback_bootloader_command(void *cb)
{
  bsp_uart_bootloader_comand_callback = cb;
}

/* Function definitions ----------------------------------------------- */
void bsp_uart_init(void)
{
  bsp_uart_set_callback_handle_rx_data(bootloader_command_handle_data);
  /* Init UART */
  bsp_uart_dma_unregister_callback(&hdma_usart1_rx, HAL_DMA_XFER_HALFCPLT_CB_ID);
  /* receive data uart dma */
  bsp_uart_receive_to_idle_dma(&huart1, data_receive_dma, RX_BUFFER_SIZE);
  buffer_save_data_handle = rx_buffer;
}

void bsp_uart_printf(UART_HandleTypeDef *huart, uint8_t *string)
{
  HAL_UART_Transmit(huart, string, strlen((char *) string), TIME_OUT_TRANSMIT_UART);
}

void bsp_uart1_printf(uint8_t *string)
{
  HAL_UART_Transmit(&huart1, string, strlen((char *) string), TIME_OUT_TRANSMIT_UART);
}

void bsp_uart_printf_len(UART_HandleTypeDef *huart, uint8_t *string, uint16_t len)
{
  HAL_UART_Transmit(huart, string, len, TIME_OUT_TRANSMIT_UART);
}

void bsp_uart_receive_to_idle_dma(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size)
{
  if (HAL_OK != HAL_UARTEx_ReceiveToIdle_DMA(huart, data, size))
  {
    Error_Handler();
  }
}

void bsp_uart_dma_unregister_callback(DMA_HandleTypeDef *hdma, HAL_DMA_CallbackIDTypeDef CallbackID)
{
  HAL_DMA_UnRegisterCallback(hdma, CallbackID);
}

void bsp_uart_deinit_peripheral(void)
{
  NVIC_DisableIRQ(USART1_IRQn);
  HAL_NVIC_DisableIRQ(DMA2_Stream2_IRQn);

  HAL_UART_DeInit(&huart1);
  HAL_UART_DeInit(&huart2);
  HAL_DMA_DeInit(&hdma_usart1_rx);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
  bootloader_command_data_t command_data;
  uint16_t                  number_char_receive;
  uint8_t                   check_data_full[256];

  /* read data receive from dma */
  if ((huart->Instance == USART1))
  {
    static uint16_t old_pos = 0;
    uint16_t        i;

    /* check data when dma transfer complete */
    if (g_flag_cplt_dma)
    {
      g_flag_cplt_dma = FALSE;
      for (i = 0; i < size - old_pos; i++)
      {
        check_data_full[i] = data_receive_dma[old_pos + i];
      }
      /* check data form correct */
      if (bsp_uart_handle_rx_data_callback(&command_data, check_data_full, size - old_pos) == bootloader_command_error)
        return;
    }

    /* check if new data */
    if (size != old_pos)
    {
      /* check wraps around index data */
      if (size > old_pos)
      {
        number_char_receive = size - old_pos;
        for (i = 0; i < number_char_receive; i++)
        {
          /* read data from buffer dma */
          buffer_save_data_handle[i] = data_receive_dma[old_pos + i];
        }
      }
      else
      {
        number_char_receive = RX_BUFFER_SIZE - old_pos;
        for (i = 0; i < number_char_receive; i++)
        {
          /* read data from buffer dma */
          buffer_save_data_handle[i] = data_receive_dma[old_pos + i];
        }
        if (size > 0)
        {
          for (i = 0; i < size; i++)
          {
            /* read data from buffer dma */
            buffer_save_data_handle[number_char_receive + i] = data_receive_dma[i];
          }
          number_char_receive += size;
        }
      }

#ifdef DEBUG
      HAL_UART_Transmit(&huart2, buffer_save_data_handle, number_char_receive, 100);
      HAL_UART_Transmit(&huart2, (uint8_t *) "\r\n", 2, 100);
#endif
      /* handle data read */
      if (bsp_uart_handle_rx_data_callback(&command_data, buffer_save_data_handle, number_char_receive) == bootloader_comand_ok)
      {
        bsp_uart_bootloader_comand_callback(command_data);
      }
      else
        return;
    }
    old_pos = size;
  }
}

/* End of file -------------------------------------------------------- */
