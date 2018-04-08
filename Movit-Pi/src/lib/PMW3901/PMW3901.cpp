#include "PMW3901.h"

#include <chrono>
#include <thread>

PMW3901::PMW3901(uint8_t cspin) : _cs(cspin)
{
}

void PMW3901::setupSPI()
{
  if (!bcm2835_init())
  {
    printf("bcm2835_init failed.\n");
  }

  beginTransaction();

  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

  endTransaction();
}

bool PMW3901::begin()
{
  // Setup SPI
  setupSPI();

  // Power on reset
  registerWrite(0x3A, 0x5A);

  sleepForMilliseconds(5);

  // Test the SPI communication, checking chipId and inverse chipId
  uint8_t chipId = registerRead(0x00);
  uint8_t dIpihc = registerRead(0x5F);

  if (chipId != 0x49 && dIpihc != 0xB8)
  {
    return false;
  }

  // Reading the motion registers one time
  registerRead(0x02);
  registerRead(0x03);
  registerRead(0x04);
  registerRead(0x05);
  registerRead(0x06);

  sleepForMilliseconds(5);

  initRegisters();

  return true;
}

// Functional access

void PMW3901::readMotionCount(int16_t *deltaX, int16_t *deltaY)
{
  registerRead(0x02);
  *deltaX = ((int16_t)registerRead(0x04) << 8) | registerRead(0x03);
  *deltaY = ((int16_t)registerRead(0x06) << 8) | registerRead(0x05);
}

// Low level register access
void PMW3901::registerWrite(uint8_t reg, uint8_t value)
{
  reg |= 0x80u;

  beginTransaction();

  bcm2835_spi_transfer(reg);
  bcm2835_spi_transfer(value);

  endTransaction();
}

uint8_t PMW3901::registerRead(uint8_t reg)
{
  reg &= ~0x80u;

  beginTransaction();

  bcm2835_spi_transfer(reg);
  sleepForMicroseconds(50);
  uint8_t value = bcm2835_spi_transfer(0);
  sleepForMicroseconds(200);

  endTransaction();
  return value;
}

// Performance optimisation registers
void PMW3901::initRegisters()
{
  registerWrite(0x7F, 0x00);
  registerWrite(0x61, 0xAD);
  registerWrite(0x7F, 0x03);
  registerWrite(0x40, 0x00);
  registerWrite(0x7F, 0x05);
  registerWrite(0x41, 0xB3);
  registerWrite(0x43, 0xF1);
  registerWrite(0x45, 0x14);
  registerWrite(0x5B, 0x32);
  registerWrite(0x5F, 0x34);
  registerWrite(0x7B, 0x08);
  registerWrite(0x7F, 0x06);
  registerWrite(0x44, 0x1B);
  registerWrite(0x40, 0xBF);
  registerWrite(0x4E, 0x3F);
  registerWrite(0x7F, 0x08);
  registerWrite(0x65, 0x20);
  registerWrite(0x6A, 0x18);
  registerWrite(0x7F, 0x09);
  registerWrite(0x4F, 0xAF);
  registerWrite(0x5F, 0x40);
  registerWrite(0x48, 0x80);
  registerWrite(0x49, 0x80);
  registerWrite(0x57, 0x77);
  registerWrite(0x60, 0x78);
  registerWrite(0x61, 0x78);
  registerWrite(0x62, 0x08);
  registerWrite(0x63, 0x50);
  registerWrite(0x7F, 0x0A);
  registerWrite(0x45, 0x60);
  registerWrite(0x7F, 0x00);
  registerWrite(0x4D, 0x11);
  registerWrite(0x55, 0x80);
  registerWrite(0x74, 0x1F);
  registerWrite(0x75, 0x1F);
  registerWrite(0x4A, 0x78);
  registerWrite(0x4B, 0x78);
  registerWrite(0x44, 0x08);
  registerWrite(0x45, 0x50);
  registerWrite(0x64, 0xFF);
  registerWrite(0x65, 0x1F);
  registerWrite(0x7F, 0x14);
  registerWrite(0x65, 0x60);
  registerWrite(0x66, 0x08);
  registerWrite(0x63, 0x78);
  registerWrite(0x7F, 0x15);
  registerWrite(0x48, 0x58);
  registerWrite(0x7F, 0x07);
  registerWrite(0x41, 0x0D);
  registerWrite(0x43, 0x14);
  registerWrite(0x4B, 0x0E);
  registerWrite(0x45, 0x0F);
  registerWrite(0x44, 0x42);
  registerWrite(0x4C, 0x80);
  registerWrite(0x7F, 0x10);
  registerWrite(0x5B, 0x02);
  registerWrite(0x7F, 0x07);
  registerWrite(0x40, 0x41);
  registerWrite(0x70, 0x00);

  sleepForMilliseconds(100);
  registerWrite(0x32, 0x44);
  registerWrite(0x7F, 0x07);
  registerWrite(0x40, 0x40);
  registerWrite(0x7F, 0x06);
  registerWrite(0x62, 0xf0);
  registerWrite(0x63, 0x00);
  registerWrite(0x7F, 0x0D);
  registerWrite(0x48, 0xC0);
  registerWrite(0x6F, 0xd5);
  registerWrite(0x7F, 0x00);
  registerWrite(0x5B, 0xa0);
  registerWrite(0x4E, 0xA8);
  registerWrite(0x5A, 0x50);
  registerWrite(0x40, 0x80);
}

void PMW3901::sleepForMicroseconds(uint32_t microseconds)
{
  std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void PMW3901::sleepForMilliseconds(uint32_t milliseconds)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void PMW3901::beginTransaction()
{
  if (!bcm2835_spi_begin())
  {
    printf("bcm2835_spi_begin failed.\n");
  }
  sleepForMicroseconds(200);
}

void PMW3901::beginTransaction()
{
  if (!bcm2835_spi_begin())
  {
    printf("bcm2835_spi_begin failed.\n");
  }
}

void PMW3901::endTransaction()
{
  bcm2835_spi_end();
  sleepForMicroseconds(200);
}