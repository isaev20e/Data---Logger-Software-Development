#include <wiringPiI2C.h>
#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <time.h>
#include "mywiring.h"

#define Device_Address 0x68	/*Device Address/Identifier for MPU6050*/

#define PWR_MGMT_1   0x6B
#define SMPLRT_DIV   0x19
#define CONFIG       0x1A
#define GYRO_CONFIG  0x1B
#define INT_ENABLE   0x38
#define ACCEL_XOUT_H 0x3B
#define ACCEL_YOUT_H 0x3D
#define ACCEL_ZOUT_H 0x3F
#define GYRO_XOUT_H  0x43
#define GYRO_YOUT_H  0x45
#define GYRO_ZOUT_H  0x47

int fd;

void MPU6050_Init(){

	wiringPiI2CWriteReg8 (fd, SMPLRT_DIV, 0x07);	/* Write to sample rate register */
	wiringPiI2CWriteReg8 (fd, PWR_MGMT_1, 0x01);	/* Write to power management register */
	wiringPiI2CWriteReg8 (fd, CONFIG, 0);		/* Write to Configuration register */
	wiringPiI2CWriteReg8 (fd, GYRO_CONFIG, 24);	/* Write to Gyro Configuration register */
	wiringPiI2CWriteReg8 (fd, INT_ENABLE, 0x01);	/*Write to interrupt enable register */

	}

int main(){

	float x,y,z;
	int counter = 0;
	fd = wiringPiI2CSetup(Device_Address);   /*Initializes I2C with device Address*/
	MPU6050_Init();		                 /* Initializes MPU6050 */

	time_t rawtime;
	struct tm *info;
	time( &rawtime );
	info = localtime( &rawtime );
	printf("Current local time and date: %s", asctime(info));

	while(counter<10000)
	{
		/*Read raw value of Accelerometer and gyroscope from MPU6050*/
		x = wiringPiI2CReadReg32(fd,ACCEL_XOUT_H);
		y = wiringPiI2CReadReg32(fd,ACCEL_ZOUT_H);
		z = wiringPiI2CReadReg32(fd,GYRO_YOUT_H);

		printf("%.3f,%.3f,%.3f\n",x,y,z);
		counter++;
	}

	time( &rawtime );
	info = localtime( &rawtime );
	printf("Current local time and date: %s", asctime(info));

	return 0;
}
