#include <Wire.h>
#include <ADS1X15.h>

// ADS1115 ads;  /* Use this for the 16-bit version */
ADS1115 ads;     /* Use thi for the 12-bit version */

void setup(void) 
{
  Serial.begin(115200);
  Serial.println("Hello!");
  
  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  Wire.begin();
  if (ads.begin()) {
    Serial.println("ADS begin with success");
  } else {
    Serial.println("ADS begin with failed");
  }
}

void loop(void) 
{
  uint16_t value;

  for (int i = 0; i < 4; i++) {
    Serial.print("AIN");
    Serial.print(i);
    Serial.print(": ");
    if (ads.readADC_SingleEnded(i, &value)) {
      Serial.println(value);
    } else {
      Serial.println("no value");
    }
  }
  Serial.println(" ");
  delay(1000);
}
