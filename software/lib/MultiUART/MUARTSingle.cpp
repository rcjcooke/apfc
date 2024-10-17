#include "MUARTSingle.hpp"

/*******************************
 * Constructors
 *******************************/
MUARTSingle::MUARTSingle(MULTIUART* multiUARTInstance, char intUARTIndex) {
  mIntUARTIndex = intUARTIndex;
  mMultiUARTInstance = multiUARTInstance;
}

/*******************************
 * Actions
 *******************************/
void MUARTSingle::begin(unsigned long baud) {

  // Initialise the UART baud rates
  // 0=1200, 1=2400, 2=4800, 3=9600, 4=19200, 5=38400, 6=57600, 7=115200
  char baudCode = 3;
  switch(baud) {
    case 1200:
      baudCode = 0;
      break;
    case 2400:
      baudCode = 1;
      break;
    case 4800:
      baudCode = 2;
      break;
    case 9600:
      baudCode = 3;
      break;
    case 19200:
      baudCode = 4;
      break;
    case 38400:
      baudCode = 5;
      break;
    case 57600:
      baudCode = 6;
      break;
    case 115200:
      baudCode = 7;
      break;
    default:
      // If all else fails, default to 9600 as this is a fairly common standard
      baudCode = 3;
      break;
  }

  mMultiUARTInstance->SetBaud(mIntUARTIndex, baudCode);
}

char MUARTSingle::checkRx() {
  return mMultiUARTInstance->CheckRx(mIntUARTIndex);
}

char MUARTSingle::checkTx() {
  return mMultiUARTInstance->CheckTx(mIntUARTIndex);
}

uint8_t MUARTSingle::receiveByte() {
  return mMultiUARTInstance->ReceiveByte(mIntUARTIndex);
}

void MUARTSingle::receiveString(uint8_t *RETVAL, char NUMBYTES) {
  mMultiUARTInstance->ReceiveString(RETVAL, mIntUARTIndex, NUMBYTES);
}

void MUARTSingle::transmitByte(uint8_t DATA) {
  mMultiUARTInstance->TransmitByte(mIntUARTIndex, DATA);
}

void MUARTSingle::transmitString(uint8_t *DATA, char NUMBYTES) {
  mMultiUARTInstance->TransmitString(mIntUARTIndex, DATA, NUMBYTES);
}

int MUARTSingle::read() {
  return receiveByte();
}

int MUARTSingle::available() {
  return checkRx();
}

