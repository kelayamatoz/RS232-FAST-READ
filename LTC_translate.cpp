#include <stdint.h>
#include "LTC_translate.h"

#define VREF 4.096
#define POW2_18 262143
#define POW2_17 131072
// Setting input range of all channels - 0V to 0.5 Vref with no gain compression (SS2 = 0, SS1 = 0, SS0 = 1)
struct Config_Word_Struct CWSTRUCT = { 7, 7, 7, 7, 7, 7, 7, 7};

// Calculates the voltage from ADC output data depending on the channel configuration
float LTC2348_voltage_calculator(int32_t data, uint8_t channel)
{
  float voltage;
  uint8_t CW;
  switch (channel)
  {
    case 0:
      CW = CWSTRUCT.LTC2348_CHAN0_CONFIG;
      break;
    case 1:
      CW = CWSTRUCT.LTC2348_CHAN1_CONFIG;
      break;
    case 2:
      CW = CWSTRUCT.LTC2348_CHAN2_CONFIG;
      break;
    case 3:
      CW = CWSTRUCT.LTC2348_CHAN3_CONFIG;
      break;
    case 4:
      CW = CWSTRUCT.LTC2348_CHAN4_CONFIG;
      break;
    case 5:
      CW = CWSTRUCT.LTC2348_CHAN5_CONFIG;
      break;
    case 6:
      CW = CWSTRUCT.LTC2348_CHAN6_CONFIG;
      break;
    case 7:
      CW = CWSTRUCT.LTC2348_CHAN7_CONFIG;
      break;
  }

  switch (CW)
  {
    case 0:
      voltage = 0;
      break;   // Disable Channel
    case 1:
      voltage = (float)data * (1.25 * VREF / 1.000) / POW2_18;
      break;
    case 2:
      {
        voltage = (float)data * (1.25 * VREF / 1.024) / POW2_17;
        if (voltage > 5.12)
          voltage -= 10.24;
        break;
      }
    case 3:
      {
        voltage = (float)data * (1.25 * VREF / 1.000) / POW2_17;
        if (voltage > 5.12)
          voltage -= 10.24;
        break;
      }
    case 4:
      voltage = (float)data * (2.50 * VREF / 1.024) / POW2_18;
      break;
    case 5:
      voltage = (float)data * (2.50 * VREF / 1.000) / POW2_18;
      break;
    case 6:
      {
        voltage = (float)data * (2.50 * VREF / 1.024) / POW2_17;
        if (voltage > 10.24)
          voltage -= 20.48;
        break;
      }
    case 7:
      {
        voltage = (float)data * (2.50 * VREF / 1.000) / POW2_17;
        if (voltage > 10.24)
          voltage -= 20.48;
        break;
      }
  }
  return voltage;
}
