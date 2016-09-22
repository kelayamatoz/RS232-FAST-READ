
#ifndef LTC2348_H
#define LTC2348_H

struct Config_Word_Struct
{
  uint8_t LTC2348_CHAN0_CONFIG : 3;
  uint8_t LTC2348_CHAN1_CONFIG : 3;
  uint8_t LTC2348_CHAN2_CONFIG : 3;
  uint8_t LTC2348_CHAN3_CONFIG : 3;
  uint8_t LTC2348_CHAN4_CONFIG : 3;
  uint8_t LTC2348_CHAN5_CONFIG : 3;
  uint8_t LTC2348_CHAN6_CONFIG : 3;
  uint8_t LTC2348_CHAN7_CONFIG : 3;
};

float LTC2348_voltage_calculator(int32_t data,
                                 uint8_t channel
                                );
#endif
