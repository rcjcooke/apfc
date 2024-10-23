#ifndef __MUARTSINGLE_H_INCLUDED__
#define __MUARTSINGLE_H_INCLUDED__

#include <Arduino.h>

#include "MULTIUART.hpp"

class MUARTSingleStream : public Stream {

public:

  /*******************************
   * Constructors
   *******************************/
  MUARTSingleStream(MULTIUART* multiUARTInstance, char intUARTIndex);

  /*******************************
   * Getters / Setters
   *******************************/
  // For Debug purposes: Get the underlying MultiUART instance that this is abstracting from
  MULTIUART* getMultiUARTInstance();
  // Get the MultiUART UART Index number that this instance abstracts
  char getIntUARTIndex();

  /*******************************
   * Actions
   *******************************/
  void begin(unsigned long baud);
  uint8_t checkRx();
  char checkTx();
  uint8_t receiveByte();
  void receiveString(char *RETVAL, size_t length);
  void transmitByte(uint8_t DATA);
  void transmitBytes(const uint8_t *buffer, size_t length);

  /* Stream class virtual function implementations */
  // Read a single character from the stream
  int read();
  // Read a specified number of bytes in to buffer
  void readBytes(uint8_t *buffer, size_t length);
  // How many characters are available to read
  int available();
  // Write a byte to the stream, returns number of bytes transmitted
  size_t write(uint8_t);
  // Write a number of bytes to the stream, returns number of bytes transmitted
  size_t write(const uint8_t *buffer, size_t size);

  // NOTE: THERE IS NO PEEK SUPPORT. THIS ALSO INVALIDATES ALL PARSE FUNCTIONS IN THE Stream CLASS.
  int peek() {return 0;};
  int timedPeek() {return -1;};
  int peekNextDigit(LookaheadMode lookahead, bool detectDecimal) {return -1;};
  long parseInt(LookaheadMode lookahead, char ignore) {return 0;};
  float parseFloat(LookaheadMode lookahead, char ignore) {return 0;};
  long parseInt(char ignore) {return 0;}
  float parseFloat(char ignore) {return 0;}

private:

  // The MultiUART Index number that this instance interfaces with
  char mIntUARTIndex;
  // The instance of the MultiUART board that this UART interface is on
  MULTIUART* mMultiUARTInstance;

};

#endif // __MUARTSINGLE_H_INCLUDED__