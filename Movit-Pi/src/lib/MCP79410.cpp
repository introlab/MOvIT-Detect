#include "MCP79410.h"


MCP79410::MCP79410() 
{
    devAddr = ADDR_MCP79410;
}

void MCP79410::setDateTime() 
{
	/*
    I2Cdev::writeByte(devAddr,ADDR_SEC,0);       	//STOP  MCP79410
	I2Cdev::writeByte(devAddr,ADDR_MIN,0x10);    		//MINUTE=10
	I2Cdev::writeByte(devAddr,ADDR_HOUR,0x17);    	//HOUR=15
	I2Cdev::writeByte(devAddr,ADDR_DAY,0x09);    	//DAY=1(MONDAY) AND VBAT=1
	I2Cdev::writeByte(devAddr,ADDR_DATE,0x10);    	//DATE=29
	I2Cdev::writeByte(devAddr,ADDR_MNTH,0x07);    		//MONTH=6
	I2Cdev::writeByte(devAddr,ADDR_YEAR,0x16);    	//YEAR=17
	I2Cdev::writeByte(devAddr,ADDR_SEC,0x80);    	//START  MCP79410, SECOND=00;
	*/
	I2Cdev::writeByte(devAddr,ADDR_SEC,0);       	//STOP  MCP79410
	I2Cdev::writeByte(devAddr,ADDR_MIN,0x20);    		//MINUTE=20
	I2Cdev::writeByte(devAddr,ADDR_HOUR,0x10);    	//HOUR=10 (0b0001 0000)
	I2Cdev::writeByte(devAddr,ADDR_DAY,0x0B);    	//DAY=3(WEDNESDAY) AND VBAT=1 (0b0000 1011)
	I2Cdev::writeByte(devAddr,ADDR_DATE,0x27);    	//DATE=27 (0b0010 0111)
	I2Cdev::writeByte(devAddr,ADDR_MNTH,0x09);    		//MONTH=9 (0b0000 1001)
	I2Cdev::writeByte(devAddr,ADDR_YEAR,0x17);    	//YEAR=17 (0b0001 0111)
	I2Cdev::writeByte(devAddr,ADDR_SEC,0x80);    	//START  MCP79410, SECOND=00;
}

void MCP79410::setDateTime(unsigned char date[]) 
{
    I2Cdev::writeByte(devAddr,ADDR_SEC,0);       	//STOP  MCP79410
	I2Cdev::writeByte(devAddr,ADDR_MIN,date[0]);    	//MINUTE=20
	I2Cdev::writeByte(devAddr,ADDR_HOUR,date[1]);    	//HOUR=15
	I2Cdev::writeByte(devAddr,ADDR_DAY,0x09);    	//DAY=1(MONDAY) AND VBAT=1
	I2Cdev::writeByte(devAddr,ADDR_DATE,date[2]);    	//DATE=29
	I2Cdev::writeByte(devAddr,ADDR_MNTH,date[3]);    	//MONTH=6
	I2Cdev::writeByte(devAddr,ADDR_YEAR,date[4]);    	//YEAR=17
	I2Cdev::writeByte(devAddr,ADDR_SEC,0x80);    	//START  MCP79410, SECOND=00;
}

void MCP79410::getDateTime(unsigned char dt[]) 
{
	I2Cdev::readByte(devAddr,ADDR_SEC,buffer);
	dt[0] = *buffer & 0xff>>(8-validBits[0]);
	I2Cdev::readByte(devAddr,ADDR_MIN,buffer);
	dt[1] = *buffer & 0xff>>(8-validBits[1]);
	I2Cdev::readByte(devAddr,ADDR_HOUR,buffer);
	dt[2] = *buffer & 0xff>>(8-validBits[2]);
	I2Cdev::readByte(devAddr,ADDR_DATE,buffer);
	dt[4] = *buffer & 0xff>>(8-validBits[4]);
	I2Cdev::readByte(devAddr,ADDR_MNTH,buffer);
	dt[5] = *buffer & 0xff>>(8-validBits[5]);
	I2Cdev::readByte(devAddr,ADDR_YEAR,buffer);
	dt[6] = *buffer & 0xff>>(8-validBits[6]);
}



















