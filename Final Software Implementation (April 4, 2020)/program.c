// READ ME ////////////////////////////////////////////////////////////////////
// File created on April 1, 2020
// File creator : Eduard Isaev
// Purposes : University of Regina Capstone Project, Acceleration Avionic
// Performance Capture System
//
// Utilized external libraries;
//
// WiringPi C library for serial initializations
// By : Gordon Henderson
// Version 2, GNU Lesser General Public License
//
// pthread C library for threading
// GNU Free Documentation License, Open Source
///////////////////////////////////////////////////////////////////////////////

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
#include <pthread.h>

#define MPU_ADDRS 0x68

unsigned char mpu_buff[3]; // Address and flag buffer for mpu
unsigned char serial1[1]; // Temp buffer for serial data
unsigned char serial2[5]; // Temp buffer for serial logic
unsigned char data_buff[23]; // Data buffer for storage (20 bytes of storage + 3 bytes for serial data format, not stored)
int fd; // File descriptor for mpu operation
int bus; // I2C bus initialization variable
int counter = 0; // Program iteration counter for testing and troubleshooting
int data_rdy = 0; // Write to file flag
int g_setting = 0; // Accelerometer setting variable (2g, 4g, 8g, 16g)
int g_update = 0; // Update g_setting flag
int8_t x_H,y_H,z_H,x_L,y_L,z_L; // Comparison holds for updating g_setting logic

int serial_port; // File descriptor for gps operation
int serial_start = 15; // Start byte of gps data in buffer

FILE *new_file;
int i;

int select_i2c_Device(int fd,int addr,char *name){ // Selecting address of I2C device
    int select;
    char str[128];
    select = ioctl(fd, I2C_SLAVE, addr);

    if(select==-1){ //Error handling
       sprintf(str, "selectDevice for %s", name);
       perror(str);
    }
    return select;
}

int writeTo_i2c_Device(int fd, int reg, int val){ // Write function for I2C configuration
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

void MPU6050_init(){ // MPU6050 initialization function
  writeTo_i2c_Device(fd, 0x19, 0x07); // SMPLRT_DIV register
  writeTo_i2c_Device(fd, 0x1A, 0x00); // General configuration register
  writeTo_i2c_Device(fd, 0x1B, 0x00); // Gyroscopic configuration register
  writeTo_i2c_Device(fd, 0x1C, 0x00); // Accelerometer configuration register
  writeTo_i2c_Device(fd, 0x38, 0x01); // Interrupt enable register
  writeTo_i2c_Device(fd, 0x6B, 0x01); // Power management settings register
}

void* read_MPU(){ // Function for reading MPU6050 I2C communication
  while(1){
    if(g_update==1){ // check for configuration (2g, 4g...) flag, update if conditions met
      switch(g_setting){
        case 0:
        writeTo_i2c_Device(fd, 0x1C, 0x00);
        break;
        case 1:
        writeTo_i2c_Device(fd, 0x1C, 0x08);
        break;
        case 2:
        writeTo_i2c_Device(fd, 0x1C, 0x10);
        break;
        case 3:
        writeTo_i2c_Device(fd, 0x1C, 0x18);
        break;
      }
      g_update = 0;
    }else{ // read data from the mpu
      write(fd,mpu_buff,1);
      read(fd,mpu_buff+2,1);
      if(mpu_buff[2]==1){
        write(fd,mpu_buff+1,1);
        read(fd,data_buff,14);
        counter++;
        data_rdy=1;
      }
    }
  }
  pthread_exit(NULL);
}

int GPS_init(){ // GPS initialization function
  if((serial_port=serialOpen("/dev/ttyAMA0", 9600))<0){
    fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
    return 1;
  }else {}
}

void* read_GPS(){ // Serial communication function
  while(1){
    read(serial_port,serial1,1);
    serial2[0]=*serial1;
    if(serial2[0]=='R'){
      read(serial_port,serial1,1);
      serial2[0]=*serial1;
      if(serial2[0]=='M'){
        read(serial_port,serial1,1);
        serial2[0]=*serial1;
        if(serial2[0]=='C'){
          read(serial_port,serial1,1);
          read(serial_port,serial1,1);
          while(*serial1!=','){
              data_buff[serial_start]=*serial1;
              read(serial_port,serial1,1);
              serial_start++;
          }serial_start=15;
        }
      }
    }
  }
  pthread_exit(NULL);
}

void* write_data(){ // Write buffer to data file
  while(1){
    if(data_rdy==1){
      for(i=0;i<14;i++){
        fprintf(new_file,"%d,",data_buff[i]);
      }fprintf(new_file,"%d,",g_setting);
      for(i=15;i<21;i++){
        fprintf(new_file, "%c",data_buff[i]);
      }
      fprintf(new_file,"%s\n","");
      data_rdy=0;
    }
  }
  pthread_exit(NULL);
}

void* g_config(){ // Comparator style logic function for dynamic g_setting updates
  while(1){
    if(g_update==0){
      x_H = data_buff[0];
      x_L = data_buff[1];
      y_H = data_buff[2];
      y_L = data_buff[3];
      z_H = data_buff[4];
      z_L = data_buff[5];
      if(((x_H>115)||(x_H<-115))||((y_H>115)||(y_H<-115))||((z_H>115)||(z_H<-115))){
        switch(g_setting){
          case 0:
          g_setting++;
          g_update=1;
          break;
          case 1:
          g_setting++;
          g_update=1;
          break;
          case 2:
          g_setting++;
          g_update=1;
          break;
        }
      }else if((15>x_L>-15)||(15>y_L>-15)||(15>z_L>-15)){
        switch (g_setting) {
          case 1:
          g_setting--;
          g_update=1;
          break;
          case 2:
          g_setting--;
          g_update=1;
          break;
          case 3:
          g_setting--;
          g_update=1;
          break;
        }
      }
    }
  }
  pthread_exit(NULL);
}

FILE *create_file(){ // File storage creation
  DIR *data;
  FILE *new_file;
  struct dirent *entry;
  data = opendir("./data");

  char *target;
  int num;
  int file_num = 0;
  char file_name[100] = "";

  if(data == NULL){ // check files is directory and select new file name based on existing files
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
  new_file = fopen(file_name, "w");
  closedir(data);
  return new_file;
}

int main(int argc, char **argv){

  mpu_buff[0]=0x3A; // Interrupt regiter read address
  mpu_buff[1]=0x3B; // Accelerometer data starting address

  if(argc > 1){ // Open I2C bus for communication
    bus = atoi(argv[1]);
  }else{
    bus = 1;
  }sprintf(data_buff, "/dev/i2c-%d", bus);

  if((fd = open(data_buff, O_RDWR)) < 0){ // Initialize file descriptor for I2C communication
    fprintf(stderr, "Failed to open i2c bus /dev/i2c-%d\n", bus);
    exit(1);
  }

  new_file = create_file();
  select_i2c_Device(fd, MPU_ADDRS, "MPU6050");
  MPU6050_init();
  GPS_init();

  // Thread initializations and operation
  pthread_t accel;
  pthread_t gps;
  pthread_t write;
  pthread_t g_check;


  pthread_create(&gps,NULL,read_GPS,NULL);
  pthread_create(&accel,NULL,read_MPU,NULL);
  pthread_create(&write,NULL,write_data,NULL);
  pthread_create(&g_check,NULL,g_config,NULL);

  pthread_join(write,NULL);
  pthread_join(g_check,NULL);
  pthread_join(accel,NULL);
  pthread_join(gps,NULL);
  return 0;
}
