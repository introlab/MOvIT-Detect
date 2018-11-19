#include "PMW3901.h"
#include "Utils.h"
#include "SysTime.h"

#include <chrono>
#include <thread>

#define PIN RPI_GPIO_P1_22
#define TIME_BETWEEN_COMMANDS 60
#define TIME_TO_END_TRANSACTION 200
#define TIME_TO_START_TRANSACTION 50
#define SHORT_MIN -32768
#define SHORT_MAX 32767

PMW3901::~PMW3901()
{
  bcm2835_close();
}

bool PMW3901::Initialize()
{
  //Set the CS pin
  bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);
  SetChipSelect(HIGH);

  // Power on reset
  RegisterWrite(0x3A, 0x5A);

  // Test the SPI communication, checking chipId and inverse chipId
  uint8_t chipId = RegisterRead(0x00);
  uint8_t dIpihc = RegisterRead(0x5F);
  if (chipId != 0x49 || dIpihc != 0xB6)
  {
    return false;
  }

  // Reading the motion registers one time
  RegisterRead(0x02);
  RegisterRead(0x03);
  RegisterRead(0x04);
  RegisterRead(0x05);
  RegisterRead(0x06);

  InitRegisters();

  return true;
}

void PMW3901::ReadMotionCount(int16_t *deltaX, int16_t *deltaY)
{
  RegisterRead(0x02);
  *deltaX = ((int16_t)(RegisterRead(0x04) & 0xff) << 8) | (RegisterRead(0x03) & 0xff);
  *deltaY = ((int16_t)(RegisterRead(0x06) & 0xff) << 8) | (RegisterRead(0x05) & 0xff);

  if (*deltaX == SHORT_MIN || *deltaX == SHORT_MAX || *deltaY == SHORT_MIN || *deltaY == SHORT_MAX)
  {
    printf("ERROR: Optical motion tracking chip max value reach\n");
  }
}

void PMW3901::RegisterWrite(uint8_t reg, uint8_t value)
{
  BeginTransaction();
  bcm2835_spi_transfer(reg | 0x80u);
  sleep_for_microseconds(TIME_BETWEEN_COMMANDS);
  bcm2835_spi_transfer(value);
  EndTransaction();
}

uint8_t PMW3901::RegisterRead(uint8_t reg)
{
  BeginTransaction();
  bcm2835_spi_transfer(reg & ~0x80u);
  sleep_for_microseconds(TIME_BETWEEN_COMMANDS);
  uint8_t value = bcm2835_spi_transfer(0x00);
  EndTransaction();
  return value;
}

// Performance optimisation registers
void PMW3901::InitRegisters()
{
  RegisterWrite(0x7F, 0x00);
  RegisterWrite(0x61, 0xAD);
  RegisterWrite(0x7F, 0x03);
  RegisterWrite(0x40, 0x00);
  RegisterWrite(0x7F, 0x05);
  RegisterWrite(0x41, 0xB3);
  RegisterWrite(0x43, 0xF1);
  RegisterWrite(0x45, 0x14);
  RegisterWrite(0x5B, 0x32);
  RegisterWrite(0x5F, 0x34);
  RegisterWrite(0x7B, 0x08);
  RegisterWrite(0x7F, 0x06);
  RegisterWrite(0x44, 0x1B);
  RegisterWrite(0x40, 0xBF);
  RegisterWrite(0x4E, 0x3F);

  RegisterWrite(0x7F, 0x08);
  RegisterWrite(0x65, 0x20);
  RegisterWrite(0x6A, 0x18);
  RegisterWrite(0x7F, 0x09);
  RegisterWrite(0x4F, 0xAF);
  RegisterWrite(0x5F, 0x40);
  RegisterWrite(0x48, 0x80);
  RegisterWrite(0x49, 0x80);
  RegisterWrite(0x57, 0x77);
  RegisterWrite(0x60, 0x78);
  RegisterWrite(0x61, 0x78);
  RegisterWrite(0x62, 0x08);
  RegisterWrite(0x63, 0x50);
  RegisterWrite(0x7F, 0x0A);
  RegisterWrite(0x45, 0x60);
  RegisterWrite(0x7F, 0x00);
  RegisterWrite(0x4D, 0x11);
  RegisterWrite(0x55, 0x80);
  RegisterWrite(0x74, 0x1F);
  RegisterWrite(0x75, 0x1F);
  RegisterWrite(0x4A, 0x78);
  RegisterWrite(0x4B, 0x78);
  RegisterWrite(0x44, 0x08);
  RegisterWrite(0x45, 0x50);
  RegisterWrite(0x64, 0xFF);
  RegisterWrite(0x65, 0x1F);
  RegisterWrite(0x7F, 0x14);
  RegisterWrite(0x65, 0x60);
  RegisterWrite(0x66, 0x08);
  RegisterWrite(0x63, 0x78);
  RegisterWrite(0x7F, 0x15);
  RegisterWrite(0x48, 0x58);
  RegisterWrite(0x7F, 0x07);
  RegisterWrite(0x41, 0x0D);
  RegisterWrite(0x43, 0x14);
  RegisterWrite(0x4B, 0x0E);
  RegisterWrite(0x45, 0x0F);
  RegisterWrite(0x44, 0x42);
  RegisterWrite(0x4C, 0x80);
  RegisterWrite(0x7F, 0x10);
  RegisterWrite(0x5B, 0x02);
  RegisterWrite(0x7F, 0x07);
  RegisterWrite(0x40, 0x41);
  RegisterWrite(0x70, 0x00);

  sleep_for_milliseconds(100);
  RegisterWrite(0x32, 0x44);
  RegisterWrite(0x7F, 0x07);
  RegisterWrite(0x40, 0x40);
  RegisterWrite(0x7F, 0x06);
  RegisterWrite(0x62, 0xf0);
  RegisterWrite(0x63, 0x00);
  RegisterWrite(0x7F, 0x0D);
  RegisterWrite(0x48, 0xC0);
  RegisterWrite(0x6F, 0xd5);
  RegisterWrite(0x7F, 0x00);
  RegisterWrite(0x5B, 0xa0);
  RegisterWrite(0x4E, 0xA8);
  RegisterWrite(0x5A, 0x50);
  RegisterWrite(0x40, 0x80);
}

void PMW3901::BeginTransaction()
{
  bcm2835_spi_begin();
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
  SetChipSelect(LOW);
  sleep_for_microseconds(TIME_TO_START_TRANSACTION);
}

void PMW3901::EndTransaction()
{
  sleep_for_microseconds(TIME_TO_END_TRANSACTION);
  SetChipSelect(HIGH);
  bcm2835_spi_end();
}

void PMW3901::SetChipSelect(uint8_t active)
{
  bcm2835_gpio_write(PIN, active);
}
