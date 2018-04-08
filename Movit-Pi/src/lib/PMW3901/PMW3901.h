#ifndef PMW3901_H
#define PMW3901_H

#include "BCM2835\bcm2835.h"
#include <stdint.h>

class PMW3901
{
public:
  PMW3901(uint8_t cspin);

  void spi_init();

  bool begin();

  void readMotionCount(int16_t *deltaX, int16_t *deltaY);

private:
  uint8_t _cs;

  void registerWrite(uint8_t reg, uint8_t value);
  uint8_t registerRead(uint8_t reg);

  void sleepForMicroseconds(uint32_t microseconds);
  void sleepForMilliseconds(uint32_t milliseconds);

  void beginTransaction();
  void endTransaction();

  void initRegisters();
  void setupSPI();
  void initSPI();
};

#endif //PMW3901_H
