/*
Fujitsu FRAM library, v0.0.1 Beta
By Paul Kourany, June 2014

This library is compatible with the Fujitsu MB85RS64V 64kb/8kB FRAM
device though it will work with larger devices with minor modifications

At present, the library does not support multiple devices.  It also has
functions specific for use with a Sharp Memory Display.

*/

#include "FRAM.h"

#define _select	  PIN_MAP[_cs].gpio_peripheral->BRR = PIN_MAP[_cs].gpio_pin //FRAM CS LOW
#define _deselect	PIN_MAP[_cs].gpio_peripheral->BSRR = PIN_MAP[_cs].gpio_pin //FRAM CS HIGH


// Constructor
FRAM::FRAM(uint8_t chipSelect)
{
	_cs = chipSelect;
	pinMode(_cs, OUTPUT);
	digitalWrite(_cs, HIGH);
}


// FRAMRead(addr, *buf, count)
// where addr = 16 bit start address for read
// 		 *buf = pointer to byte buffer where read data is place sequencially
//		 count = 16 bit number of bytes to read
// returns -1 if error, 0 if ok
//
// The FRAM has an auto-increment address counter so sequential reads only
// require start address to be setup which makes multi-byte reads much\
// faster than single byte reads.
int8_t FRAM::FRAMRead(uint16_t addr, uint8_t* buf, uint16_t count)
{
  if (addr > FRAM_ADDR_MAX)
	return -1;

  _select;
  
  SPI.transfer(CMD_READ);
  SPI.transfer(addr / 256);
  SPI.transfer(addr % 256);

  for (uint16_t i=0; i < count; i++)
	buf[i] = SPI.transfer(DUMMY_BYTE);

  _deselect;

  return 0;
}

// FRAMReadToSharp(addr, vlines, llenght, sharpCS)
// where addr = 16 bit start address for read
// 		 vlines = 16 bit number of vertical lines to read
//		 llength = 8 bit length of line in bytes (total pixels / 8)
//		 sharpCS = SharpMem display chipselect pin
int8_t FRAM::FRAMReadToSharp(uint16_t addr, uint16_t vlines, uint8_t llength, uint8_t sharpCS)
{
  if (addr > FRAM_ADDR_MAX)
	return -1;

  SPI.setClockDivider(SHARP_SPI_SPEED);	//Slow down SPI clock for Sharp Display

  _select;

  SPI.transfer(CMD_READ);
  SPI.transfer(addr / 256);
  SPI.transfer(addr % 256);

  PIN_MAP[sharpCS].gpio_peripheral->BSRR = PIN_MAP[sharpCS].gpio_pin;	//SHARP CS HIGH
  
  for (uint16_t i=0; i < (llength+1); i++)	//First line with WRITE command
	SPI.transfer(DUMMY_BYTE);
	
  for (uint16_t j=0; j < (vlines-1); j++) {	//Subsequent lines
	for (uint16_t k=0; k < llength; k++)
		SPI.transfer(DUMMY_BYTE);
  }

  PIN_MAP[sharpCS].gpio_peripheral->BRR = PIN_MAP[sharpCS].gpio_pin;	//SHARP CS LOW
  _deselect;
  
  SPI.setClockDivider(FRAM_SPI_SPEED);	//Speed up SPI clock for FRAM

  return 0;
}

// FRAMWrite(addr, *buf, count)
// where addr = 16 bit start address for write
// 		 *buf = pointer to byte buffer where write data is read from sequencially
//		 count = 16 bit number of bytes to write
// returns -1 if error, 0 if ok
//
// The FRAM has an auto-increment address counter so sequential writes only
// require start address to be setup which makes multi-byte writes much\
// faster than single byte writes.
int8_t FRAM::FRAMWrite(uint16_t addr, uint8_t* buf, uint16_t count)
{
  if (addr > FRAM_ADDR_MAX)
	return -1;

  _select;
  SPI.transfer(CMD_WREN);  //write enable 
  _deselect;

  _select;
  SPI.transfer(CMD_WRITE); //write command
  SPI.transfer(addr / 256);
  SPI.transfer(addr % 256);

  for (uint16_t i = 0;i < count;i++)
	SPI.transfer(buf[i]);

  _deselect;

  return 0;
}	

// FRAMisPresent()
// Returns 0 if MB85RS64V is found, -1 otherwise
//
// This function only works with the MB85RS64V part as the 256kb does not have
// an RDID register to read
int8_t FRAM::FRAMisPresent(void)
{
	uint8_t a[4] = {0,0,0,0};
	uint8_t results;
	
	_select;
	SPI.transfer(CMD_RDID);
	for (uint8_t i = 0; i < 4; i++)
		a[i] = SPI.transfer(DUMMY_BYTE);

	_deselect;

	uint16_t prodID = (a[2] << 8) + a[3];
	
	if ((a[0] != 0x04) || (prodID != 0x0302))
		return -1;
	else
		return 0;	
		
}


int8_t FRAM::checkForFRAM()
{
	// Tests that the unused status register bits can be read, inverted, written back and read again
		
	const byte srMask = 0x78; // Unused bits are bits 6..4
	byte registerValue = 0;
	byte newValue = 0;
	boolean isPresent = true;
		
	// Read current value
	_select;
	SPI.transfer(CMD_RDSR);
	registerValue = SPI.transfer(DUMMY_BYTE);
	_deselect;
		
	// Invert current value
	newValue = registerValue ^ srMask;
		
	// Write new value
	_select;
	SPI.transfer(CMD_WREN);
	_deselect;
	
	_select;
	SPI.transfer(CMD_WRSR);
	SPI.transfer(newValue);
	_deselect;
		
	// Read again
	_select;
	SPI.transfer(CMD_RDSR);
	registerValue = SPI.transfer(DUMMY_BYTE);
	_deselect;
		
	if (((registerValue & srMask) == (newValue & srMask)))
		return 0;
	else
		return -1;	
		
}