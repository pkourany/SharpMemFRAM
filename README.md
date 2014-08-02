SharpMemory display buffer using FRAM
=====================================

Overview
--------

The FRAM/SharpMem display library uses a Fujitsu (8k x 8) FRAM instead of RAM, which is limited
on the Spark, to create a display buffer.  All pixel draw commands are performed
on the FRAM display buffer instead of RAM.

Both the FRAM and SharpMem display use the SPI bus though the FRAM can run at DIV4 clock (9MHz) and
the SharpMem at DIV16 (2.25MHz).  The SharpMem display requires line or multi-line (whole-screen) data
writes.  To refresh the entire display using a multi-line write, the line data structure is as follows:

```
first line:		command|line#|...data...|trailer (=0)	(command bits = 1V000000, V=VCOM bit)
other lines:	line#|...data...|0 (trailer)
last line:		line#|...data...|0|0 (trailer)

command:	8 bits
line#:		8 bits
Data bits:	leftmost pixel first, data width depending on horizontal resolution of panel
trailer:	8 or 16 "zero" bits
```

VCOM is required to maintain the display and must be toggled at a maximum 1sec interval.  In this library,
it is toggled on each line write.

Each line has DISPLAY_WIDTH (in pixels) / 8 bytes per line.  For example, a 128x128 display has 16 bytes
per line and a display buffer of 16*128 = 2048 bytes.  The largest SharpMem display is 400x240 pixels and
will require a 400*240/8 = 12,000 byte display buffer.

Typically, display operations are done in FRAM and when the display is refreshed, data is read from the
FRAM one line at a time and written to the SharpMem display, requiring a lot of SPI cycles.  In order to
speed up display processing, a method proposed by @timb was used instead.

By connecting the MOSI line (data in) of the display to the MISO line (data out) of the FRAM and using
the chipselect on both devices, data read from the FRAM can be simultaneously written to the display
in a single operation.  The catch is that with this configuration, the Spark cannot access the display
directly!  In order to get around this, commands and display lines are pre-structured in the FRAM
display buffer and sent to the display via FRAM reads with the display chipselect enabled.  This done
in the FRAMReadToSharp() function.

The Adafruit_SharpMem.cpp display driver has been substantially modified to construct the display buffer
and to operate on FRAM data in the correct bit order (as required by the display). 

Connections
-----------

Spark		SharpMem	FRAM
3.3v		3.3v		3.3v
GND			GND			GND
MOSI		-			MOSI
MISO		DI			MISO
SCK			SCK			SCK
SS						SS
D3			CS


