/*********************************************************************
This is an Arduino library for our Monochrome SHARP Memory Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1393

These displays use SPI to communicate, 3 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/
#ifndef ADAFRUIT_SHARPMEM_H
#define ADAFRUIT_SHARPMEM_H

#include "Adafruit_mfGFX.h"


// LCD Dimensions
#define SHARPMEM_LCDWIDTH       (400)	//(96)
#define SHARPMEM_LCDHEIGHT      (240)	//(96) 
#define LINE_BUFF_SIZE			(SHARPMEM_LCDWIDTH / 8)

//Data structure pointer offsets for FRAM
#define CLEAR		0
#define WRITE		2
#define LINENUM		3
#define DATA		4
#define TRAILER		(DATA + LINE_BUFF_SIZE)


class Adafruit_SharpMem : public Adafruit_GFX {
 public: 
  Adafruit_SharpMem(uint8_t ss);
  void begin(void);
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  uint8_t getPixel(uint16_t x, uint16_t y);
  void clearDisplay();
  void refresh(void);
  void showFRAM(void);

 private:
  uint8_t _ss, _clk, _mosi;
  //volatile uint8_t *dataport, *clkport;
  uint8_t _sharpmem_vcom;
  
  uint8_t revBits(uint8_t val);

};

#endif