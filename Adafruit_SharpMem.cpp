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

// Adapted to use FRAM as display buffer
#include "Adafruit_SharpMem.h"
#include "FRAM.h"

/**************************************************************************
    Sharp Memory Display Connector
    -----------------------------------------------------------------------
    Pin   Function        Notes
    ===   ==============  ===============================
      1   VIN             3.3-5.0V (into LDO supply)
      2   3V3             3.3V out
      3   GND
      4   SCLK            Serial Clock
      5   MOSI            Serial Data Input
      6   CS              Serial Chip Select
      9   EXTMODE         COM Inversion Select (Low = SW clock/serial)
      7   EXTCOMIN        External COM Inversion Signal
      8   DISP            Display On(High)/Off(Low)

 **************************************************************************/

#define SHARPMEM_BIT_WRITECMD   (0x80)
#define SHARPMEM_BIT_VCOM       (0x40)
#define SHARPMEM_BIT_CLEAR      (0x20)
#define TOGGLE_VCOM             do { _sharpmem_vcom = _sharpmem_vcom ? 0x00 : SHARPMEM_BIT_VCOM; } while(0);

FRAM fram(D3);

/* ************* */
/* CONSTRUCTORS  */
/* ************* */
Adafruit_SharpMem::Adafruit_SharpMem(uint8_t ss) :
Adafruit_GFX(SHARPMEM_LCDWIDTH, SHARPMEM_LCDHEIGHT) {
  _ss = ss;

// Set pin state before direction to make sure they start this way (no glitching)
  pinMode(_ss, OUTPUT);
  digitalWrite(_ss, LOW);  

  // Set the vcom bit to a defined state
  _sharpmem_vcom = SHARPMEM_BIT_VCOM;

}

void Adafruit_SharpMem::begin() {
//  if (fram.checkForFRAM() != 0) {
//	digitalWrite(D7,HIGH);
//	Serial.println("FRAM not detected");
//	while(1) SPARK_WLAN_Loop();
//	}

  uint8_t i;
  uint8_t commands[3] = {0x20, 0x00, 0x80}; //clear, trailer, write multi line
  uint8_t lineBuff[LINE_BUFF_SIZE+2];

  //Initialize the FRAM command bytes
  fram.FRAMWrite(0, commands, 3);	//commands are LSB first

  for (i = 0; i < (LINE_BUFF_SIZE+1); i++)  // Fill line buffer
	lineBuff[i] = 0xFF;

  lineBuff[i] = 0x00;	//add trailer byte
	
  for (uint8_t j = 0; j < SHARPMEM_LCDHEIGHT; j++) {
    lineBuff[0] = revBits(j + 1);  //line number, starting at 1, LSBFIRST
	fram.FRAMWrite((3+(j * (LINE_BUFF_SIZE+2))), lineBuff, (LINE_BUFF_SIZE+2));
	}

  setRotation(2);
}



/* *************** */
/* PRIVATE METHODS */
/* *************** */

uint8_t Adafruit_SharpMem::revBits(uint8_t val)
{
  uint8_t ret = (uint8_t)(__RBIT(val) >> 24);
  return ret;
}


void Adafruit_SharpMem::showFRAM(void)
{
  uint16_t i;
  uint8_t lineBuff[LINE_BUFF_SIZE+2];
  uint8_t c[3];

  fram.FRAMRead(0, c, 3);
  Serial.print("CLEAR+WRITE: ");
  for (i = 0; i <3;i++) {
	Serial.print(c[i], BIN);
	Serial.print(", ");
	}
  Serial.println();
 
  for (i = 0; i < SHARPMEM_LCDHEIGHT; i++) {
	fram.FRAMRead((3+(i * (LINE_BUFF_SIZE+2))), lineBuff, (LINE_BUFF_SIZE+2));
	Serial.print("Line ");
	Serial.print(i+1);
	Serial.print(": ");
	for (uint8_t j = 0; j < (LINE_BUFF_SIZE+2);j++) {
		Serial.print(lineBuff[j], HEX);
		Serial.print(", ");
		}
	Serial.println();
	}
	
	fram.FRAMRead((LINENUM+(SHARPMEM_LCDHEIGHT * (LINE_BUFF_SIZE+2))), c, 1);
	Serial.print("End trailer: ");
	Serial.println(c[0]);
}

