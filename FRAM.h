/*
Fujitsu FRAM library, v0.0.1 Beta
By Paul Kourany, June 2014

This library is compatible with the Fujitsu MB85RS64V 64kb/8kB FRAM
device though it will work with larger devices with minor modifications

At present, the library does not support multiple devices.  It also has
functions specific for use with a Sharp Memory Display.

*/

#include "application.h"
 
#define CMD_WREN		0x06	// 0000 0110 Set Write Enable Latch
#define CMD_WRDI		0x04	// 0000 0100 Write Disable
#define CMD_RDSR		0x05	// 0000 0101 Read Status Register
#define CMD_WRSR		0x01	// 0000 0001 Write Status Register
#define CMD_READ		0x03	// 0000 0011 Read Memory Data
#define CMD_WRITE		0x02	// 0000 0010 Write Memory Data
#define CMD_RDID		0x9F	// 1001 1111 Read device ID
#define DUMMY_BYTE		0x7E	// Dummy write value for SPI read
#define FRAM_ADDR_MAX 0x7FFF	// Max address value (8091)

#define FRAM_CS_PIN D4 // FRAM chip select
#define FRAM_SPI_SPEED	SPI_CLOCK_DIV2
#define SHARP_SPI_SPEED	SPI_CLOCK_DIV16

class FRAM
{
private:

	uint8_t _cs;

public:

	FRAM(uint8_t chipSelect);
	int8_t FRAMRead(uint16_t addr, uint8_t* buf, uint16_t count);
	int8_t FRAMReadToSharp(uint16_t addr, uint16_t vlines, uint8_t llength, uint8_t sharpCS);
	int8_t FRAMWrite(uint16_t addr, uint8_t* buf, uint16_t count);
	int8_t FRAMisPresent(void);
	int8_t checkForFRAM(void);

};
