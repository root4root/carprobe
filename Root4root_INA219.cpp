#include "Root4root_INA219.h"

/*!
 *  @brief  Instantiates a new INA219 class
 *  @param addr the I2C address the device can be found on. Default is 0x40
 */
Root4root_INA219::Root4root_INA219(uint8_t addr, TwoWire *theWire)
{
    this->ina219_i2caddr = addr;
    this->i2c = theWire;
}

/*!
 *  @brief  Setups the HW (defaults to 32V and 2A for calibration values)
 *  @param theWire the TwoWire object to use
 */
void Root4root_INA219::begin(uint16_t expected, byte rshunt)
{
    this->i2c->begin();
    setCalibration(expected, rshunt);
}

void Root4root_INA219::setCalibration(uint16_t expected, uint8_t rshunt)
{
    this->ina219_currentLSB = (float)expected/32768.0;
    this->ina219_calibrationValue = (uint32_t)(40960.0/(this->ina219_currentLSB * rshunt));
    this->ina219_powerLSB = 20 * this->ina219_currentLSB;

    writeRegister(INA219_CALIBRATION_REGISTER, this->ina219_calibrationValue);
}

void Root4root_INA219::changeConfig(uint16_t value, uint16_t mask)
{
    this->ina219_config &= ~mask;
    this->ina219_config |= value;

    writeRegister(INA219_CONFIG_REGISTER, this->ina219_config);
}

/*!
 *  @brief  Gets the raw bus voltage (16-bit signed integer, so +-32767)
 *  @return the raw bus voltage reading
 */
uint16_t Root4root_INA219::getBusVoltage_mV()
{
    uint16_t value;
    readRegister(INA219_BUS_VOLTAGE_REGISTER, &value);

    // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
    return (uint16_t)((value >> 3) * 4);
}

/*!
 *  @brief  Gets the raw shunt voltage (16-bit signed integer, so +-32767)
 *  @return the raw shunt voltage reading
 */
int16_t Root4root_INA219::getShuntVoltage_raw()
{
    uint16_t value;
    readRegister(INA219_SHUNT_VOLTAGE_REGISTER, &value);
    return (int16_t)value;
}


/*!
 *  @brief  Gets the shunt voltage in mV (so +-327mV)
 *  @return the shunt voltage converted to millivolts
 */
float Root4root_INA219::getShuntVoltage_mV()
{
    int16_t value;
    value = getShuntVoltage_raw();
    // LSB 10uV, so x = value*10/1000 = value/100mV or value*0.01
    return value * 0.01;
}

/*!
 *  @brief  Gets the shunt voltage in volts
 *  @return the bus voltage converted to volts
 */
float Root4root_INA219::getBusVoltage_V()
{
    int16_t value = getBusVoltage_mV();
    return value * 0.001; // x/1000
}

/*!
 *  @brief  Gets the current value in mA, taking into account the
 *          config settings and current LSB
 *  @return the current reading convereted to milliamps
 */
float Root4root_INA219::getCurrent_mA()
{
    float valueDec = getCurrent_raw();
    valueDec *= this->ina219_currentLSB;
    return valueDec;
}

/*!
 *  @brief  Gets the raw current value (16-bit signed integer, so +-32767)
 *  @return the raw current reading
 */
int16_t Root4root_INA219::getCurrent_raw()
{
    uint16_t value;

    readRegister(INA219_CURRENT_REGISTER, &value);

    if (!value) {
        if(!checkConfig()) {
            delay(10);
            return getCurrent_raw();
        }
    }

    return (int16_t)value;
}


/*!
 *  @brief  Gets the power value in mW, taking into account the
 *          config settings and current LSB
 *  @return power reading converted to milliwatts
 */
float Root4root_INA219::getPower_mW()
{
    float valueDec = getPower_raw();
    valueDec *= this->ina219_powerLSB;
    return valueDec;
}

/*!
 *  @brief  Gets the raw power value (16-bit signed integer, so +-32767)
 *  @return raw power reading
 */
int16_t Root4root_INA219::getPower_raw()
{
    uint16_t value;

    readRegister(INA219_POWER_REGISTER, &value);

    if (!value) {
        if(!checkConfig()) {
            delay(10);
            return getPower_raw();
        }
    }

    return (int16_t)value;
}

/*!
 *  @brief  enable or disable powersafe mode
 *  @param  on
 *          boolean value
 */
void Root4root_INA219::powerSave(bool on)
{
    if (on) {
        uint16_t save = this->ina219_config & ~INA219_CONFIG_MODE_MASK;
        writeRegister(INA219_CONFIG_REGISTER, save);
    } else {
        writeRegister(INA219_CONFIG_REGISTER, this->ina219_config);
    }
}

void Root4root_INA219::reset()
{
    writeRegister(INA219_CONFIG_REGISTER, INA219_CONFIG_RESET);
    delay(10);
}

/*!
 *  @brief  Sends a single command byte over I2C
 *  @param  reg
 *          register address
 *  @param  value
 *          value to write
 */
void Root4root_INA219::writeRegister(uint8_t reg, uint16_t value)
{
    this->i2c->beginTransmission(ina219_i2caddr);
    this->i2c->write(reg);                     // Register
    //this->i2c->write((uint8_t)(value & 0xFF));
    this->i2c->write((uint8_t)(value >> 8));   // Upper 8-bits
    this->i2c->write((uint8_t)(value & 0xFF)); // Lower 8-bits
    this->i2c->endTransmission();
}

/*!
 *  @brief  Reads a 16 bit values over I2C
 *  @param  reg
 *          register address
 *  @param  *value
 *          read value
 */
void Root4root_INA219::readRegister(uint8_t reg, uint16_t *value)
{
    this->i2c->beginTransmission(ina219_i2caddr);
    this->i2c->write(reg); // Register
    this->i2c->endTransmission();

    delay(5); // Max 12-bit conversion time is 586us per sample

    this->i2c->requestFrom(ina219_i2caddr, (uint8_t)2);

    *value = ((i2c->read() << 8) | i2c->read());
}



bool Root4root_INA219::checkConfig()
{
    uint16_t calibrationRegister = 0;

    readRegister(INA219_CALIBRATION_REGISTER, &calibrationRegister);

    if (!calibrationRegister) {
        writeRegister(INA219_CONFIG_REGISTER, this->ina219_config);
        writeRegister(INA219_CALIBRATION_REGISTER, this->ina219_calibrationValue);
        return false;
    }

    return true;
}

void Root4root_INA219::trigger()
{
    writeRegister(INA219_CONFIG_REGISTER, this->ina219_config);
}

