/* ============================================
PROGRAMME PAR: FRANCIS BOUCHER
POUR: MOVITPLUS
DATE: SEPTEMBRE 2017 DECEMBRE 2017

DATASHEET: https://datasheets.maximintegrated.com/en/ds/MAX11606-MAX11611.pdf
===============================================
*/

#include "MAX11611.h"
#include "I2Cdev.h"
#include <stdio.h>

// Default constructor, uses default I2C address.
MAX11611::MAX11611()
{
    _devAddr = MAX11611_DEFAULT_ADDRESS;
}

// Specific address constructor.
MAX11611::MAX11611(uint8_t address)
{
    _devAddr = address;
}

bool MAX11611::Initialize()
{
    //Setup Byte Format (Datasheet p.13)
    /*
	bit7 = 1; //Setup
	bit6 = 0; //SEL2 (Alim options)
	bit5 = 0; //SEL1
	bit4 = 0; //SEL0
        bit3 = 1; //0=Internal 1=External clock
	bit2 = 0; //Unipolar
	bit1 = 1; //No action
	bit0 = 0; //Don't-care bit
	*/
    uint8_t dataToSend;

    dataToSend = 0x8A; //10001010
    if (!I2Cdev::WriteByte(_devAddr, dataToSend))
    {
        printf("MAX11611 initialize error\n");
        return false;
    }

    //Configuration Byte
    /*
	bit7 = 0; //Configuration
	bit6 = 0; //SCAN1
	bit5 = 0; //SCAN0
        bit4 = 1; //CS3 //0011 = AIN3 (Va donc scanner de AIN0 AIN8)
	bit3 = 0; //CS2
	bit2 = 0; //CS1
	bit1 = 0; //CS0
	bit0 = 1; //Single-ended
	*/
    dataToSend = 0x11; //0b00010001
    if (!I2Cdev::WriteByte(_devAddr, dataToSend))
    {
        printf("MAX11611 initialize error\n");
        return false;
    }

    return true;
}

//Fonctions traitant les donnees brutes (2*8bits par capteur) sur une seule variable 16 bits (les données sont sur 10 bits)
bool MAX11611::GetData(uint8_t nbOfAnalogDevices, uint16_t *realData)
{
    uint8_t rawDataArray[2 * nbOfAnalogDevices];

    //Initialisation des arrays à 0
    for (int i = 0; i < 2*nbOfAnalogDevices; i++)
    {
        rawDataArray[i] = 0;
    }
    for (int i = 0; i < nbOfAnalogDevices; i++)
    {
        realData[i] = 0;
    }

    if (I2Cdev::ReadBytes(MAX11611_DEFAULT_ADDRESS, 2 * nbOfAnalogDevices, rawDataArray)) //Lecture sur le bus I2C
    {//Format des données en sortie : [ 1 1 1 1 1 1 x x ][ x x x x x x x x  ]   où les x représentes les bits de la donnée
        //                               [             1 2 ][ 3 4 5 6 7 8 9 10 ]
        //                               [ rawData[pair]   ][ rawData[impaire] ]
        //Formattage :
        for(int i = 0; i < 2 * nbOfAnalogDevices; i++)
        {
            //rawDataArray pair = Prend seulement les 2 LSB -> représente les MSB de la donnée
            if (i % 2 == 0)
            {
                //printf("\nValeur pair = %i", rawDataArray[i]);
                rawDataArray[i] &= 0x03;                  // = 0b00000011 donc garde seulement les 2 LSB
                realData[(i) / 2] = rawDataArray[i] << 8; //Déplace au 8 MSB du resultat (donc [ _ _ _ _ _ _ 1 2 x x x x x x x x ])
            }
            //rawDataArray impair = Prend tout le byte et ajoute  les LSB du resultat
            else
            {
                //printf("\nValeur impair = %i", rawDataArray[i]);
                realData[(i - 1) / 2] += rawDataArray[i]; //donc [ _ _ _ _ _ _ x x 3 4 5 6 7 8 9 10 ]
            }
        }

        /*
        printf("\n----RealData ---- \n");
        for (int i = 0; i < nbOfAnalogDevices; i++)
        {
            printf("i = %i\tdata = %i\n", i, realData[i]); //Valeurs finales
        }
        */
        return true;
    }

    return false;
}
