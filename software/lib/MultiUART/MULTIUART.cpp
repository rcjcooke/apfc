/*****************
MULTIUART Arduino Library
Created By: Ben Rowland
Date: 19 August 2016
Copyright: Rowland Technology
Released into the public domain.

Compatible with the MULTIUART or SPI2UART module
Uses a single SPI bus to control up to four buffered hardware UART channels
*****************/

#include "MULTIUART.hpp"

MULTIUART::MULTIUART(uint8_t ss)
{
  pinMode(ss, OUTPUT);
  _ss_pin = ss;
}


/*=----------------------------------------------------------------------=*\
   Use :Initialises the SPI peripheral.
\*=----------------------------------------------------------------------=*/
void MULTIUART::initialise(int SPIDivider)
{
	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPIDivider);
}


/*=----------------------------------------------------------------------=*\
   Use :Returns the number of received bytes held in queue for the selected channel.
       :Parameters for macro CheckRx:
       :  UART : UART Index Range: 0-3
       :Returns : char
\*=----------------------------------------------------------------------=*/
uint8_t MULTIUART::checkRx(char UART) {
	
	uint8_t retVal = 0;

	if (UART < 4)
	{
		digitalWrite(_ss_pin, LOW);
		SPI.transfer(0x10 | UART);
		// delayMicroseconds(250);
		retVal = SPI.transfer(0xFF);
		digitalWrite(_ss_pin, HIGH);
		// delayMicroseconds(50);
	}

	return retVal;
}

/*=----------------------------------------------------------------------=*\
   Use :Returns the number of bytes held in the transmit queue for the selected channel.
       :Parameters for macro CheckTx:
       :  UART : UART Index Range: 0-3
       :Returns : char
\*=----------------------------------------------------------------------=*/
char MULTIUART::CheckTx(char UART)
{
	char RETVAL = 0;

	if (UART < 4)
	{
		digitalWrite(_ss_pin, LOW);
		SPI.transfer(0x30 | UART);
		// delayMicroseconds(250);
		RETVAL = SPI.transfer(0xFF);
		digitalWrite(_ss_pin, HIGH);
		// delayMicroseconds(50);
	}

	return (RETVAL);
}




/*=----------------------------------------------------------------------=*\
   Use :Returns the first received byte held in queue for the selected channel.
       :Recommended to use the CheckRx macro first to collect number of bytes.
       :Parameters for macro ReceiveByte:
       :  UART : UART Index Range: 0-3
       :Returns : char - Range 0-255
\*=----------------------------------------------------------------------=*/
uint8_t MULTIUART::ReceiveByte(char UART)
{
	uint8_t RETVAL;

	if (UART < 4)
	{
		digitalWrite(_ss_pin, LOW);
		SPI.transfer(0x20 | UART);
		// delayMicroseconds(50);
		SPI.transfer(1);
		// delayMicroseconds(50);
		RETVAL = SPI.transfer(0xFF);
		digitalWrite(_ss_pin, HIGH);
		// delayMicroseconds(50);
	}

	return (RETVAL);

}


/*=----------------------------------------------------------------------=*\
   Use :Returns a string of received bytes held in queue for the selected channel.
       :Recommended to use the CheckRx macro first to collect number of bytes.
       :Parameters for macro ReceiveString:
       :  UART : UART Index Range: 0-3
       :  NumBytes : char
       :Returns : MX_CHAR
\*=----------------------------------------------------------------------=*/
void MULTIUART::ReceiveString(char *RETVAL, char UART, size_t NUMBYTES) {
	// In Arduino land, uint8_t and char are the same size, so just casting one to the other.
	readBytes((uint8_t*) RETVAL, UART, NUMBYTES);
	// Add C-style string terminator
	RETVAL[NUMBYTES] = 0;
}

void MULTIUART::readBytes(uint8_t *buffer, char UART, size_t length) {
	
	if (UART < 4) {
		unsigned int index = 0;
		digitalWrite(_ss_pin, LOW);
		SPI.transfer(0x20 | UART);
		// delayMicroseconds(50);
		SPI.transfer(length);
		// delayMicroseconds(50);
		while ((index < length))
		{
			buffer[index] = SPI.transfer(0xFF);
			// delayMicroseconds(50);
			index++;
		}
		digitalWrite(_ss_pin, HIGH);
		// delayMicroseconds(50);
	}

}


/*=----------------------------------------------------------------------=*\
   Use :Adds a byte to the transmit queue for the selected channel.
       :Parameters for macro TransmitByte:
       :  UART : UART Index Range: 0-3
       :  Data : char
\*=----------------------------------------------------------------------=*/
void MULTIUART::transmitByte(char UART, const uint8_t DATA)
{
	if (UART < 4)
	{
		digitalWrite(_ss_pin, LOW);
		SPI.transfer(0x40 | UART);
		// delayMicroseconds(50);
		SPI.transfer(1);
		// delayMicroseconds(50);
		SPI.transfer(DATA);
		digitalWrite(_ss_pin, HIGH);
		// delayMicroseconds(50);
	}
}



/*=----------------------------------------------------------------------=*\
   Use :Adds a byte to the transmit queue for the selected channel.
       :Parameters for macro TransmitString:
       :  UART : UART Index Range: 0-3
       :  Data[20] : MX_CHAR (by-ref)
\*=----------------------------------------------------------------------=*/
void MULTIUART::transmitBytes(char UART, const uint8_t *DATA, size_t NUMBYTES)
{
	unsigned int index = 0;

	if (UART < 4)
	{
		digitalWrite(_ss_pin, LOW);
		SPI.transfer(0x40 | UART);
		// delayMicroseconds(50);
		SPI.transfer(NUMBYTES);
		// delayMicroseconds(50);
		while (index < NUMBYTES)
		{
			SPI.transfer(DATA[index]);
			// delayMicroseconds(50);
			index++;
		}
		digitalWrite(_ss_pin, HIGH);
		// delayMicroseconds(50);
	}
}




/*=----------------------------------------------------------------------=*\
   Use :Configures the baud rate of the selected channel.
       :Parameters for macro SetBaud:
       :  UART : UART Index Range: 0-3
       :  Baud : 0=1200, 1=2400, 2=4800, 3=9600, 4=19200, 5=38400, 6=57600, 7=115200, 8=31250, 9=62500
\*=----------------------------------------------------------------------=*/
void MULTIUART::SetBaud(char UART, char BAUD)
{
	if (UART < 4)
	{
		if (BAUD < 10)
		{
			digitalWrite(_ss_pin, LOW);
			SPI.transfer(0x80 | UART);
			// delayMicroseconds(50);
			SPI.transfer(BAUD);
			digitalWrite(_ss_pin, HIGH);
			// delayMicroseconds(50);
		}
		delay(20);                // waits for 20ms - time for flash erase and write
	}
}

