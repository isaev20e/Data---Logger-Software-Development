import time
import serial

#class gps_data:

#    def serial_data(self):
        
ser = serial.Serial(
  
   port='/dev/ttyAMA0',
   baudrate = 9600,
   parity=serial.PARITY_NONE,
   stopbits=serial.STOPBITS_ONE,
   bytesize=serial.EIGHTBITS,
   timeout=0.1
)
counter=0

while(1):
    serial_data=ser.readline()
    print(serial_data)
    
#    if serial_data[0:6] == "$GPGGA":
#        section = serial_data.split(",")
#        return serial_data

#while(1):
#    data=gps_data()
#    print(data.serial_data())
        
