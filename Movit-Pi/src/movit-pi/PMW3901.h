#ifndef PMW3901_H
#define PMW3901_H

#include "bcm2835.h"
#include <stdint.h>

class PMW3901
{
public:
  ~PMW3901();

  bool Initialize();
  void ReadMotionCount(int16_t *deltaX, int16_t *deltaY);

private:
  void RegisterWrite(uint8_t reg, uint8_t value);
  uint8_t RegisterRead(uint8_t reg);

  void BeginTransaction();
  void EndTransaction();
  void SetChipSelect(uint8_t active);

  void InitRegisters();
};

#endif //PMW3901_H
