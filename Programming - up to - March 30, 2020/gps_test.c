#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringSerial.h>

int main ()
{
  int serial_port;
  char dat,buff[100],GGA_code[3];
  unsigned char IsitGGAstring=0;
  unsigned char GGA_index=0;
  unsigned char is_GGA_received_completely = 0;

  if ((serial_port = serialOpen ("/dev/ttyAMA0", 9600)) < 0)		/* open serial port */
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }

  if (wiringPiSetup () == -1)							/* initializes wiringPi setup */
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    return 1 ;
  }

  while(1){

		if(serialDataAvail (serial_port) )		/* check for any data available on serial port */
		  {
			dat = serialGetchar(serial_port);		/* receive character serially */
			if(dat == '$'){
				IsitGGAstring = 0;
				GGA_index = 0;
			}
			else if(IsitGGAstring ==1){
				buff[GGA_index++] = dat;
				if(dat=='\r')
					is_GGA_received_completely = 1;
				}
			else if(GGA_code[0]=='R' && GGA_code[1]=='M' && GGA_code[2]=='C'){
				IsitGGAstring = 1;
				GGA_code[0]= 0;
				GGA_code[1]= 0;
				GGA_code[2]= 0;
				}
			else{
				GGA_code[0] = GGA_code[1];
				GGA_code[1] = GGA_code[2];
				GGA_code[2] = dat;
				}
		  }
		if(is_GGA_received_completely==1){
      int i = strcspn(buff,",");
      int a=0;
			printf("%.*s,", i, buff);
      a += (i+1);
      // for(int j = 0; j < 8;j++){
      //   i = strcspn(buff+a,",");
      //   a += (i+1);
      // }
      // i = strcspn(buff+a,",");
      // printf("%.*s,", i, buff+a);
			is_GGA_received_completely = 0;
		}
	}
	return 0;
}
