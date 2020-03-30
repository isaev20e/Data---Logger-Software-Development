#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>

#include "smbus.h"
#include "i2c-dev.h"

#include <wiringPi.h>
#include <wiringPiI2C.h>

int fd;

int main(){

  fd = wiringPiI2CSetup(0x68);
  float x;
  unsigned char values[32];

  x=wiringPiI2CReadReg8(fd,0x0D);
  printf("%.3F\n",x);
//  i2c_smbus_read_block_data(fd,0x0D,values);
//  printf("%d\n",values[0]);
}
