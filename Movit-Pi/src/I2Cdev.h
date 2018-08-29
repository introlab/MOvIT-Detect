// I2Cdev library collection - Main I2C device class
// Abstracts bit and byte I2C R/W functions into a convenient class
// RaspberryPi bcm2835 library port: bcm2835 library available at http://www.airspayce.com/mikem/bcm2835/index.html
// Based on Arduino's I2Cdev by Jeff Rowberg <jeff@rowberg.net>
//

/* ============================================
I2Cdev device library code is placed under the MIT license

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef I2CDEV_H
#define I2CDEV_H

#include "bcm2835.h"
#include <math.h>
#include <stdlib.h>
#include <string>

#define SET_I2C_PINS false
/* used to boolean for setting RPi I2C pins P1-03 (SDA) and P1-05 (SCL) to alternate function ALT0, which enables those pins for I2C interface.
   setI2Cpin should be false, if the I2C are already configured in alt mode ... */

#define I2C_BAUDRATE 400000

class I2Cdev
{
      public:
        I2Cdev();

        static void Initialize();
        static void Enable(bool isEnabled);

        static bool ReadBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data);
        static bool ReadBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);
        static bool ReadByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data);
        static bool ReadWord(uint8_t devAddr, uint8_t regAddr, uint16_t *data);
        static bool ReadBytes(uint8_t devAddr, uint8_t length, uint8_t *data);
        static bool ReadBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
        static bool ReadWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data);

        static bool WriteBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data);
        static bool WriteBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
        static bool WriteByte(uint8_t devAddr, uint8_t data);
        static bool WriteByte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
        static bool WriteWord(uint8_t devAddr, uint8_t regAddr, uint16_t data);
        static bool WriteBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
        static bool WriteWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data);
};

#endif // I2CDEV_H
