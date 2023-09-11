
#ifndef BSP_UTILITY_H
#define BSP_UTILITY_H

#include "main.h"

void bsp_utility_parse_element(uint8_t *des, uint8_t *buff, uint8_t length);

uint8_t bsp_utility_char_to_hex(uint8_t c);

#endif