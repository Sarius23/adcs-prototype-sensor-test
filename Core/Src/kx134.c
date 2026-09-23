#include "kx134.h"

#define KX134_REG_XOUT_L      0x08U
#define KX134_REG_COTR        0x12U
#define KX134_REG_WHO_AM_I    0x13U
#define KX134_REG_CNTL1       0x1BU
#define KX134_REG_CNTL2       0x1CU
#define KX134_REG_ODCNTL      0x21U
#define KX134_REG_INTERNAL_7F 0x7FU

#define KX134_WHO_AM_I_VALUE  0x46U
#define KX134_COTR_VALUE      0x55U
#define KX134_CNTL1_TEST      0xC0U   /* PC1=1, RES=1, +/-8 g */
#define KX134_ODCNTL_50HZ     0x06U
#define KX134_TIMEOUT_MS      100U

static void select(KX134_t *d)   { HAL_GPIO_WritePin(d->cs_port, d->cs_pin, GPIO_PIN_RESET); }
static void deselect(KX134_t *d) { HAL_GPIO_WritePin(d->cs_port, d->cs_pin, GPIO_PIN_SET); }

static KX134_Status write_reg(KX134_t *d, uint8_t reg, uint8_t value)
{
    uint8_t tx[2] = {(uint8_t)(reg & 0x7FU), value};
    select(d);
    HAL_StatusTypeDef st = HAL_SPI_Transmit(d->hspi, tx, 2U, KX134_TIMEOUT_MS);
    deselect(d);
    return (st == HAL_OK) ? KX134_OK : KX134_ERROR_SPI;
}

static KX134_Status read_regs(KX134_t *d, uint8_t reg, uint8_t *data, uint16_t len)
{
    if (!d || !data || len == 0U) return KX134_ERROR_ARGUMENT;

    uint8_t addr = (uint8_t)(reg | 0x80U);
    select(d);

    if (HAL_SPI_Transmit(d->hspi, &addr, 1U, KX134_TIMEOUT_MS) != HAL_OK) {
        deselect(d);
        return KX134_ERROR_SPI;
    }

    if (len > 6U) {
        deselect(d);
        return KX134_ERROR_ARGUMENT;
    }

    uint8_t dummy_tx[6] = {0};

    if (HAL_SPI_TransmitReceive(d->hspi,
                                dummy_tx,
                                data,
                                len,
                                KX134_TIMEOUT_MS) != HAL_OK) {
        deselect(d);
        return KX134_ERROR_SPI;
    }

    deselect(d);
    return KX134_OK;
}

static KX134_Status read_reg(KX134_t *d, uint8_t reg, uint8_t *value)
{
    return read_regs(d, reg, value, 1U);
}

KX134_Status KX134_Init(KX134_t *d, SPI_HandleTypeDef *hspi,
                        GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    if (!d || !hspi || !cs_port) return KX134_ERROR_ARGUMENT;

    d->hspi = hspi;
    d->cs_port = cs_port;
    d->cs_pin = cs_pin;
    d->who_am_i = 0U;
    d->cotr = 0U;

    deselect(d);
    HAL_Delay(60U);

    /* Kionix TN027 recommended SPI reset sequence. */
    if (write_reg(d, KX134_REG_INTERNAL_7F, 0x00U) != KX134_OK) return KX134_ERROR_SPI;
    if (write_reg(d, KX134_REG_CNTL2, 0x00U) != KX134_OK) return KX134_ERROR_SPI;
    if (write_reg(d, KX134_REG_CNTL2, 0x80U) != KX134_OK) return KX134_ERROR_SPI;
    HAL_Delay(5U); /* reset time specified as 2 ms; margin added */

    if (read_reg(d, KX134_REG_WHO_AM_I, &d->who_am_i) != KX134_OK) return KX134_ERROR_SPI;
    if (d->who_am_i != KX134_WHO_AM_I_VALUE) return KX134_ERROR_WHO_AM_I;

    if (read_reg(d, KX134_REG_COTR, &d->cotr) != KX134_OK) return KX134_ERROR_SPI;
    if (d->cotr != KX134_COTR_VALUE) return KX134_ERROR_COTR;

    /* Configure while PC1=0. */
    if (write_reg(d, KX134_REG_CNTL1, 0x00U) != KX134_OK) return KX134_ERROR_SPI;
    if (write_reg(d, KX134_REG_ODCNTL, KX134_ODCNTL_50HZ) != KX134_OK) return KX134_ERROR_SPI;
    if (write_reg(d, KX134_REG_CNTL1, KX134_CNTL1_TEST) != KX134_OK) return KX134_ERROR_SPI;

    HAL_Delay(25U);
    return KX134_OK;
}

KX134_Status KX134_ReadAcceleration(KX134_t *d, KX134_Data *out)
{
    if (!d || !out) return KX134_ERROR_ARGUMENT;

    uint8_t b[6];
    KX134_Status st = read_regs(d, KX134_REG_XOUT_L, b, 6U);
    if (st != KX134_OK) return st;

    out->raw_x = (int16_t)(((uint16_t)b[1] << 8) | b[0]);
    out->raw_y = (int16_t)(((uint16_t)b[3] << 8) | b[2]);
    out->raw_z = (int16_t)(((uint16_t)b[5] << 8) | b[4]);

    out->x_g = (float)out->raw_x / 4096.0f;
    out->y_g = (float)out->raw_y / 4096.0f;
    out->z_g = (float)out->raw_z / 4096.0f;

    return KX134_OK;
}
