import glob
from mpu6050 import mpu6050
import time

file_list = glob.glob("Data/*.txt")
file_list = sorted(file_list)

if file_list:
    file_list = file_list[-1]
    file_list = file_list[5:]
    file_list = file_list[:-4]
    file_list = int(file_list) + 1
else:
    file_list = 1

new_file = open("Data/" + str(file_list) + ".txt", "w+")

sensor = mpu6050(0x68)

while(1):
    accelerometer_data = str(sensor.get_accel_data())[1:-1]
    gyroscopic_data = str(sensor.get_gyro_data())[1:-1]
    time1 = time.localtime()
    time1 = time1.tm_year,time1.tm_mon,time1.tm_mday,time1.tm_hour,time1.tm_min,time1.tm_sec
    time1 = str(time1)[1:-1]
    new_file.write(accelerometer_data + ', ' + gyroscopic_data + ', ' + time1 + '\n')
    print(accelerometer_data + ', ' + gyroscopic_data + ', ' + time1)