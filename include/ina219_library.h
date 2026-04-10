#pragma once
#include "driver/i2c_master.h"
#include <cstdint>

class INA219
{
private:
    i2c_master_dev_handle_t _dev_handle;
    uint8_t _dev_addr;

    static constexpr uint8_t REG_CONFIG = 0x00;
    static constexpr uint8_t REG_CURRENT = 0x04;
    static constexpr uint8_t REG_CALIBRATION = 0x05;
    static constexpr uint8_t REG_BUS_VOLTAGE = 0x02;

public:
    INA219(i2c_master_bus_handle_t bus_handle,
           uint8_t dev_addr = CONFIG_INA219_DEV_ADDR);

    esp_err_t calibrate(uint16_t cal_value = CONFIG_INA219_CALIBRATION_VALUE);

    esp_err_t readCurrent_mA(float *out_current);
    ~INA219();

    esp_err_t readBusVoltage_V(float *out_voltage);
};