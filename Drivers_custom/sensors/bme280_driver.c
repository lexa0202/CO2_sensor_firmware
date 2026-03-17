#include "bme280_driver.h"
#include "i2c.h"

#define BME280_ADDR (0x76 << 1)

typedef struct
{
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;

    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;

} BME280_CalibData;

static BME280_CalibData calib;
static int32_t t_fine;

static uint8_t read8(uint8_t reg)
{
    uint8_t val = 0;

    HAL_I2C_Mem_Read(&hi2c1,
                     BME280_ADDR,
                     reg,
                     I2C_MEMADD_SIZE_8BIT,
                     &val,
                     1,
                     100);

    return val;
}

static void readBlock(uint8_t reg, uint8_t *buf, uint8_t len)
{
    HAL_I2C_Mem_Read(&hi2c1,
                     BME280_ADDR,
                     reg,
                     I2C_MEMADD_SIZE_8BIT,
                     buf,
                     len,
                     100);
}

static void write8(uint8_t reg, uint8_t val)
{
    HAL_I2C_Mem_Write(&hi2c1,
                      BME280_ADDR,
                      reg,
                      I2C_MEMADD_SIZE_8BIT,
                      &val,
                      1,
                      100);
}

int BME280_Init(void)
{
    uint8_t id = read8(0xD0);

    if (id != 0x60)
        return -1;

    uint8_t buf[26];

    readBlock(0x88, buf, 26);

    calib.dig_T1 = (buf[1] << 8) | buf[0];
    calib.dig_T2 = (buf[3] << 8) | buf[2];
    calib.dig_T3 = (buf[5] << 8) | buf[4];

    calib.dig_P1 = (buf[7] << 8) | buf[6];
    calib.dig_P2 = (buf[9] << 8) | buf[8];
    calib.dig_P3 = (buf[11] << 8) | buf[10];
    calib.dig_P4 = (buf[13] << 8) | buf[12];
    calib.dig_P5 = (buf[15] << 8) | buf[14];
    calib.dig_P6 = (buf[17] << 8) | buf[16];
    calib.dig_P7 = (buf[19] << 8) | buf[18];
    calib.dig_P8 = (buf[21] << 8) | buf[20];
    calib.dig_P9 = (buf[23] << 8) | buf[22];

    calib.dig_H1 = buf[25];

    uint8_t hbuf[7];
    readBlock(0xE1, hbuf, 7);

    calib.dig_H2 = (hbuf[1] << 8) | hbuf[0];
    calib.dig_H3 = hbuf[2];
    calib.dig_H4 = (hbuf[3] << 4) | (hbuf[4] & 0x0F);
    calib.dig_H5 = (hbuf[5] << 4) | (hbuf[4] >> 4);
    calib.dig_H6 = (int8_t)hbuf[6];

    write8(0xF2, 0x01);
    write8(0xF4, 0x25); //sensor sleep, after wake up, measure, sleep again//if always on - 0x27
    write8(0xF5, 0xA0);

    return 0;
}

static int32_t compensate_T(int32_t adc_T)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) *
           ((int32_t)calib.dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) *
            ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) *
            ((int32_t)calib.dig_T3)) >> 14;

    t_fine = var1 + var2;

    T = (t_fine * 5 + 128) >> 8;

    return T;
}

static uint32_t compensate_P(int32_t adc_P)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) +
           ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) *
           ((int64_t)calib.dig_P1) >> 33;

    if (var1 == 0)
        return 0;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) *
           (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) +
        (((int64_t)calib.dig_P7) << 4);

    return (uint32_t)p;
}

static uint32_t compensate_H(int32_t adc_H)
{
    int32_t v_x1;

    v_x1 = (t_fine - ((int32_t)76800));

    v_x1 = (((((adc_H << 14) -
              (((int32_t)calib.dig_H4) << 20) -
              (((int32_t)calib.dig_H5) * v_x1)) +
             ((int32_t)16384)) >> 15) *
            (((((((v_x1 *
                  ((int32_t)calib.dig_H6)) >> 10) *
                (((v_x1 *
                   ((int32_t)calib.dig_H3)) >> 11) +
                 ((int32_t)32768))) >> 10) +
              ((int32_t)2097152)) *
             ((int32_t)calib.dig_H2) + 8192) >> 14));

    v_x1 = (v_x1 -
           (((((v_x1 >> 15) *
               (v_x1 >> 15)) >> 7) *
             ((int32_t)calib.dig_H1)) >> 4));

    if (v_x1 < 0)
        v_x1 = 0;

    if (v_x1 > 419430400)
        v_x1 = 419430400;

    return (uint32_t)(v_x1 >> 12);
}

int BME280_StartMeasurement(void)
{
    uint8_t ctrl = read8(0xF4);
    write8(0xF4, ctrl | 0x01);
    return 0;
}

int BME280_ReadMeasurement(float *temperature,
                           float *humidity,
                           float *pressure)
{
    uint8_t data[8];

    readBlock(0xF7, data, 8);

    int32_t adc_P = (data[0] << 12) |
                    (data[1] << 4) |
                    (data[2] >> 4);

    int32_t adc_T = (data[3] << 12) |
                    (data[4] << 4) |
                    (data[5] >> 4);

    int32_t adc_H = (data[6] << 8) |
                     data[7];

    int32_t T = compensate_T(adc_T);
    uint32_t P = compensate_P(adc_P);
    uint32_t H = compensate_H(adc_H);

    *temperature = T / 100.0f;
    *pressure = (P / 256.0f) / 100.0f;
    *humidity = H / 1024.0f;

    return 0;
}

