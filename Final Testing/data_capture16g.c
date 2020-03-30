#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <dirent.h>
#include <time.h>

#define I2C_ADDR 0x68
   int fd;
   unsigned char buf[14];
   int counter = 0;
   int i;

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

void MPU6050_init(){
  writeToDevice(fd, 0x19, 0x07);
  writeToDevice(fd, 0x6B, 0x01);
  writeToDevice(fd, 0x1A, 0);
  writeToDevice(fd, 0x1B, 0x00);
  writeToDevice(fd, 0x1C, 0x18);
  writeToDevice(fd, 0x38, 0x01);
}

FILE *create_file(){
  DIR *data;
  FILE *new_file;
  struct dirent *entry;
  data = opendir("./data");

  char *target;
  int num;
  int file_num = 0;
  char file_name[100] = "";

  if(data == NULL){
      file_num = 1;
  }else{
      while(entry=readdir(data)){
        target = entry->d_name;
        target[strlen(target)-4]=0;
        num = atoi(target);
        if(num>file_num){
          file_num = num;
        }
      }
      file_num++;
  }
  sprintf(file_name, "data/%d.txt",file_num);
  printf("%s\n",file_name);
  new_file = fopen(file_name, "w");
  closedir(data);
  return new_file;
}

void read_data(FILE *new_file){
  time_t rawtime;
  struct tm *info;
  time( &rawtime );
  info = localtime( &rawtime );
  fprintf(new_file, asctime(info));

  while(1){
    buf[0] = 0x3B;
    write(fd,buf,1);
    read(fd,buf,14);

    for(i=0;i<14;i++){
      fprintf(new_file,"%.0f,",(float)buf[i]);
    }
    counter++;
    if(counter%10000==0){
      time( &rawtime );
      info = localtime( &rawtime );
      fprintf(new_file, asctime(info));
    }else{
      fprintf(new_file, "%s\n","");
    }
  }
}

int main(int argc, char **argv){
   int bus;

   if (argc > 1) bus = atoi(argv[1]);
   else bus = 1;

   sprintf(buf, "/dev/i2c-%d", bus);

   if ((fd = open(buf, O_RDWR)) < 0){
      // Open port for reading and writing
      fprintf(stderr, "Failed to open i2c bus /dev/i2c-%d\n", bus);
      exit(1);
   }

   selectDevice(fd, I2C_ADDR, "MPU6050");
   MPU6050_init();
   FILE *new_file = create_file();

   read_data(new_file);

   fclose(new_file);

   return 0;
}
