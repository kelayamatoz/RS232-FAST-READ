#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include "rs232.h"
#include "LTC_translate.h"

// int process
#define BUF_SIZE 8192 
// FILE *fp;

// void exitHandler(int dummy)
// {
//   printf("Exiting the program under Ctrl+C\n");
//   fclose(fp);
// }

int main()
{
  int n,
      readBytes=1024,
      cport_nr=0,        /* /dev/ttyS0 is reserved for the testing port */
      bdrate=230400;

  uint8_t buf[BUF_SIZE];
  uint8_t compBuf[BUF_SIZE];
  memset(compBuf, 0, BUF_SIZE);
  memset(buf, 0, BUF_SIZE);
  char mode[]={'8','N','1',0};

  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");
    return(0);
  }

  usleep(10000); //sleep for 10 mili to wait before start
  int lastTracker = 0;
  double readNums = 0;
  struct timeval start, end;
  long utime, seconds, useconds;
  gettimeofday(&start, NULL);

//  // only start when a delimiter is found
//  while (1)
//  {
//    char oneByteBuf[1];
//    RS232_PollComport(cport_nr, (uint8_t*)oneByteBuf, 1);
//    // printf("buf = %s\n", (char*)oneByteBuf);
//    if (*oneByteBuf == ';')
//    {
//      // printf("Start transmitting\n");
//      break;
//    }
//  }

  // use a rotating buffer to continuously store the read info
  // todo: check memory leakage!

//  remove("result.log");
//  fp = fopen("result.log", "ab+");
  RS232_PollComport(cport_nr, buf, readBytes);
  do
  {
    n = RS232_PollComport(cport_nr, buf, readBytes);
    if (n + lastTracker >= BUF_SIZE-1)
    {
      int ii;
      for (ii = lastTracker - 1; ii > 0; ii --)
      {
        if (compBuf[ii] == 59) //;
        {
          ii ++;
          break;
        }
      }

      size_t cpySize = lastTracker-ii+1;
      uint8_t* tmpBuf = (uint8_t*)malloc(cpySize);
      memset(tmpBuf, 0, cpySize); 
      memcpy(tmpBuf, compBuf+ii, cpySize);

 //     fprintf(fp, compBuf); 

      memset(compBuf, 0, BUF_SIZE);
      memcpy(compBuf, tmpBuf, cpySize);
      free(tmpBuf);
      memcpy(compBuf+cpySize, buf, n);
 
//      printf("Saturated the buffer; exiting\n");
//      printf("read compBuf content = \n");
//      printf("%s\n", (char*) compBuf);

      break;
    }
    else if (n > 0)
    {
      readNums += 1.0;
      memcpy(compBuf+lastTracker, buf, n);
      lastTracker += n;
    }
  }
  while (1);

  gettimeofday(&end, NULL);
  seconds = end.tv_sec - start.tv_sec;
  useconds = end.tv_usec - start.tv_usec;
  utime = seconds * 1000000 + useconds;

  double averU = utime / readNums;
  printf("Average read takes %f microseconds. Read in %f entries \n", averU, readNums);
  printf("%s\n", (char*) compBuf);
//  fclose(fp);
  return(0);
}

