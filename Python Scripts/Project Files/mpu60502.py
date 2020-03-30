import smbus

class mpu6050:

    def __init__(self):
        self.bus = smbus.SMBus(1)
        self.bus.write_byte_data(0x68, 0x6B, 0x00)

    def read_i2c_word(self, register):
        high = self.bus.read_byte_data(0x68, register)
        low = self.bus.read_byte_data(0x68, register+1)
        value = (high << 8) + low
        if (value >= 0x8000):
            return -((65535-value)+1)
        else:
            return value

    def data(self):
        x = self.read_i2c_word(0x3B)
        y = self.read_i2c_word(0x3D)
        z = self.read_i2c_word(0x3F)

        return x, y, z
