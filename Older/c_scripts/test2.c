#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <time.h>

#define I2C_ADDR 0x68

int selectDevice(int fd,int addr,char *name){
    int s;
    char str[128];
    s = ioctl(fd, I2C_SLAVE, addr);

    if(s==-1){
       sprintf(str, "selectDevice for %s", name);
       perror(str);
    }
    return s;
}

int writeToDevice(int fd, int reg, int val){
   int s;
   char buf[2];

   buf[0]=reg; buf[1]=val;
   s = write(fd, buf, 2);

   if(s==-1){
      perror("writeToDevice");
   }
   else if(s != 2){
      fprintf(stderr, "short write to device\n");
   }
}

int main(int argc, char **argv){
   int bus;
   int fd;
   unsigned char buf[14];
   int counter = 0;

   if (argc > 1) bus = atoi(argv[1]);
   else bus = 1;

   sprintf(buf, "/dev/i2c-%d", bus);

   if ((fd = open(buf, O_RDWR)) < 0){
      // Open port for reading and writing
      fprintf(stderr, "Failed to open i2c bus /dev/i2c-%d\n", bus);
      exit(1);
   }

   selectDevice(fd, I2C_ADDR, "MPU6050");

   writeToDevice(fd, 0x19, 0x07);
   writeToDevice(fd, 0x6B, 0x01);
   writeToDevice(fd, 0x1A, 0);
   writeToDevice(fd, 0x1B, 24);
   writeToDevice(fd, 0x38, 0x01);

   time_t rawtime;
   struct tm *info;
   time( &rawtime );
   info = localtime( &rawtime );
   printf("Current local time and date: %s", asctime(info));

while(1){
  buf[0] = 0x3B;

  if ((write(fd, buf, 1)) != 1){
  // Send the register to read from
  fprintf(stderr, "Error writing to MPU6050\n");
  }

  if (read(fd, buf, 4) != 4){
  fprintf(stderr, "Error reading from MPU6050\n");
  }else{}

  for(int i=0;i<4;i++){
    printf("%.0f,",(float)buf[i]);
  }
  printf("\n");
//  counter++;
  usleep(10000);
}

time( &rawtime );
info = localtime( &rawtime );
printf("Current local time and date: %s", asctime(info));

   return 0;
}
