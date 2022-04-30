/**************************************************************************/
/*!
    @file     ADS1X15.cpp
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

#include <Wire.h>
#include <math.h>


/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
#define ADS1X15_REG_POINTER_CONVERT (0x00)   ///< Conversion
#define ADS1X15_REG_POINTER_CONFIG (0x01)    ///< Configuration
#define ADS1X15_REG_POINTER_LOWTHRESH (0x02) ///< Low threshold
#define ADS1X15_REG_POINTER_HITHRESH (0x03)  ///< High threshold
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
#define ADS1X15_REG_CONFIG_OS_SINGLE (0x8000) ///< Write: Set to start a single-conversion
#define ADS1X15_REG_CONFIG_OS_BUSY (0x0000) ///< Read: Bit = 0 when conversion is in progress
#define ADS1X15_REG_CONFIG_OS_NOTBUSY (0x8000) ///< Read: Bit = 1 when device is not performing a conversion

#define ADS1X15_REG_CONFIG_MUX_DIFF_0_1 (0x0000) ///< Differential P = AIN0, N = AIN1 (default)
#define ADS1X15_REG_CONFIG_MUX_DIFF_0_3 (0x1000) ///< Differential P = AIN0, N = AIN3
#define ADS1X15_REG_CONFIG_MUX_DIFF_1_3 (0x2000) ///< Differential P = AIN1, N = AIN3
#define ADS1X15_REG_CONFIG_MUX_DIFF_2_3 (0x3000) ///< Differential P = AIN2, N = AIN3
#define ADS1X15_REG_CONFIG_MUX_DIFF_0_GND (0x4000) ///< Differential P = AIN0, N = GND
#define ADS1X15_REG_CONFIG_MUX_DIFF_1_GND (0x5000) ///< Differential P = AIN1, N = GND
#define ADS1X15_REG_CONFIG_MUX_DIFF_2_GND (0x6000) ///< Differential P = AIN2, N = GND
#define ADS1X15_REG_CONFIG_MUX_DIFF_3_GND (0x7000) ///< Differential P = AIN3, N = GND

#define ADS1X15_REG_CONFIG_PGA_6_144V (0x0000) ///< +/-6.144V range = Gain 2/3
#define ADS1X15_REG_CONFIG_PGA_4_096V (0x0200) ///< +/-4.096V range = Gain 1
#define ADS1X15_REG_CONFIG_PGA_2_048V (0x0400) ///< +/-2.048V range = Gain 2 (default)
#define ADS1X15_REG_CONFIG_PGA_1_024V (0x0600) ///< +/-1.024V range = Gain 4
#define ADS1X15_REG_CONFIG_PGA_0_512V (0x0800) ///< +/-0.512V range = Gain 8
#define ADS1X15_REG_CONFIG_PGA_0_256V (0x0A00) ///< +/-0.256V range = Gain 16

#define ADS1X15_REG_CONFIG_MODE_CONTIN (0x0000) ///< Continuous conversion mode
#define ADS1X15_REG_CONFIG_MODE_SINGLE (0x0100) ///< Power-down single-shot mode (default)

#define ADS1015_REG_CONFIG_DR_128SPS (0x0000) ///< 128 samples per second
#define ADS1015_REG_CONFIG_DR_250SPS (0x0020) ///< 250 samples per second
#define ADS1015_REG_CONFIG_DR_490SPS (0x0040) ///< 490 samples per second
#define ADS1015_REG_CONFIG_DR_920SPS (0x0060) ///< 920 samples per second
#define ADS1015_REG_CONFIG_DR_1600SPS (0x0080) ///< 1600 samples per second (default)
#define ADS1015_REG_CONFIG_DR_2400SPS (0x00A0) ///< 2400 samples per second
#define ADS1015_REG_CONFIG_DR_3300SPS (0x00C0) ///< 3300 samples per second

#define ADS1115_REG_CONFIG_DR_8SPS (0x0000) ///< 8 samples per second
#define ADS1115_REG_CONFIG_DR_16SPS (0x0020) ///< 16 samples per second
#define ADS1115_REG_CONFIG_DR_32SPS (0x0040) ///< 32 samples per second
#define ADS1115_REG_CONFIG_DR_64SPS (0x0060) ///< 64 samples per second
#define ADS1115_REG_CONFIG_DR_128SPS (0x0080) ///< 128 samples per second (default)
#define ADS1115_REG_CONFIG_DR_250SPS (0x00A0) ///< 250 samples per second
#define ADS1115_REG_CONFIG_DR_475SPS (0x00C0) ///< 475 samples per second
#define ADS1115_REG_CONFIG_DR_860SPS (0x00E0) ///< 860 samples per second

#define ADS1X15_REG_CONFIG_CMODE_TRAD (0x0000) ///< Traditional comparator with hysteresis (default)
#define ADS1X15_REG_CONFIG_CMODE_WINDOW (0x0010) ///< Window comparator

#define ADS1X15_REG_CONFIG_CPOL_ACTVLOW (0x0000) ///< ALERT/RDY pin is low when active (default)
#define ADS1X15_REG_CONFIG_CPOL_ACTVHI (0x0008) ///< ALERT/RDY pin is high when active

#define ADS1X15_REG_CONFIG_CLAT_NONLAT (0x0000) ///< Non-latching comparator (default)
#define ADS1X15_REG_CONFIG_CLAT_LATCH (0x0004) ///< Latching comparator

#define ADS1X15_REG_CONFIG_CQUE_1CONV (0x0000) ///< Assert ALERT/RDY after one conversions
#define ADS1X15_REG_CONFIG_CQUE_2CONV (0x0001) ///< Assert ALERT/RDY after two conversions
#define ADS1X15_REG_CONFIG_CQUE_4CONV (0x0002) ///< Assert ALERT/RDY after four conversions
#define ADS1X15_REG_CONFIG_CQUE_NONE (0x0003) ///< Disable the comparator and put ALERT/RDY in high state (default)
/*=========================================================================*/

