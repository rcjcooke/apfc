/*****************
MULTIUART Arduino Library
Created By: Ben Rowland
Date: 19 August 2016
Copyright: Rowland Technology
Released into the public domain.

Update: Ray Cooke: Added readBytes method. Aligned some of the class better with Arduino code standards.

Compatible with the MULTIUART or SPI2UART module
Uses a single SPI bus to control up to four buffered hardware UART channels
*****************/

#ifndef __MULTIUART_H_INCLUDED__
#define __MULTIUART_H_INCLUDED__

#include <Arduino.h>
#include <SPI.h>

class MULTIUART {

public:

	MULTIUART(uint8_t ss);
	void initialise(int SPIDivider);
	uint8_t checkRx(char UART);
	char CheckTx(char UART);
	uint8_t ReceiveByte(char UART);
	void ReceiveString(char *RETVAL, char UART, size_t NUMBYTES);
	void transmitByte(char UART, const uint8_t DATA);
	void transmitBytes(char UART, const uint8_t *DATA, size_t NUMBYTES);
	void SetBaud(char UART, char BAUD);

	void readBytes(uint8_t *buffer, char UART, size_t length);
	
private:
	uint8_t _ss_pin;

};

#endif // __MULTIUART_H_INCLUDED__

