/**************************************************************************/
/*!
    @file     ADS1015.cpp
    @author   K.Townsend (Adafruit Industries)

    @mainpage Adafruit ADS1X15 ADC Breakout Driver

    @section intro_sec Introduction

    This is a library for the Adafruit ADS1X15 ADC breakout boards.

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section author Author

    Written by Kevin "KTOWN" Townsend for Adafruit Industries.

    @section  HISTORY

    v1.0  - First release
    v1.1  - Added ADS1115 support - W. Earl

    @section license License

    BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/

#include "ADS1X15.h"

#include <math.h>

ADS1X15::ADS1X15(uint8_t bitShift,
                 uint16_t dataRateBits,
                 I2CAddress i2cAddress,
                 TwoWire *i2cBus) :
    m_bitShift(bitShift),
    _i2cAddress(i2cAddress),
    _i2cBus(i2cBus),
    _dataRateBits(dataRateBits),
    m_gain(GAIN_TWOTHIRDS) {
}

ADS1X15::~ADS1X15() {
}

/**************************************************************************/
/*!
    @brief  Sets up the HW (reads coefficients values, etc.)
*/
/**************************************************************************/
bool ADS1X15::begin() { return true; }

/**************************************************************************/
/*!
    @brief  Sets the gain and input voltage range

    @param gain gain setting to use
*/
/**************************************************************************/
void ADS1X15::setGain(adsGain_t gain) { m_gain = gain; }

/**************************************************************************/
/*!
    @brief  Gets a gain and input voltage range

    @return the gain setting
*/
/**************************************************************************/
adsGain_t ADS1X15::getGain() const { return m_gain; }

