/**************************************************************************/
/*!
    @file     Adafruit_ADS1015.h

    This is a library for the Adafruit ADS1X15 ADC breakout boards.

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    Written by Kevin "KTOWN" Townsend for Adafruit Industries.

    BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/

#ifndef ADS1X15_h
#define ADS1X15_h

#include <Arduino.h>

class TwoWire;

class ADS1X15 {
 public:
  enum class I2CAddress {
    ToGND = 0x48,
    ToVDD = 0x49,
    ToSDA = 0x4A,
    ToSCL = 0x4B,
  };

  enum class InputChannel {
    A0ToA1,
    A0ToA3,
    A1ToA3,
    A2ToA3,
    A0ToGND,
    A1ToGND,
    A2ToGND,
    A3ToGND,
  };

  enum class Gain {
    TwoThirds,
    One,
    Two,
    Four,
    Eight,
    Sixteen,
  };

  virtual ~ADS1X15();
 
  bool begin() { return true; };
  bool readADCSingleEnded(InputChannel channel, int16_t* value);
  bool startComparator_SingleEnded(InputChannel channel, int16_t threshold);
  int16_t getLastConversionResults();
  void setGain(Gain gain) { _gain = gain; };
  Gain gain() const { return _gain; };
  // Time to wait until the value is ready.
  I2CAddress i2cAddress() const { return _i2cAddress; };

 protected:
  ADS1X15(TwoWire *i2cBus, I2CAddress i2cAddress);

  unsigned int conversionDelay() const;
  bool writeRegister(uint8_t reg, uint16_t value) const;
  bool readRegister(uint8_t reg, uint16_t *value) const;
  // Number of samples per seconds.
  virtual unsigned int samplePerSecond() const = 0;
  virtual uint8_t bitShift() const = 0;
  virtual uint16_t dataRateRegisterValue() const = 0;

  // Instance-specific properties
  const I2CAddress _i2cAddress;      ///< the I2C address
  TwoWire *const _i2cBus = NULL;
  Gain _gain = Gain::TwoThirds;       ///< ADC gain
};

/**************************************************************************/
/*!
    @brief  Sensor driver for the Adafruit ADS1015 ADC breakout.
*/
/**************************************************************************/
class ADS1015 : public ADS1X15 {
 public:
  enum class DataRate : uint16_t {
    SPS128,
    SPS250,
    SPS490,
    SPS920,
    SPS1600, // Default
    SPS2400,
    SPS3300,
  };

  ADS1015(TwoWire *i2cBus, I2CAddress i2cAddress = I2CAddress::ToGND);

  void setDataRate(DataRate dataRate) { _dataRate = dataRate; };
  DataRate dataRate() const { return _dataRate; };
  unsigned int samplePerSecond() const override;
  uint8_t bitShift() const override;
  uint16_t dataRateRegisterValue() const override;

 protected:
  DataRate _dataRate = DataRate::SPS1600;
};

/**************************************************************************/
/*!
    @brief  Sensor driver for the Adafruit ADS1115 ADC breakout.
*/
/**************************************************************************/
class ADS1115 : public ADS1X15 {
 public:
  enum class DataRate : uint16_t {
    SPS8,
    SPS16,
    SPS32,
    SPS64,
    SPS128, // Default
    SPS250,
    SPS475,
    SPS860,
  };

  ADS1115(TwoWire *i2cBus, I2CAddress i2cAddress = I2CAddress::ToGND);

  void setDataRate(DataRate dataRate) { _dataRate = dataRate; };
  DataRate dataRate() const { return _dataRate; };
  unsigned int samplePerSecond() const override;
  uint8_t bitShift() const override;
  uint16_t dataRateRegisterValue() const override;

 protected:
  DataRate _dataRate = DataRate::SPS128;
};

#endif  // ADS1X15_h
