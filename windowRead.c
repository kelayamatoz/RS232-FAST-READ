#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include "rs232.h"
#include "Linduino.h"

#define WIN_SIZE 25
#define POLL_SIZE 1
#define INFO_STR_SIZE 512
#define BYTES_PER_CHNL 3
#define VREF 4.096
#define POW2_18 262143
#define POW2_17 131072
#define CHNL_CONFIG 7


struct timeval start, end;
long utime, seconds, useconds;
double readNums;
FILE *fp;

struct Config_Word_Struct
{
  uint8_t LTC2348_CHAN0_CONFIG : 7;
  uint8_t LTC2348_CHAN1_CONFIG : 7;
  uint8_t LTC2348_CHAN2_CONFIG : 7;
  uint8_t LTC2348_CHAN3_CONFIG : 7;
  uint8_t LTC2348_CHAN4_CONFIG : 7;
  uint8_t LTC2348_CHAN5_CONFIG : 7;
  uint8_t LTC2348_CHAN6_CONFIG : 7;
  uint8_t LTC2348_CHAN7_CONFIG : 7;
};
// Setting input range of all channels - 0V to 0.5 Vref with no gain compression (SS2 = 0, SS1 = 0, SS0 = 1)
struct Config_Word_Struct CWSTRUCT = { 7, 7, 7, 7, 7, 7, 7, 7};

// function signatures
void exitHandler();
float LTC2348_voltage_calculator(int32_t data, uint8_t channel);

int main()
{
  signal(SIGINT, exitHandler);
  int n,
  readBytes = 1,
  cport_nr = 0, // /dev/ttys0 is reserved for the testing port
  bdrate = 230400;

  uint8_t winBuf[WIN_SIZE+1];
  memset(winBuf, 0, WIN_SIZE);
  
  char mode[]={'8','N','1',0};
  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");
    return(0);
  }

  gettimeofday(&start, NULL);
  remove("result.log");
  fp = fopen("result.log", "ab+");

  uint8_t ch=0, lch=0, rdCh=0; // current char, last char, read char
  int winPtr=0, readSize=0, i, ii, chnlPtr=0;
//  printf("winPtr is inited to %d\n", winPtr);
  char infoStr[INFO_STR_SIZE];
  union LT_union_int32_4bytes data;
  int chnls[8];
  float chnlVolts[8];
  uint32_t chnlCodes[8];

  do
  {
    // it is fair to read one byte each time. Serial port updates way slower than the while loop
    rdCh = 0;
    n = RS232_PollComport(cport_nr, &rdCh, readBytes); 
    if (n != 0)
    {
//      printf("debug: getting a byte\n");
      readNums ++;
      lch = ch;
      ch = rdCh;
//      printf("debug: assigning to data uint32\n");
      data.LT_uint32 = 0;
//      printf("debug: after assigning to data uint32\n");
      if (lch == 59) //;
      {
        printf("debug: getting a semicolon\n");

        readSize = strlen((char*)winBuf);
        memset(infoStr, 0, INFO_STR_SIZE);
        if (readSize != WIN_SIZE)
        {
          sprintf(infoStr, "read a corrupted data seg, read %d bytes\n", readSize);
        }
        else
        {
//          printf("debug: start iterating over 24 bytes\n");
          for (i=0; i<24; i=i+3)
          {
//            printf("debug: starting assigning bettwen LT_byte and winBuf. i = %d\n", i);
            for (ii=0; i<3; i++)
            {
              data.LT_byte[2-ii] = winBuf[i+ii];
            }
            
            chnlPtr = i/BYTES_PER_CHNL;
            printf("debug: chnlPtr = %d, i = %d, BYTES_PER_CHNL = %d\n", chnlPtr, i, BYTES_PER_CHNL);
            chnls[chnlPtr] = (data.LT_uint32 & 0x38) >> 3;
            chnlCodes[chnlPtr] = (data.LT_uint32 & 0xFFFFC0) >> 6;
            chnlVolts[chnlPtr] = LTC2348_voltage_calculator(chnlCodes[chnlPtr], chnls[chnlPtr]);
            printf("debug: i = %d, chnlPtr = %d, chnl = %d, reading = %6f\n",i, chnlPtr, chnls[chnlPtr], chnlVolts[chnlPtr]);
          }

//          sprintf(infoStr, "read one data seg = %s\n", winBuf); // indicator for a successful read
            sprintf(infoStr, "c=%d, v=%6f; c=%d, v=%6f; c=%d, v=%6f; c=%d, v=%6f; c=%d, v=%6f; c=%d, v=%6f; c=%d, v=%6f; c=%d, v=%6f\n", chnls[0], chnlVolts[0], chnls[1], chnlVolts[1], chnls[2], chnlVolts[2], chnls[3], chnlVolts[3], chnls[4], chnlVolts[4], chnls[5], chnlVolts[5], chnls[6], chnlVolts[6], chnls[7], chnlVolts[7]);
        }

        fprintf(fp, infoStr);
        memset(winBuf, 0, WIN_SIZE);
        winPtr = 0;
      }
      else
      {
//        printf("debug: incrementing winPtr, before incrementing winPtr = %d\n", winPtr);
        winPtr++;
      }

//      printf("debug: assiging to winBuf, winPtr = %d, ch = %c\n", winPtr, (char)ch); 
      winBuf[winPtr] = ch;
//      printf("debug: after assiging to winBuf\n");
    }
  }
  while (1); 
}

void exitHandler()
{
  printf("exiting...\n");
  gettimeofday(&end, NULL);
  seconds = end.tv_sec - start.tv_sec;
  useconds = end.tv_usec - start.tv_usec;
  utime = seconds * 1000000 + useconds;

  double averU = utime / readNums;
  printf("Average read takes %f microseconds. Read in %f entries \n", averU, readNums);

  fclose(fp);

  exit(0);
}

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