/* ************** */
/* PUBLIC METHODS */
/* ************** */

/**************************************************************************/
/*! 
    @brief Draws a single pixel in image buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)
*/
/**************************************************************************/
void Adafruit_SharpMem::drawPixel(int16_t x, int16_t y, uint16_t color) 
{
  uint8_t c;
  
  if ((x >= SHARPMEM_LCDWIDTH) || (y >= SHARPMEM_LCDHEIGHT))
    return;

  unsigned int addr = (y * (LINE_BUFF_SIZE+2)) + ((x / 8) + DATA);

  fram.FRAMRead(addr, &c, 1);
  c = revBits(c);
  
  if (color)
	c |= (1 << x % 8);
  else
	c &= ~(1 << x % 8);

  c = revBits(c);
  fram.FRAMWrite(addr, &c, 1);

}

/**************************************************************************/
/*! 
    @brief Gets the value (1 or 0) of the specified pixel from the buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)

    @return     1 if the pixel is enabled, 0 if disabled
*/
/**************************************************************************/
uint8_t Adafruit_SharpMem::getPixel(uint16_t x, uint16_t y)
{
  uint8_t c;
  
  if ((x >=SHARPMEM_LCDWIDTH) || (y >=SHARPMEM_LCDHEIGHT))
	return 0;

  unsigned int addr = (y * (LINE_BUFF_SIZE+2)) + ((x / 8) + DATA);
  
  fram.FRAMRead(addr, &c, 1);
  c = revBits(c);
  return (c & ((1 << x % 8)) ? 1 : 0);
}

/**************************************************************************/
/*! 
    @brief Clears the screen
*/
/**************************************************************************/
void Adafruit_SharpMem::clearDisplay() 
{
  uint16_t i;
  uint8_t lineBuff[LINE_BUFF_SIZE+2];
  
  for (i = 0; i < (LINE_BUFF_SIZE+1); i++)  // Fill line buffer
	lineBuff[i] = 0xFF;

  lineBuff[i] = 0x00;	//add trailer byte


  for (i = 0; i < SHARPMEM_LCDHEIGHT; i++) {
    lineBuff[0] = revBits(i + 1);  //line number, starting at 1, LSBFIRST
	fram.FRAMWrite((3+(i * (LINE_BUFF_SIZE+2))), lineBuff, (LINE_BUFF_SIZE+2));
	}

  uint8_t c = _sharpmem_vcom | SHARPMEM_BIT_CLEAR;
  fram.FRAMWrite(0, &c, 1);
  TOGGLE_VCOM;

  // "Read" data from FRAM, bit order is irrelevant
  fram.FRAMReadToSharp(CLEAR, 1, 2, _ss);
}

/**************************************************************************/
/*! 
    @brief Renders the contents of the pixel buffer on the LCD
*/
/**************************************************************************/
void Adafruit_SharpMem::refresh(void) 
{
  uint8_t c = _sharpmem_vcom | SHARPMEM_BIT_WRITECMD;

  PIN_MAP[D7].gpio_peripheral->BSRR = PIN_MAP[D7].gpio_pin; //HIGH

  fram.FRAMWrite(WRITE, &c, 1);
  TOGGLE_VCOM;
    
  // "Read" data from FRAM, bit order is irrelevant
  fram.FRAMReadToSharp(WRITE, SHARPMEM_LCDHEIGHT, (LINE_BUFF_SIZE + 2), _ss);

  PIN_MAP[D7].gpio_peripheral->BRR = PIN_MAP[D7].gpio_pin; //LOW
}
