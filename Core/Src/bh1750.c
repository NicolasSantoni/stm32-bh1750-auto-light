#include "bh1750.h"

#define BH1750_CMD_POWER_ON 0x01 // sai do power down, liga o ADC interno

HAL_StatusTypeDef BH1750_PowerOn(I2C_HandleTypeDef *hi2c) {
    uint8_t cmd = BH1750_CMD_POWER_ON;
    return HAL_I2C_Master_Transmit(hi2c, BH1750_ADDR, &cmd, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BH1750_SetMode(I2C_HandleTypeDef *hi2c, uint8_t mode) {
    // mode = um dos BH1750_MODE_* (bh1750.h). Isso ja dispara a medicao.
    return HAL_I2C_Master_Transmit(hi2c, BH1750_ADDR, &mode, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BH1750_ReadLux(I2C_HandleTypeDef *hi2c, float *lux) {
    uint8_t data[2];
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(hi2c, BH1750_ADDR, data, 2, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        uint16_t raw = ((uint16_t)data[0] << 8) | data[1]; // MSB primeiro
        *lux = raw / 1.2f; // fator de conversao do datasheet (modo H-res)
    }
    return status;
}