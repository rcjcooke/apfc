#ifndef __SERIALDISPLAY_H_INCLUDED__
#define __SERIALDISPLAY_H_INCLUDED__

enum class SerialDisplayType {
  ansi_vt100,
  serialMonitor
};

class SerialDisplay {

public:
  /*******************************
   * Constructors
   *******************************/
  SerialDisplay(SerialDisplayType serialDisplayType);

protected:
  /*******************************
   * Actions
   *******************************/
  // Clears down the serial display
  void clearSerialDisplay();

private:
  /*******************************
   * Member variables
   *******************************/
  // The display type
  SerialDisplayType mSerialDisplayType;

};

#endif // __SERIALDISPLAY_H_INCLUDED__