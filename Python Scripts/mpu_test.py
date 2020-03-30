from __future__ import print_function

from mpu6050 import mpu6050
import time

start_time = time.localtime()
sensor = mpu6050(0x68)

counter = 0

print(start_time)
#print (start_time.tm_year,start_time.tm_mon,
#       start_time.tm_mday,start_time.tm_hour,
#       start_time.tm_min,start_time.tm_sec)

while (counter<10000):
    counter = counter + 1
    accelerometer_data = str(sensor.get_accel_data())[1:-1] 
    gyroscopic_data = str(sensor.get_gyro_data())[1:-1] 
    print (accelerometer_data + ", " + gyroscopic_data)
    time.sleep(0.01)
    
end_time = time.gmtime()
print (end_time)