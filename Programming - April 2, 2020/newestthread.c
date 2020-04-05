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
#include <pthread.h>

//Definitions and Initializations/////////////////////////////////////////
//MPU definitions and initializations
#define MPU_ADDRS 0x68

int fd;
int counter;
unsigned char buf[14];
unsigned char buf_hold[14];
int i;
FILE *new_file;
int accel_run = 1;
int updated;

//GPS definitions and initializations
int serial_port;
unsigned char serial[10];
unsigned char serial_time[10];
unsigned char serial_date[10];
int serial_read = 1;
char *serial_ptr;
int serial_run;

//LCD definitions and initializations
char* accell_text = "ACCELL ";
char* gps_text = "GPS ";

int lcd_y=0;
int lcd_x=0;
int text_size=2;
int wrap_size=1;
int color=1;

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
void* read_MPU(){
  while(counter<10000){
    buf[0] = 0x3B;
    write(fd,buf,1);
    read(fd,buf,14);
    while(updated){}//Wait for data to be saved before overwriting
    for(i=0;i<14;i++){
      buf_hold[i] = buf[i];
    }
    counter++;
    updated = 1;
  }
  accel_run=0;
  pthread_exit(NULL);
}

//GPS Functions///////////////////////////////////////////////////////////
//initialize gps
int GPS_init(){
  if((serial_port=serialOpen("/dev/ttyAMA0", 9600))<0){
    fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
    return 1;
  }else {}
}

//read gps data from the RMC NMEA string
void* read_GPS(){
  while(serial_run){
      read(serial_port,serial,10);
      if(serial_ptr = strstr(serial,"$GPRMC")){
        read(serial_port,serial,10);
      for(i=0;i<10;i++){
        serial_time[i]=serial[i];
      }
        read(serial_port,serial,10);
        read(serial_port,serial,10);
      for(i=0;i<10;i++){
        serial_date[i]=serial[i];
      }
      serial_run=0;
    }
  }
  pthread_exit(NULL);
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
  new_file = fopen(file_name, "w");
  closedir(data);
  return new_file;
}

//LCD Display/////////////////////////////////////////////////////////////
//LCD initialization
void LCD_init(){
  ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
  ssd1306_display();
  delay(1000);
  ssd1306_clearDisplay();
  ssd1306_drawChar(30,15,'E',WHITE,5);
  ssd1306_drawChar(60,15,'I',WHITE,5);
  ssd1306_display();
}

//Changed write function
void lcd_write(int c, int color){
	if (c == '\n') {
		lcd_y += text_size * 8;
		lcd_x = 0;
	} else if (c == '\r') {
	} else {
		ssd1306_drawChar(lcd_x, lcd_y, c, color, text_size);
		lcd_x += text_size * 6;
		if (wrap_size && (lcd_x > (WIDTH - text_size * 6))) {
			lcd_y += text_size * 8;
			lcd_x = 0;
		}
	}
}

//Changed draw string function
void lcd_drawString(char *str, int color){
	int i, end;
	end = strlen(str);
	for (i = 0; i < end; i++){
      lcd_write(str[i], color);
  }
}

//display accellerometer
void accell_display(_Bool check, int color){
  if(check){
    lcd_y=15;
    lcd_drawString(accell_text,color);
    ssd1306_display();
  }
}

//display GPS
void gps_display(_Bool check, _Bool fix, int color){
  if(check && fix){
    lcd_y=35;
    lcd_x=0;
    lcd_drawString(gps_text,color);
    ssd1306_display();
  }
}

//indicator
void display_indicator(int x, int y, int w, int h, int fillcolor){
  ssd1306_fillRect(x, y, w, h, fillcolor);
  ssd1306_display();
}

//PROGRAM/////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

    int bus;
    new_file = create_file();
    updated = 0;
    serial_run=1;

    if (argc > 1) bus = atoi(argv[1]);
    else bus = 1;
    sprintf(buf, "/dev/i2c-%d", bus);

    if ((fd = open(buf, O_RDWR)) < 0){
      // Open port for reading and writing on the i2c bus
      fprintf(stderr, "Failed to open i2c bus /dev/i2c-%d\n", bus);
      exit(1);
    }

    select_i2c_Device(fd, MPU_ADDRS, "MPU6050");
    MPU6050_init();
    GPS_init();

    pthread_t accell;
    pthread_t gps;

    pthread_create(&gps,NULL,read_GPS,NULL);
    pthread_join(gps,NULL);
    fprintf(new_file,"%s%s\n",serial_time,serial_date);

    pthread_create(&accell, NULL, read_MPU, NULL);

    while(accel_run){
       if((updated==1) && (counter%1000==0) && (counter>100)){
         pthread_create(&gps, NULL, read_GPS, NULL);
         for(i=0;i<14;i++){
           fprintf(new_file,"%.0f,",(float)buf_hold[i]);
         }
         pthread_join(gps,NULL);
         updated=0;
         fprintf(new_file,"%s%s\n",serial_time,serial_date);
       }else if((updated==1)){
         for(i=0;i<14;i++){
           fprintf(new_file,"%.0f,",(float)buf_hold[i]);
         }fprintf(new_file, "%s\n","");
         updated=0;
       }else{}
     }
     pthread_join(accell, NULL);
     fclose(new_file);
   return 0;
}
