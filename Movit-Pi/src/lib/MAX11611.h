/* ============================================
PROGRAMME PAR: FRANCIS BOUCHER
POUR: MOVITPLUS
DATE: SEPTEMBRE 2017 à DECEMBRE 2017

DATASHEET: https://datasheets.maximintegrated.com/en/ds/MAX11606-MAX11611.pdf
===============================================
*/

#ifndef MAX11611_H
#define MAX11611_H

#include "I2Cdev.h"

#define MAX11611_DEFAULT_ADDRESS 0x35 //0b00110101
#define BUFFER_LENGTH 32

class MAX11611
{
  public:
    MAX11611();
    MAX11611(uint8_t address);

    bool initialize();
    void getData(uint8_t nbOfAnalogDevices, uint16_t *realData);

  private:
    uint8_t devAddr;
    uint8_t buffer[14];
};

#endif /* _MAX11611_H */