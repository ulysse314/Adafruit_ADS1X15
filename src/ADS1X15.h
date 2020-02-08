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

#ifndef ADS1115_h
#define ADS1115_h

#include <Arduino.h>
#include <Wire.h>

/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
#define ADS1015_REG_POINTER_MASK (0x03)      ///< Point mask
#define ADS1015_REG_POINTER_CONVERT (0x00)   ///< Conversion
#define ADS1015_REG_POINTER_CONFIG (0x01)    ///< Configuration
#define ADS1015_REG_POINTER_LOWTHRESH (0x02) ///< Low threshold
#define ADS1015_REG_POINTER_HITHRESH (0x03)  ///< High threshold
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
#define ADS1015_REG_CONFIG_OS_MASK (0x8000) ///< OS Mask
#define ADS1015_REG_CONFIG_OS_SINGLE                                           \
  (0x8000) ///< Write: Set to start a single-conversion
#define ADS1015_REG_CONFIG_OS_BUSY                                             \
  (0x0000) ///< Read: Bit = 0 when conversion is in progress
#define ADS1015_REG_CONFIG_OS_NOTBUSY                                          \
  (0x8000) ///< Read: Bit = 1 when device is not performing a conversion

#define ADS1015_REG_CONFIG_MUX_MASK (0x7000) ///< Mux Mask
#define ADS1015_REG_CONFIG_MUX_DIFF_0_1                                        \
  (0x0000) ///< Differential P = AIN0, N = AIN1 (default)
#define ADS1015_REG_CONFIG_MUX_DIFF_0_3                                        \
  (0x1000) ///< Differential P = AIN0, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_1_3                                        \
  (0x2000) ///< Differential P = AIN1, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_2_3                                        \
  (0x3000) ///< Differential P = AIN2, N = AIN3
#define ADS1015_REG_CONFIG_MUX_SINGLE_0 (0x4000) ///< Single-ended AIN0
#define ADS1015_REG_CONFIG_MUX_SINGLE_1 (0x5000) ///< Single-ended AIN1
#define ADS1015_REG_CONFIG_MUX_SINGLE_2 (0x6000) ///< Single-ended AIN2
#define ADS1015_REG_CONFIG_MUX_SINGLE_3 (0x7000) ///< Single-ended AIN3

#define ADS1015_REG_CONFIG_PGA_MASK (0x0E00)   ///< PGA Mask
#define ADS1015_REG_CONFIG_PGA_6_144V (0x0000) ///< +/-6.144V range = Gain 2/3
#define ADS1015_REG_CONFIG_PGA_4_096V (0x0200) ///< +/-4.096V range = Gain 1
#define ADS1015_REG_CONFIG_PGA_2_048V                                          \
  (0x0400) ///< +/-2.048V range = Gain 2 (default)
#define ADS1015_REG_CONFIG_PGA_1_024V (0x0600) ///< +/-1.024V range = Gain 4
#define ADS1015_REG_CONFIG_PGA_0_512V (0x0800) ///< +/-0.512V range = Gain 8
#define ADS1015_REG_CONFIG_PGA_0_256V (0x0A00) ///< +/-0.256V range = Gain 16

#define ADS1015_REG_CONFIG_MODE_MASK (0x0100)   ///< Mode Mask
#define ADS1015_REG_CONFIG_MODE_CONTIN (0x0000) ///< Continuous conversion mode
#define ADS1015_REG_CONFIG_MODE_SINGLE                                         \
  (0x0100) ///< Power-down single-shot mode (default)

#define ADS1015_REG_CONFIG_DR_MASK (0x00E0)   ///< Data Rate Mask
#define ADS1015_REG_CONFIG_DR_128SPS (0x0000) ///< 128 samples per second
#define ADS1015_REG_CONFIG_DR_250SPS (0x0020) ///< 250 samples per second
#define ADS1015_REG_CONFIG_DR_490SPS (0x0040) ///< 490 samples per second
#define ADS1015_REG_CONFIG_DR_920SPS (0x0060) ///< 920 samples per second
#define ADS1015_REG_CONFIG_DR_1600SPS                                          \
  (0x0080) ///< 1600 samples per second (default)
#define ADS1015_REG_CONFIG_DR_2400SPS (0x00A0) ///< 2400 samples per second
#define ADS1015_REG_CONFIG_DR_3300SPS (0x00C0) ///< 3300 samples per second

#define ADS1015_REG_CONFIG_CMODE_MASK (0x0010) ///< CMode Mask
#define ADS1015_REG_CONFIG_CMODE_TRAD                                          \
  (0x0000) ///< Traditional comparator with hysteresis (default)
#define ADS1015_REG_CONFIG_CMODE_WINDOW (0x0010) ///< Window comparator

#define ADS1015_REG_CONFIG_CPOL_MASK (0x0008) ///< CPol Mask
#define ADS1015_REG_CONFIG_CPOL_ACTVLOW                                        \
  (0x0000) ///< ALERT/RDY pin is low when active (default)
