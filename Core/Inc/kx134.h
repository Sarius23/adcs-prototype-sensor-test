#ifndef KX134_H
#define KX134_H

#include "main.h"
#include <stdint.h>

typedef enum {
    KX134_OK = 0,
    KX134_ERROR_ARGUMENT,
    KX134_ERROR_SPI,
    KX134_ERROR_WHO_AM_I,
    KX134_ERROR_COTR
} KX134_Status;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    uint8_t who_am_i;
    uint8_t cotr;
} KX134_t;

typedef struct {
    int16_t raw_x, raw_y, raw_z;
    float x_g, y_g, z_g;
} KX134_Data;

KX134_Status KX134_Init(KX134_t *dev, SPI_HandleTypeDef *hspi,
                        GPIO_TypeDef *cs_port, uint16_t cs_pin);
KX134_Status KX134_ReadAcceleration(KX134_t *dev, KX134_Data *data);

#endif
