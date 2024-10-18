//
//   SDL_Arduino_INA3221 Library
//   SDL_Arduino_INA3221.cpp Arduino code - runs in continuous mode
//   Version 1.2
//   SwitchDoc Labs   September 2019
//
//



/*
    Initial code from INA219 code (Basically just a core structure left)
    @author   K.Townsend (Adafruit Industries)
	@license  BSD (see BSDlicense.txt)
*/
#include "ina3221.h"

uint8_t INA3221_i2caddr = INA3221_ADDRESS;
float INA3221_shuntresistor = 0.05;

void wireWriteRegister(uint8_t reg, uint16_t value)
{
    uint8_t temp[2] = {0};

    temp[0] = ((value >> 8) & 0xFF);
    temp[1] = (value & 0xFF);
    I2C_Write_Bytes(INA3221_WRITE_ADDRESS, reg, temp, 2, I2C_TIMOUT_MS);
}

void wireReadRegister(uint8_t reg, uint16_t *value)
{
    uint8_t data[2] = {0};

    I2C_Read_Bytes(INA3221_READ_ADDRESS, reg, data, 2, I2C_TIMOUT_MS);

    *value = ((data[0] << 8) | data[1]);
}

void INA3221SetConfig(void)
{
    // Set Config register to take into account the settings above
    uint16_t config = INA3221_CONFIG_ENABLE_CHAN1 |
                    INA3221_CONFIG_ENABLE_CHAN2 |
                    INA3221_CONFIG_ENABLE_CHAN3 |
                    INA3221_CONFIG_AVG1 |
                    INA3221_CONFIG_VBUS_CT2 |
                    INA3221_CONFIG_VSH_CT2 |
                    INA3221_CONFIG_MODE_2 |
                    INA3221_CONFIG_MODE_1 |
                    INA3221_CONFIG_MODE_0;
    wireWriteRegister(INA3221_REG_CONFIG, config);
}

/**************************************************************************/
/*! 
    @brief  Gets the raw bus voltage (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
int16_t getBusVoltage_raw(int channel) {
    uint16_t value;
    wireReadRegister(INA3221_REG_BUSVOLTAGE_1+(channel -1) *2, &value);

    // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
    return (int16_t)(value );
}

/**************************************************************************/
/*! 
    @brief  Gets the raw shunt voltage (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
int16_t getShuntVoltage_raw(int channel) {
    uint16_t value;
    wireReadRegister(INA3221_REG_SHUNTVOLTAGE_1+(channel -1) *2, &value);

    return (int16_t)value;
}

/**************************************************************************/
/*! 
    @brief  Gets the shunt voltage in mV (so +-168.3mV)
*/
/**************************************************************************/
float getShuntVoltage_mV(int channel) {
    int16_t value;
    value = getShuntVoltage_raw(channel);
    return value * 0.005;
}

/**************************************************************************/
/*! 
    @brief  Gets the shunt voltage in volts
*/
/**************************************************************************/
float getBusVoltage_V(int channel) {
    int16_t value = getBusVoltage_raw(channel);
    return value * 0.001;
}

/**************************************************************************/
/*! 
    @brief  Gets the current value in mA, taking into account the
            config settings and current LSB
*/
/**************************************************************************/
float getCurrent_mA(int channel) {
    float valueDec = getShuntVoltage_mV(channel)/INA3221_shuntresistor;

    return valueDec;
}

/**************************************************************************/
/*! 
    @brief  Gets the Manufacturers ID
*/
/**************************************************************************/
int getManufID(void)
{
    uint16_t value;
    wireReadRegister(0xFE, &value);
    return value;
}