/**************************************************************************/
/*!
    @brief  Gets a single-ended ADC reading from the specified channel

    @param channel ADC channel to read

    @return the ADC reading
*/
/**************************************************************************/
bool ADS1X15::readADC_SingleEnded(uint8_t channel, int16_t *value) {
  if (channel > 3) {
    return false;
  }

  // Start with default values
  uint16_t config =
      ADS1015_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
      ADS1015_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1015_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      _dataRateBits                 |   // samples per second
      ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set single-ended input channel
  switch (channel) {
  case (0):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
    break;
  case (1):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
    break;
  case (2):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
    break;
  case (3):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
    break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  if (!writeRegister(ADS1015_REG_POINTER_CONFIG, config)) {
    return false;
  }

  // Wait for the conversion to complete
  delay(conversionDelay());

  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1015
  if (!readRegister(ADS1015_REG_POINTER_CONVERT, (uint16_t *)value)) {
    return false;
  }
  if (value) {
    *value = *value >> m_bitShift;
  }
  return true;
}

/**************************************************************************/
/*!
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN0) and N (AIN1) input.  Generates
            a signed value since the difference can be either
            positive or negative.

    @return the ADC reading
*/
/**************************************************************************/
int16_t ADS1X15::readADC_Differential_0_1() {
  // Start with default values
  uint16_t config =
      ADS1015_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
      ADS1015_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1015_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      _dataRateBits                 |   // samples per second
      ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set channels
  config |= ADS1015_REG_CONFIG_MUX_DIFF_0_1; // AIN0 = P, AIN1 = N

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(ADS1015_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  delay(conversionDelay());

  // Read the conversion results
  uint16_t res;
  readRegister(ADS1015_REG_POINTER_CONVERT, &res);
  res = res >> m_bitShift;
  if (m_bitShift == 0) {
    return (int16_t)res;
  } else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF) {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}

/**************************************************************************/
/*!
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN2) and N (AIN3) input.  Generates
            a signed value since the difference can be either
            positive or negative.

    @return the ADC reading
*/
/**************************************************************************/
int16_t ADS1X15::readADC_Differential_2_3() {
  // Start with default values
  uint16_t config =
      ADS1015_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
      ADS1015_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1015_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      _dataRateBits                 |   // samples per second
      ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set channels
  config |= ADS1015_REG_CONFIG_MUX_DIFF_2_3; // AIN2 = P, AIN3 = N

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(ADS1015_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  delay(conversionDelay());

  // Read the conversion results
  uint16_t res;
  readRegister(ADS1015_REG_POINTER_CONVERT, &res);
  res = res >> m_bitShift;
  if (m_bitShift == 0) {
    return (int16_t)res;
  } else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF) {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}

/**************************************************************************/
/*!
    @brief  Sets up the comparator to operate in basic mode, causing the
            ALERT/RDY pin to assert (go from high to low) when the ADC
            value exceeds the specified threshold.

            This will also set the ADC in continuous conversion mode.

    @param channel ADC channel to use
    @param threshold comparator threshold
*/
/**************************************************************************/
bool ADS1X15::startComparator_SingleEnded(uint8_t channel, int16_t threshold) {
  // Start with default values
  uint16_t config =
      ADS1015_REG_CONFIG_CQUE_1CONV |   // Comparator enabled and asserts on 1
                                        // match
      ADS1015_REG_CONFIG_CLAT_LATCH |   // Latching mode
      ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1015_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      _dataRateBits                 |   // samples per second
      ADS1015_REG_CONFIG_MODE_CONTIN |  // Continuous conversion mode
      ADS1015_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

  // Set PGA/voltage range
  config |= m_gain;

  // Set single-ended input channel
  switch (channel) {
  case (0):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
    break;
  case (1):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
    break;
  case (2):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
    break;
  case (3):
    config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
    break;
  }

  // Set the high threshold register
  // Shift 12-bit results left 4 bits for the ADS1015
  if (!writeRegister(ADS1015_REG_POINTER_HITHRESH, threshold << m_bitShift)) {
    return false;
  }

  // Write config register to the ADC
  return writeRegister(ADS1015_REG_POINTER_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  In order to clear the comparator, we need to read the
            conversion results.  This function reads the last conversion
            results without changing the config value.

    @return the last ADC reading
*/
/**************************************************************************/
int16_t ADS1X15::getLastConversionResults() {
  // Wait for the conversion to complete
  delay(conversionDelay());

  // Read the conversion results
  uint16_t res;
  readRegister(ADS1015_REG_POINTER_CONVERT, &res);
  res = res >> m_bitShift;
  if (m_bitShift == 0) {
    return (int16_t)res;
  } else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF) {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}

unsigned int ADS1X15::conversionDelay() const {
  unsigned int value = ceil(1000. / (float)samplePerSecond());
  // Adding an extra 1ms for security.
  return value + 1;
};

bool ADS1X15::writeRegister(uint8_t reg, uint16_t value) const {
  Wire.beginTransmission((uint8_t)_i2cAddress);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)(value>>8));
  Wire.write((uint8_t)(value & 0xFF));
  return Wire.endTransmission() == 0;
}

bool ADS1X15::readRegister(uint8_t reg, uint16_t *value) const {
  Wire.beginTransmission((uint8_t)_i2cAddress);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  Wire.requestFrom((uint8_t)_i2cAddress, 2);
  if (value) {
    *value = ((Wire.read() << 8) | Wire.read());
  }
  return true;
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1015 class w/appropriate properties

    @param i2cAddress I2C address of device
    @param i2cBus I2C bus
*/
/**************************************************************************/
ADS1015::ADS1015(I2CAddress i2cAddress, TwoWire *i2cBus) :
    ADS1X15(4, (uint16_t)DataRate::SPS1600, i2cAddress, i2cBus)
{
}

unsigned int ADS1015::samplePerSecond() const {
  switch (dataRate()) {
  case DataRate::SPS128:
    return 128;
  case DataRate::SPS250:
    return 250;
  case DataRate::SPS490:
    return 490;
  case DataRate::SPS920:
    return 920;
  case DataRate::SPS1600:
    return 1600;
  case DataRate::SPS2400:
    return 2400;
  case DataRate::SPS3300:
    return 3300;
  }
  return 1;
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties

    @param i2cAddress I2C address of device
    @param i2cBus I2C bus
*/
/**************************************************************************/
ADS1115::ADS1115(I2CAddress i2cAddress, TwoWire *i2cBus) :
    ADS1X15(0, (uint16_t)DataRate::SPS128, i2cAddress, i2cBus)
{
}

unsigned int ADS1115::samplePerSecond() const {
  switch (dataRate()) {
  case DataRate::SPS8:
    return 8;
  case DataRate::SPS16:
    return 16;
  case DataRate::SPS32:
    return 32;
  case DataRate::SPS64:
    return 64;
  case DataRate::SPS128:
    return 128;
  case DataRate::SPS250:
    return 250;
  case DataRate::SPS475:
    return 475;
  case DataRate::SPS860:
    return 860;
  }
  return 1;
}
