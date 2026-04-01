# INA219 Library for ESP-IDF

An ESP-IDF component for the INA219 bidirectional current/voltage sensor by Texas Instruments.

## What is INA219?

INA219 is a current and voltage sensing chip that communicates over I2C. It can measure bus voltage (up to 26V) and the current flowing through a shunt resistor on that bus. You set a calibration value based on your shunt resistor, and it gives you current readings directly.

## Wiring

| INA219 Pin | ESP32 Pin |
|------------|-----------|
| SDA        | I2C SDA   |
| SCL        | I2C SCL   |
| VCC        | 3.3V      |
| GND        | GND       |

> The actual GPIO pins for SDA/SCL depend on your I2C bus configuration. Check your `menuconfig` settings or your code.

## File Structure

```
ina219_Library/
├── CMakeLists.txt          # ESP-IDF component registration
├── Kconfig.projbuild       # menuconfig options
├── include/
│   └── ina219_library.h    # Class definition
├── ina219_library.cpp      # Implementation
└── README.md
```

## menuconfig Options

Run `idf.py menuconfig` and navigate to **INA219 Configuration** to set:

- **I2C Port** — Which I2C peripheral to use (default: I2C_NUM_0)
- **SDA GPIO** — SDA pin number (default: GPIO21)
- **SCL GPIO** — SCL pin number (default: GPIO22)
- **I2C Clock Speed** — Bus frequency in Hz (default: 100000)
- **Device Address** — INA219 I2C address (default: 0x40)
- **Calibration Value** — Calibration register value (default: 4096)

These are compiled-in defaults. You can still override them in code by passing different values to the constructor or `calibrate()`.

## I2C Address Table

The INA219 address depends on how A0 and A1 pins are wired:

| A1  | A0  | Address |
|-----|-----|---------|
| GND | GND | 0x40    |
| GND | VS+ | 0x41    |
| GND | SDA | 0x42    |
| GND | SCL | 0x43    |
| VS+ | GND | 0x44    |
| VS+ | VS+ | 0x45    |
| VS+ | SDA | 0x46    |
| VS+ | SCL | 0x47    |
| SDA | GND | 0x48    |
| SDA | VS+ | 0x49    |
| SDA | SDA | 0x4A    |
| SDA | SCL | 0x4B    |
| SCL | GND | 0x4C    |
| SCL | VS+ | 0x4D    |
| SCL | SDA | 0x4E    |
| SCL | SCL | 0x4F    |

## Usage

### 1. Create the I2C Bus

```cpp
#include "driver/i2c_master.h"

i2c_master_bus_config_t bus_config = {};
bus_config.i2c_port = CONFIG_INA219_I2C_PORT;
bus_config.sda_io_num = CONFIG_INA219_SDA_GPIO;
bus_config.scl_io_num = CONFIG_INA219_SCL_GPIO;
bus_config.clk_source = I2C_CLK_SRC_DEFAULT;

i2c_master_bus_handle_t bus_handle;
ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));
```

### 2. Create the INA219 Object

```cpp
#include "ina219_library.h"

INA219 sensor(bus_handle, CONFIG_INA219_DEV_ADDR);
```

### 3. Calibrate

```cpp
sensor.calibrate(CONFIG_INA219_CALIBRATION_VALUE);
```

The calibration value depends on your shunt resistor and desired measurement range. See the [INA219 datasheet](https://www.ti.com/lit/ds/symlink/ina219.pdf) for how to calculate it.

### 4. Read Measurements

```cpp
float voltage = sensor.readBusVoltage_V();
float current = sensor.readCurrent_mA();

printf("Bus Voltage: %.2f V\n", voltage);
printf("Current: %.2f mA\n", current);
```

## API Reference

### `INA219(i2c_master_bus_handle_t bus_handle, uint8_t dev_addr)`

Constructor. Takes an I2C master bus handle and the sensor's 7-bit address. Adds the device to the bus.

### `esp_err_t calibrate(uint16_t cal_value)`

Writes `cal_value` to the Calibration register (0x05). This determines the current measurement resolution and range. Returns `ESP_OK` on success.

### `float readCurrent_mA()`

Reads the Current register (0x04) and returns the value in milliamps. Returns `-1.0f` on I2C error.

How it works:
- Reads 2 bytes from the register (MSB first)
- Combines them into a signed 16-bit integer (current can be negative)
- Multiplies by 0.01 to convert to mA (each LSB = 0.01 mA per datasheet)

### `float readBusVoltage_V()`

Reads the Bus Voltage register (0x02) and returns the value in Volts. Returns `0.0f` on I2C error.

How it works:
- Reads 2 bytes from the register
- The bus voltage bits are not right-aligned in the register — the first 3 bits are status flags (CNVR, OVF)
- Shifts right by 3 to extract the 13-bit voltage value
- Multiplies by 4 mV (0.004) to convert to Volts (each LSB = 4 mV per datasheet)

### `~INA219()`

Destructor. Removes the I2C device from the bus.

## Dependencies

- ESP-IDF v5.0+
- `esp_driver_i2c` component (specified in CMakeLists.txt)

## License

Developed as part of an embedded systems project.
