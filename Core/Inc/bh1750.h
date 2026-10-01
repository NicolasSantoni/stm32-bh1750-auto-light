#ifndef BH1750_H
#define BH1750_H

#include "stm32f3xx_hal.h"  // Biblioteca HAL

#define BH1750_ADDR (0x23 << 1)  // 0100011

// modos de medição continua (datasheet)
#define BH1750_MODE_CONTINUOUS_H_RES  0x10 // 1 lx
#define BH1750_MODE_CONTINUOUS_H_RES2 0x11 // 0.5 lx
#define BH1750_MODE_CONTINUOUS_L_RES  0x13 // 4 lx

HAL_StatusTypeDef BH1750_PowerOn(I2C_HandleTypeDef *hi2c); // Sensor ligado e aguardando o comando de medição
HAL_StatusTypeDef BH1750_SetMode(I2C_HandleTypeDef *hi2c, uint8_t mode); // Define o modo e inicia a medição
HAL_StatusTypeDef BH1750_ReadLux(I2C_HandleTypeDef *hi2c, float *lux); // Lê a última medição e converte para lux

#endif