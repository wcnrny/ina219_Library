#include "ina219_library.h"

INA219::INA219(i2c_master_bus_handle_t bus_handle, uint8_t dev_addr) : _dev_addr(dev_addr)
{
    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = _dev_addr;
    dev_cfg.scl_speed_hz = CONFIG_INA219_I2C_CLK_SPEED;

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &_dev_handle));
};

esp_err_t INA219::calibrate(uint16_t cal_value)
{
    uint8_t payload[3];
    payload[0] = REG_CALIBRATION;
    payload[1] = static_cast<uint8_t>(cal_value >> 8);
    payload[2] = static_cast<uint8_t>(cal_value & 0xFF);

    return i2c_master_transmit(_dev_handle, payload, sizeof(payload), 100);
};

esp_err_t INA219::readCurrent_mA(float *out_current)
{
    uint8_t rx_data[2];
    esp_err_t err = i2c_master_transmit_receive(_dev_handle, &REG_CURRENT, 1, rx_data, sizeof(rx_data), 100);

    if (err != ESP_OK)
    {
        return err;
    }

    int16_t raw_current = (rx_data[0] << 8) | rx_data[1];

    /**
     * NOTE: We're multiplying 0.01f to convert raw_current data that we get from INA219 to mA data;
     * To understand what we're doing, please refer to
     * https://www.ti.com/lit/ds/symlink/ina219.pdf#%5B%7B%22num%22%3A87%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C0%2C575.3%2C0%5D
     * as we calculated
     */
    *out_current = (float)raw_current * 0.01f;
    return ESP_OK;
};
INA219::~INA219()
{
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(_dev_handle));
    _dev_handle = NULL;
};

esp_err_t INA219::readBusVoltage_V(float *out_voltage)
{
    uint8_t rx_data[2];

    esp_err_t err = i2c_master_transmit_receive(_dev_handle, &REG_BUS_VOLTAGE, 1, rx_data, 2, 100);
    if (err != ESP_OK)
    {
        return err;
    }

    uint16_t raw_status = (uint16_t)((rx_data[0] << 8) | rx_data[1]);

    /**
     * From the datasheet, we need to shift the 3 bits to the right so we can get the raw_voltage value.
     * Refer to: https://www.ti.com/lit/ds/symlink/ina219.pdf#%5B%7B%22num%22%3A87%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C0%2C575.3%2C0%5D
     * The Bus Voltage register bits are not right-aligned. In order to compute the value of the Bus Voltage, Bus Voltage
    Register contents must be shifted right by three bits. This shift puts the BD0 bit in the LSB position so that the
    contents can be multiplied by the Bus Voltage LSB of 4-mV to compute the bus voltage measured by the device.
    */
    uint16_t raw_voltage = raw_status >> 3;

    /**
     * And from the same page as the referred previously, we need to multiply 4 mV to compute the actual bus voltage read from
     * the sensor.
     */
    *out_voltage = (float)raw_voltage * 0.004f;
    return ESP_OK;
}