namespace {

uint16_t gainRegisterValue(ADS1X15::Gain gain) {
  switch (gain) {
  case ADS1X15::Gain::TwoThirds:
    return ADS1X15_REG_CONFIG_PGA_6_144V;
  case ADS1X15::Gain::One:
    return ADS1X15_REG_CONFIG_PGA_4_096V;
  case ADS1X15::Gain::Two:
    return ADS1X15_REG_CONFIG_PGA_2_048V;
  case ADS1X15::Gain::Four:
    return ADS1X15_REG_CONFIG_PGA_1_024V;
  case ADS1X15::Gain::Eight:
    return ADS1X15_REG_CONFIG_PGA_0_512V;
  case ADS1X15::Gain::Sixteen:
    return ADS1X15_REG_CONFIG_PGA_0_256V;
  }
  return 0;
}

uint16_t inputChannelRegisterValue(ADS1X15::InputChannel channel) {
  switch (channel) {
  case ADS1X15::InputChannel::A0ToA1:
    return ADS1X15_REG_CONFIG_MUX_DIFF_0_1;
  case ADS1X15::InputChannel::A0ToA3:
    return ADS1X15_REG_CONFIG_MUX_DIFF_0_3;
  case ADS1X15::InputChannel::A1ToA3:
    return ADS1X15_REG_CONFIG_MUX_DIFF_1_3;
  case ADS1X15::InputChannel::A2ToA3:
    return ADS1X15_REG_CONFIG_MUX_DIFF_2_3;
  case ADS1X15::InputChannel::A0ToGND:
    return ADS1X15_REG_CONFIG_MUX_DIFF_0_GND;
  case ADS1X15::InputChannel::A1ToGND:
    return ADS1X15_REG_CONFIG_MUX_DIFF_1_GND;
  case ADS1X15::InputChannel::A2ToGND:
    return ADS1X15_REG_CONFIG_MUX_DIFF_2_GND;
  case ADS1X15::InputChannel::A3ToGND:
    return ADS1X15_REG_CONFIG_MUX_DIFF_3_GND;
  }
  return 0;
}

} // namespace


/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1X15 class w/appropriate properties

    @param i2cBus I2C bus
    @param i2cAddress I2C address of device
*/
/**************************************************************************/
ADS1X15::ADS1X15(TwoWire *i2cBus, I2CAddress i2cAddress) :
    _i2cAddress(i2cAddress),
    _i2cBus(i2cBus) {
}

ADS1X15::~ADS1X15() {
}