#define ADS1015_REG_CONFIG_CPOL_ACTVHI                                         \
  (0x0008) ///< ALERT/RDY pin is high when active

#define ADS1015_REG_CONFIG_CLAT_MASK                                           \
  (0x0004) ///< Determines if ALERT/RDY pin latches once asserted
#define ADS1015_REG_CONFIG_CLAT_NONLAT                                         \
  (0x0000) ///< Non-latching comparator (default)
#define ADS1015_REG_CONFIG_CLAT_LATCH (0x0004) ///< Latching comparator

#define ADS1015_REG_CONFIG_CQUE_MASK (0x0003) ///< CQue Mask
#define ADS1015_REG_CONFIG_CQUE_1CONV                                          \
  (0x0000) ///< Assert ALERT/RDY after one conversions
#define ADS1015_REG_CONFIG_CQUE_2CONV                                          \
  (0x0001) ///< Assert ALERT/RDY after two conversions
#define ADS1015_REG_CONFIG_CQUE_4CONV                                          \
  (0x0002) ///< Assert ALERT/RDY after four conversions
#define ADS1015_REG_CONFIG_CQUE_NONE                                           \
  (0x0003) ///< Disable the comparator and put ALERT/RDY in high state (default)
/*=========================================================================*/

/** Gain settings */
typedef enum {
  GAIN_TWOTHIRDS = ADS1015_REG_CONFIG_PGA_6_144V,
  GAIN_ONE = ADS1015_REG_CONFIG_PGA_4_096V,
  GAIN_TWO = ADS1015_REG_CONFIG_PGA_2_048V,
  GAIN_FOUR = ADS1015_REG_CONFIG_PGA_1_024V,
  GAIN_EIGHT = ADS1015_REG_CONFIG_PGA_0_512V,
  GAIN_SIXTEEN = ADS1015_REG_CONFIG_PGA_0_256V
} adsGain_t;

class ADS1X15 {
 public:
  enum class I2CAddress {
    GND = 0x48,
    VDD = 0x49,
    SDA = 0x4A,
    SCL = 0x4B,
  };

  virtual ~ADS1X15();
 
  bool begin();
  bool      readADC_SingleEnded(uint8_t channel, int16_t* value);
  int16_t   readADC_Differential_0_1();
  int16_t   readADC_Differential_2_3();
  bool      startComparator_SingleEnded(uint8_t channel, int16_t threshold);
  int16_t   getLastConversionResults();
  void setGain(adsGain_t gain);
  adsGain_t getGain() const;
  // Time to wait until the value is ready.
  unsigned int conversionDelay() const;
  I2CAddress i2cAddress() const { return _i2cAddress; };

 protected:
  ADS1X15(uint8_t bitShift, uint16_t dataRateBits, I2CAddress i2cAddress, TwoWire *i2cBus);

  bool writeRegister(uint8_t reg, uint16_t value) const;
  bool readRegister(uint8_t reg, uint16_t *value) const;
  // Number of samples per seconds.
  virtual unsigned int samplePerSecond() const = 0;

  // Instance-specific properties
  uint8_t m_bitShift;                ///< bit shift amount
  const I2CAddress _i2cAddress;      ///< the I2C address
  TwoWire *const _i2cBus;
  uint16_t _dataRateBits;
  adsGain_t m_gain;                  ///< ADC gain
};

/**************************************************************************/
/*!
    @brief  Sensor driver for the Adafruit ADS1015 ADC breakout.
*/
/**************************************************************************/
class ADS1015 : public ADS1X15 {
 public:
  enum class DataRate : uint16_t {
    SPS128 = 000,
    SPS250 = 0x20,
    SPS490 = 0x40,
    SPS920 = 0x60,
    SPS1600 = 0x80, // Default
    SPS2400 = 0xA0,
    SPS3300 = 0xC0,
  };

  ADS1015(I2CAddress i2cAddress = I2CAddress::GND, TwoWire *i2cBus = &Wire);

  void setDataRate(DataRate dataRate) { _dataRateBits = (uint16_t)dataRate; };
  DataRate dataRate() const { return (DataRate)_dataRateBits; };
  unsigned int samplePerSecond() const override;
};

/**************************************************************************/
/*!
    @brief  Sensor driver for the Adafruit ADS1115 ADC breakout.
*/
/**************************************************************************/
class ADS1115 : public ADS1X15 {
 public:
  enum class DataRate : uint16_t {
    SPS8 = 000,
    SPS16 = 0x20,
    SPS32 = 0x40,
    SPS64 = 0x60,
    SPS128 = 0x80, // Default
    SPS250 = 0xA0,
    SPS475 = 0xC0,
    SPS860 = 0xE0,
  };

  ADS1115(I2CAddress i2cAddress = I2CAddress::GND, TwoWire *i2cBus = &Wire);

  void setDataRate(DataRate dataRate) { _dataRateBits = (uint16_t)dataRate; };
  DataRate dataRate() const { return (DataRate)_dataRateBits; };
  unsigned int samplePerSecond() const override;
};

#endif  // ADS1115_h
