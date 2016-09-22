#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "rs232.h"

int main()
{
  int n,
      cport_nr=0,        /* /dev/ttyS0 is reserved for the testing port */
      bdrate=230400; 

  uint8_t buf[8192];
  uint8_t compBuf[8192];
  memset(compBuf, 0, 8192);
  memset(buf, 0, 8192);
  char mode[]={'8','N','1',0};

  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");
    return(0);
  }

  int lastTracker = 0;
  do
  {
    n = RS232_PollComport(cport_nr, buf, 4095);
    if (n + lastTracker >= 8191)
    {
      compBuf[8192] = 0;
      printf("Saturated the buffer; exiting\n");
      printf("read compBuf content = %s", compBuf);
      break;
    }
    else
    {
      memcpy(compBuf+lastTracker, buf, n);
      lastTracker += n;
    }
    
    // usleep(100);
  }

  while (1);
  return(0);
}

