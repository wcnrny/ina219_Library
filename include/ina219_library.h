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

public:
    INA219(i2c_master_bus_handle_t bus_handle, uint8_t dev_addr);

    esp_err_t calibrate(uint16_t cal_value);

    float readCurrent_mA();
    ~INA219();

    float readBusVoltage_V();
};