#include "bh1750.h"

#define BH1750_CMD_POWER_ON 0x01 // Liga o ADC

HAL_StatusTypeDef BH1750_PowerOn(I2C_HandleTypeDef *hi2c) {
    uint8_t cmd = BH1750_CMD_POWER_ON;
    return HAL_I2C_Master_Transmit(hi2c, BH1750_ADDR, &cmd, 1, HAL_MAX_DELAY); // Endereço + escrita, 0x01, STOP
}

HAL_StatusTypeDef BH1750_SetMode(I2C_HandleTypeDef *hi2c, uint8_t mode) {
    return HAL_I2C_Master_Transmit(hi2c, BH1750_ADDR, &mode, 1, HAL_MAX_DELAY); // Endereço + escrita, modo, STOP
}

HAL_StatusTypeDef BH1750_ReadLux(I2C_HandleTypeDef *hi2c, float *lux) {
    uint8_t data[2]; // Byte alto e byte baixo
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(hi2c, BH1750_ADDR, data, 2, HAL_MAX_DELAY); // Endereço + leitura, 2 bytes, STOP
    if (status == HAL_OK) {
        uint16_t raw = ((uint16_t)data[0] << 8) | data[1]; // MSB primeiro
        *lux = raw / 1.2f; // Contagem / 1.2
    }
    return status;
}