idf_component_register(
    SRCS "ina219_library.cpp"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_i2c
)