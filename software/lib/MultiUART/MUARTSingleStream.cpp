#include "MUARTSingleStream.hpp"

/*******************************
 * Constructors
 *******************************/
MUARTSingleStream::MUARTSingleStream(MULTIUART* multiUARTInstance, char intUARTIndex) {
  mIntUARTIndex = intUARTIndex;
  mMultiUARTInstance = multiUARTInstance;
}

/*******************************
 * Getters / Setters
 *******************************/
// For Debug purposes: Get the underlying MultiUART instance that this is abstracting from
MULTIUART* MUARTSingleStream::getMultiUARTInstance() {
  return mMultiUARTInstance;
}

// Get the MultiUART UART Index number that this instance abstracts
char MUARTSingleStream::getIntUARTIndex() {
  return mIntUARTIndex;
}

/*******************************
 * Actions
 *******************************/
void MUARTSingleStream::begin(unsigned long baud) {

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

uint8_t MUARTSingleStream::checkRx() {
  return mMultiUARTInstance->checkRx(mIntUARTIndex);
}

char MUARTSingleStream::checkTx() {
  return mMultiUARTInstance->CheckTx(mIntUARTIndex);
}

uint8_t MUARTSingleStream::receiveByte() {
  return mMultiUARTInstance->ReceiveByte(mIntUARTIndex);
}

void MUARTSingleStream::receiveString(char *RETVAL, size_t length) {
  mMultiUARTInstance->ReceiveString(RETVAL, mIntUARTIndex, length);
}

void MUARTSingleStream::transmitByte(uint8_t DATA) {
  mMultiUARTInstance->transmitByte(mIntUARTIndex, DATA);
}

void MUARTSingleStream::transmitBytes(const uint8_t *buffer, size_t length) {
  mMultiUARTInstance->transmitBytes(mIntUARTIndex, buffer, length);
}

int MUARTSingleStream::read() {
  return receiveByte();
}

int MUARTSingleStream::available() {
  return checkRx();
}

void MUARTSingleStream::readBytes(uint8_t *buffer, size_t length) {
  mMultiUARTInstance->readBytes(buffer, mIntUARTIndex, length);
}

// Write a byte to the stream
size_t MUARTSingleStream::write(uint8_t data) {
  transmitByte(data);
  return 1;
}

// Write a number of bytes to the stream
size_t MUARTSingleStream::write(const uint8_t *buffer, size_t size) {
  transmitBytes(buffer, size);
  return size;
}



