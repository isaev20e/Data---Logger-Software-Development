from mpu60502 import mpu6050
import time

start_time = time.localtime()
sensor = mpu6050()
counter = 0

print(start_time)


while(counter<10000):
    counter = counter+1
    data = str(sensor.data())
    print(data)

end_time = time.localtime()
print(end_time)
