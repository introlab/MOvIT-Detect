/* ============================================
PROGRAMME PAR: FRANCIS BOUCHER
POUR: MOVITPLUS
DATE: SEPTEMBRE 2017 � DECEMBRE 2017

DATASHEET: https://datasheets.maximintegrated.com/en/ds/MAX11606-MAX11611.pdf
===============================================
*/

#include "MAX11611.h"
#include "I2Cdev.h"
#include <stdio.h>

// Default constructor, uses default I2C address.
MAX11611::MAX11611()
{
    devAddr = MAX11611_DEFAULT_ADDRESS;
}

// Specific address constructor.
MAX11611::MAX11611(uint8_t address)
{
    devAddr = address;
}

bool MAX11611::initialize()
{
    //Setup Byte Format (Datasheet p.13)
    /*
	bit7 = 1; //Setup
	bit6 = 0; //SEL2 (Alim options)
	bit5 = 0; //SEL1
	bit4 = 0; //SEL0
	bit3 = 0; //0=Internal 1=External clock
	bit2 = 0; //Unipolar
	bit1 = 1; //No action
	bit0 = 0; //Don't-care bit
	*/
    uint8_t dataToSend;

    dataToSend = 0x8A; //10001010
    if (!I2Cdev::writeByte(devAddr, dataToSend))
    {
        return false;
    }

    //Configuration Byte
    /*
	bit7 = 0; //Configuration
	bit6 = 0; //SCAN1
	bit5 = 0; //SCAN0
	bit4 = 1; //CS3 //0011 = AIN3 (Va donc scanner de AIN0 � AIN3)
	bit3 = 0; //CS2
	bit2 = 0; //CS1
	bit1 = 0; //CS0
	bit0 = 1; //Single-ended
	*/
    dataToSend = 0x11; //0b00010001
    if (!I2Cdev::writeByte(devAddr, dataToSend))
    {
        return false;
    }

    return true;
}

//Fonctions traitant les donnees brutes (2*8bits par capteur) sur une seule variable 16 bits
void MAX11611::getData(uint8_t nbOfAnalogDevices, uint16_t *realData)
{
    uint8_t rawDataArray[2 * nbOfAnalogDevices];
    uint8_t *rawData;
    //uint16_t valeur;
    uint8_t rawArraySize;

    rawData = rawDataArray;
    rawArraySize = sizeof(rawDataArray);
    //Tres lent dans une loop, fait a fin de test
    for (int i = 0; i < rawArraySize; i++)
    {
        rawData[i] = 0;
        if (i < (rawArraySize / 2))
        {
            realData[i] = 0;
        }
    }

    //Ancien call deprecated
    //readBytes(2 * nbOfAnalogDevices, rawData); //2 bytes par capteur (car valeur sur 10 bits (fig.11 datasheet p.16))
    //Nouveau call à implémenter
    //mise en commentaire des 4 printfs, decommenter pour debug
    I2Cdev::readBytes(MAX11611_DEFAULT_ADDRESS, 2 * nbOfAnalogDevices, rawData);

    for (int i = 0; i < (2 * nbOfAnalogDevices); i++)
    {
        //Pair = Prend seulement les 2 LSB (MSB du resultat), impair tout le byte est les LSB du resultat
        if (i % 2 == 0)
        {
            //printf("\nValeur pair = %i", rawData[i]);
            rawData[i] &= 0x03;                  // 0b00000011;            //Garde seulement les 2 LSB
            realData[(i) / 2] = rawData[i] << 8; //Deplace au 8 MSB du resultat
        }
        else
        {
            //printf("\nValeur impair = %i", rawData[i]);
            realData[(i - 1) / 2] += rawData[i];
        }
    }

    //printf("\n----RealData ---- \n");
    for (int i = 0; i < nbOfAnalogDevices; i++)
    {
        //printf("i = %i\tdata = %i\n", i, realData[i]);
    }
}
