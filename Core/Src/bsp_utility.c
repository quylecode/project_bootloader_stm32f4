#include "bsp_utility.h"

void bsp_utility_parse_element(uint8_t *des, uint8_t *buff, uint8_t length)
{
  uint8_t count_array = 0U;
  uint8_t buffer_temp[256];

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

uint8_t bsp_utility_char_to_hex(uint8_t c)
{
  uint8_t result = 0;
  /*0 ... 9*/
  if ((c >= '0') && (c <= '9'))
    result = c - 48;
  /*A ... F*/
  else if ((c >= 'A') && (c <= 'F'))
    result = c - 55;
  return result;
}
