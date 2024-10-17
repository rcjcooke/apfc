#ifndef __MUARTSINGLE_H_INCLUDED__
#define __MUARTSINGLE_H_INCLUDED__

#include "MULTIUART.h"

class MUARTSingle {

public:

  /*******************************
   * Constructors
   *******************************/
  MUARTSingle(MULTIUART* multiUARTInstance, char intUARTIndex);

  /*******************************
   * Getters / Setters
   *******************************/
  void begin(unsigned long baud);

  /*******************************
   * Actions
   *******************************/
  char checkRx();
  char checkTx();
  uint8_t receiveByte();
  void receiveString(uint8_t *RETVAL, char NUMBYTES);
  void transmitByte(uint8_t DATA);
  void transmitString(uint8_t *DATA, char NUMBYTES);

  int read();
  void readBytes(uint8_t *buffer, size_t length);
  int available();

private:

  // The MultiUART Index number that this instance interfaces with
  char mIntUARTIndex;
  // The instance of the MultiUART board that this UART interface is on
  MULTIUART* mMultiUARTInstance;

};

#endif // __MUARTSINGLE_H_INCLUDED__