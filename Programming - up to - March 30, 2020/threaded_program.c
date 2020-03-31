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
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "ssd1306_i2c.h"

//Definitions and Initializations/////////////////////////////////////////
//MPU Definitions
#define MPU_ADDRS 0x68

int fd;
int counter;
unsigned char buf[14];

//GPS Definitions
int serial_port;
char serial_char;
char serial[100];
char serial_select[3];
unsigned char selectTrue=0;
unsigned char serial_index=0;
unsigned char serial_recieved=0;

//Function Definitions////////////////////////////////////////////////////

//select i2c device
int select_i2c_Device(int fd,int addr,char *name){
    int select;
    char str[128];
    select = ioctl(fd, I2C_SLAVE, addr);

    if(select==-1){ //Error handling
       sprintf(str, "selectDevice for %s", name);
       perror(str);
    }
    return select;
}

//write to i2c device
int writeTo_i2c_Device(int fd, int reg, int val){
   int select;
   char buf[2];
   buf[0]=reg; buf[1]=val;

   select = write(fd, buf, 2);

   if(select==-1){ //Error handling
      perror("writeToDevice");
   }else if(select!=2){
      fprintf(stderr, "short write to device\n");
   }
}

//MPU Functions///////////////////////////////////////////////////////////
//initialize mpu
void MPU6050_init(){
  writeTo_i2c_Device(fd, 0x19, 0x07);
  writeTo_i2c_Device(fd, 0x6B, 0x01);
  writeTo_i2c_Device(fd, 0x1A, 0);
  writeTo_i2c_Device(fd, 0x1B, 0x00);
  writeTo_i2c_Device(fd, 0x1C, 0x00);
  writeTo_i2c_Device(fd, 0x38, 0x01);
}

//read and save mpu data
void read_MPU(){
  while(1){
    buf[0] = 0x3B;
    write(fd,buf,1);
    read(fd,buf,14);
    return buf;
  }
}

//GPS Functions///////////////////////////////////////////////////////////
//initialize gps
void GPS_init(){
  if((serial_port)=serialOpen("/dev/ttyAMA0", 9600))<0){
    fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno))
    return 1;
  }else if(wiringPiSetup()==-1){
    fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
    return 1;
  }
}

//read gps data from the RMC NMEA string
void read_GPS(){
  if(serialDataAvail(serial_port)){
    dat=serialGetchar(serial_port);
    if(dat=='$'){
      selectTrue=0;
      index=0;
    }else if(selectTrue==1){
      buff[index++]=serial_char;
      if(serial_char=='\r'){
        serial_recieved=1;
      }
    }else if(serial_select[0]=='R' && serial_select[1]=='M' && serial_select[2]=='C'){
      selectTrue=1;
      serial_select[0]=0;
      serial_select[1]=0;
      serial_select[2]=0;
    }else if(serial_recieved==1){
      return serial;
    }else{
      serial_select[0]=serial_select[1];
      serial_select[1]=serial_select[2];
      serial_select[2]=serial_char;
    }
  }
}

//File Management/////////////////////////////////////////////////////////
//create new file
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

//LCD Display/////////////////////////////////////////////////////////////
LCD_init(){
  ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
  ssd1306_clearDisplay();
  ssd1306_drawChar(30,15,'E',WHITE,5);
  ssd1306_drawChar(60,15,'I',WHITE,5);
  ssd1306_display();
}

//PROGRAM/////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
   int bus;

   if (argc > 1) bus = atoi(argv[1]);
   else bus = 1;
   sprintf(buf, "/dev/i2c-%d", bus);

   if ((fd = open(buf, O_RDWR)) < 0){
      // Open port for reading and writing on the i2c bus
      fprintf(stderr, "Failed to open i2c bus /dev/i2c-%d\n", bus);
      exit(1);
   }

   select_i2c_Device(fd, I2C_ADDR, "MPU6050");
   MPU6050_init();
   GPS_init();
   FILE *new_file = create_file();
   LCD_init();

   fclose(new_file);
   return 0;
}
