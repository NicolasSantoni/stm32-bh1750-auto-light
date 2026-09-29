#ifndef BH1750_H
#define BH1750_H

#include "stm32f3xx_hal.h"  // ajuste o nome se a família gerada for diferente

#define BH1750_ADDR (0x23 << 1)  // ADDR no GND = 0x23 (7-bit); HAL espera já deslocado

// modos de medição continua (datasheet, tabela de instruções)
#define BH1750_MODE_CONTINUOUS_H_RES  0x10 // 1 lx, ~120-180ms
#define BH1750_MODE_CONTINUOUS_H_RES2 0x11 // 0.5 lx, ~120-180ms
#define BH1750_MODE_CONTINUOUS_L_RES  0x13 // 4 lx, ~16-24ms

HAL_StatusTypeDef BH1750_PowerOn(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BH1750_SetMode(I2C_HandleTypeDef *hi2c, uint8_t mode);
HAL_StatusTypeDef BH1750_ReadLux(I2C_HandleTypeDef *hi2c, float *lux);

#endif