/**************************************************************************/
/*!
    @brief  Gets a single-ended ADC reading from the specified channel

    @param channel ADC channel to read
    @param value a pointer to int16_t to get the value

    @return true if the read was successful
*/
/**************************************************************************/
bool ADS1X15::readADCSingleEnded(InputChannel channel, int16_t *value) {
  // Start with default values
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      ADS1X15_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // samples per second
  config |= dataRateRegisterValue();

  // Set PGA/voltage range
  config |= gainRegisterValue(_gain);

  // Set single-ended input channel
  config |= inputChannelRegisterValue(channel);

  // Set 'start single-conversion' bit
  config |= ADS1X15_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  if (!writeRegister(ADS1X15_REG_POINTER_CONFIG, config)) {
    return false;
  }

  // Wait for the conversion to complete
  delay(conversionDelay());

  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1015
  if (!readRegister(ADS1X15_REG_POINTER_CONVERT, (uint16_t *)value)) {
    return false;
  }
  if (value) {
    *value = *value >> bitShift();
  }
  // Shift 12-bit results right 4 bits for the ADS1015,
  // making sure we keep the sign bit intact
  if (*value > 0x07FF) {
    // negative number - extend the sign to 16th bit
    *value |= 0xF000;
  }
  return true;
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
bool ADS1X15::startComparator_SingleEnded(InputChannel channel, int16_t threshold) {
  // Start with default values
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_1CONV |   // Comparator enabled and asserts on 1
                                        // match
      ADS1X15_REG_CONFIG_CLAT_LATCH |   // Latching mode
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      ADS1X15_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

  // samples per second
  config |= dataRateRegisterValue();

  // Set PGA/voltage range
  config |= gainRegisterValue(_gain);

  // Set single-ended input channel
  config |= inputChannelRegisterValue(channel);

  // Set the high threshold register
  // Shift 12-bit results left 4 bits for the ADS1015
  if (!writeRegister(ADS1X15_REG_POINTER_HITHRESH, threshold << bitShift())) {
    return false;
  }

  // Write config register to the ADC
  return writeRegister(ADS1X15_REG_POINTER_CONFIG, config);
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
  readRegister(ADS1X15_REG_POINTER_CONVERT, &res);
  res = res >> bitShift();
  if (bitShift() == 0) {
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
  uint8_t read = Wire.requestFrom((uint8_t)_i2cAddress, 2);
  if (read != 2) {
    return false;
  }
  if (value) {
    *value = ((Wire.read() << 8) | Wire.read());
  }
  return true;
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1015 class w/appropriate properties

    @param i2cBus I2C bus
    @param i2cAddress I2C address of device
*/
/**************************************************************************/
ADS1015::ADS1015(TwoWire *i2cBus, I2CAddress i2cAddress) :
    ADS1X15(i2cBus, i2cAddress)
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

uint8_t ADS1015::bitShift() const {
  return 4;
}

uint16_t ADS1015::dataRateRegisterValue() const {
  switch (dataRate()) {
  case DataRate::SPS128:
    return ADS1015_REG_CONFIG_DR_128SPS;
  case DataRate::SPS250:
    return ADS1015_REG_CONFIG_DR_250SPS;
  case DataRate::SPS490:
    return ADS1015_REG_CONFIG_DR_490SPS;
  case DataRate::SPS920:
    return ADS1015_REG_CONFIG_DR_920SPS;
  case DataRate::SPS1600:
    return ADS1015_REG_CONFIG_DR_1600SPS;
  case DataRate::SPS2400:
    return ADS1015_REG_CONFIG_DR_2400SPS;
  case DataRate::SPS3300:
    return ADS1015_REG_CONFIG_DR_3300SPS;
  }
  return 0;
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties

    @param i2cBus I2C bus
    @param i2cAddress I2C address of device
*/
/**************************************************************************/
ADS1115::ADS1115(TwoWire *i2cBus, I2CAddress i2cAddress) :
    ADS1X15(i2cBus, i2cAddress)
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

uint8_t ADS1115::bitShift() const {
  return 0;
}

uint16_t ADS1115::dataRateRegisterValue() const {
  switch (dataRate()) {
  case DataRate::SPS8:
    return ADS1115_REG_CONFIG_DR_8SPS;
  case DataRate::SPS16:
    return ADS1115_REG_CONFIG_DR_16SPS;
  case DataRate::SPS32:
    return ADS1115_REG_CONFIG_DR_32SPS;
  case DataRate::SPS64:
    return ADS1115_REG_CONFIG_DR_64SPS;
  case DataRate::SPS128:
    return ADS1115_REG_CONFIG_DR_128SPS;
  case DataRate::SPS250:
    return ADS1115_REG_CONFIG_DR_250SPS;
  case DataRate::SPS475:
    return ADS1115_REG_CONFIG_DR_475SPS;
  case DataRate::SPS860:
    return ADS1115_REG_CONFIG_DR_860SPS;
  }
  return 0;
}
