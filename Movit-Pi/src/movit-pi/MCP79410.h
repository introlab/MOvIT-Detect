#ifndef MCP79410_H
#define MCP79410_H

#include "I2Cdev.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                    GLOBAL CONSTANTS MCP79410 - ADDRESSES
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    #define ADDR_MCP79410 		0x6f	   //  DEVICE ADDR
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    #define  ADDR_EEPROM_WRITE 0xae       //  DEVICE ADDR for EEPROM (writes)
    #define  ADDR_EEPROM_READ  0xaf       //  DEVICE ADDR for EEPROM (reads)
    #define  ADDR_RTCC_WRITE   0xde       //  DEVICE ADDR for RTCC MCHP  (writes)
    #define  ADDR_RTCC_READ    0xdf       //  DEVICE ADDR for RTCC MCHP  (reads)
//................................................................................
    #define  SRAM_PTR          0x20       //  pointer of the SRAM area (RTCC)
    #define  ADDR_EEPROM_SR    0xff       //  STATUS REGISTER in the  EEPROM
//................................................................................
    #define  ADDR_SEC          0x00       //  address of SECONDS      register
    #define  ADDR_MIN          0x01       //  address of MINUTES      register
    #define  ADDR_HOUR         0x02       //  address of HOURS        register
    #define  ADDR_DAY          0x03       //  address of DAY OF WK    register
    #define  ADDR_STAT         0x03       //  address of STATUS       register
    #define  ADDR_DATE         0x04       //  address of DATE         register
    #define  ADDR_MNTH         0x05       //  address of MONTH        register
    #define  ADDR_YEAR         0x06       //  address of YEAR         register
    #define  ADDR_CTRL         0x07       //  address of CONTROL      register
    #define  ADDR_CAL          0x08       //  address of CALIB        register
    #define  ADDR_ULID         0x09       //  address of UNLOCK ID    register
//................................................................................
    #define  ADDR_ALM0SEC      0x0a       //  address of ALARMO SEC   register
    #define  ADDR_ALM0MIN      0x0b       //  address of ALARMO MIN   register
    #define  ADDR_ALM0HR       0x0c       //  address of ALARMO HOUR  register
    #define  ADDR_ALM0CTL      0x0d       //  address of ALARM0 CONTR register
    #define  ADDR_ALM0DAT      0x0e       //  address of ALARMO DATE  register
    #define  ADDR_ALM0MTH      0x0f       //  address of ALARMO MONTH register
//................................................................................
    #define  ADDR_ALM1SEC      0x11       //  address of ALARM1 SEC   register
    #define  ADDR_ALM1MIN      0x12       //  address of ALARM1 MIN   register
    #define  ADDR_ALM1HR       0x13       //  address of ALARM1 HOUR  register
    #define  ADDR_ALM1CTL      0x14       //  address of ALARM1 CONTR register
    #define  ADDR_ALM1DAT      0x15       //  address of ALARM1 DATE  register
    #define  ADDR_ALM1MTH      0x16       //  address of ALARM1 MONTH register
//................................................................................
    #define  ADDR_SAVtoBAT_MIN 0x18       //  address of T_SAVER MIN(VDD->BAT)
    #define  ADDR_SAVtoBAT_HR  0x19       //  address of T_SAVER HR (VDD->BAT)
    #define  ADDR_SAVtoBAT_DAT 0x1a       //  address of T_SAVER DAT(VDD->BAT)
    #define  ADDR_SAVtoBAT_MTH 0x1b       //  address of T_SAVER MTH(VDD->BAT)
//.................................................................................
    #define  ADDR_SAVtoVDD_MIN 0x1c       //  address of T_SAVER MIN(BAT->VDD)
    #define  ADDR_SAVtoVDD_HR  0x1d       //  address of T_SAVER HR (BAT->VDD)
    #define  ADDR_SAVtoVDD_DAT 0x1e       //  address of T_SAVER DAT(BAT->VDD)
    #define  ADDR_SAVtoVDD_MTH 0x1f       //  address of T_SAVER MTH(BAT->VDD)

    #define DATE_TIME_SIZE 7

class MCP79410
{
  public:
    MCP79410();
    void SetDefaultDateTime();
    void SetDateTime(uint8_t dt[]);
    void GetDateTime(uint8_t dt[]);

  private:
    uint8_t _devAddr;
    bool IsALeapYear(uint16_t year);
    void SetLeapYearBit(uint8_t dt[]);
    int _validBits[DATE_TIME_SIZE] = {7, 7, 6, 8, 6, 5, 8};
};

#endif // MCP79410_H
