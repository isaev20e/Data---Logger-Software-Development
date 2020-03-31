/******
Demo for ssd1306 i2c driver for  Raspberry Pi
******/

#include "ssd1306_i2c.h"

void main() {

	ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

	ssd1306_display(); //Adafruit logo is visible
	ssd1306_clearDisplay();

	// char* text = "This is demo for SSD1306 i2c driver for Raspberry Pi";
	// ssd1306_drawString(text);
	// ssd1306_display();
	// delay(5000);
	//
	// ssd1306_dim(1);
	// ssd1306_startscrollright(00,0xFF);
	// delay(5000);
	//
	// ssd1306_stopscroll();
	// ssd1306_display();
	// ssd1306_clearDisplay();

		// ssd1306_clearDisplay();
		// ssd1306_drawChar(0,40,'O',WHITE,3);
		// ssd1306_drawChar(15,40,'O',WHITE,3);
		// ssd1306_drawChar(90,20,'O',WHITE,3);
		// ssd1306_drawFastHLine(20,20,70,WHITE);
		// ssd1306_drawFastHLine(20,40,70,WHITE);
		// ssd1306_display();

		ssd1306_drawChar(30,15,'E',WHITE,5);
		ssd1306_drawChar(60,15,'I',WHITE,5);
		ssd1306_display();
}
