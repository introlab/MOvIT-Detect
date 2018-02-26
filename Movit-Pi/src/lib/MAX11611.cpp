/* ============================================
PROGRAMME PAR: FRANCIS BOUCHER
POUR: MOVITPLUS
DATE: SEPTEMBRE 2017 � DECEMBRE 2017

DATASHEET: https://datasheets.maximintegrated.com/en/ds/MAX11606-MAX11611.pdf
===============================================
*/

#include "MAX11611.h"
#include "I2Cdev.h"

//uint8_t *dataToSend;

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

void MAX11611::initialize()
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

    dataToSend = 0x8A; //0b10001010
    //writeBytes(1, &dataToSend);
    // I2CDev::writeByte(devAddr, XXX, &dataToSend);

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

    dataToSend = 0x12; //0b00010001
    //writeBytes(1, &dataToSend);
    // I2CDev::writeByte(devAddr, XXX, &dataToSend);
}

//Voir librarie I2C + DataByte(Read Cycle) (DataSheet p.15) + Scan Mode (DataSheet p.17) + ***Fig.11 p.16***
//readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout)
// int8_t MAX11611::readBytes(uint8_t length, uint8_t *data)
// {
//     uint8_t devAddr = MAX11611_DEFAULT_ADDRESS;
//     int8_t count = 0;
//     uint32_t t1 = millis();
//     //I2Cdev::readBytes(MAX11611_DEFAULT_ADDRESS, 0x3B, 6, buffer);


//     // Arduino v1.0.1+, Wire library
//     // Adds official support for repeated start condition, yay!

//     // I2C/TWI subsystem uses internal buffer that breaks with large data requests
//     // so if user requests more than BUFFER_LENGTH bytes, we have to do it in
//     // smaller chunks instead of all at once
//     for (uint8_t k = 0; k < length; k += min(length, BUFFER_LENGTH))
//     {
//         //Wire.beginTransmission(devAddr);
//         //Wire.write(regAddr);
//         //Wire.endTransmission();
//         Wire.beginTransmission(devAddr);
//         Wire.requestFrom(devAddr, (uint8_t)min(length - k, BUFFER_LENGTH));

//         for (; Wire.available() && (timeout == 0 || millis() - t1 < timeout); count++)
//         {
//             data[count] = Wire.read();
// #ifdef I2CDEV_SERIAL_DEBUG
//             Serial.print(data[count], HEX);
//             if (count + 1 < length)
//                 Serial.print(" ");
// #endif
//         }
//     }

//     // check for timeout
//     if (timeout > 0 && millis() - t1 >= timeout && count < length)
//         count = -1; // timeout

// #ifdef I2CDEV_SERIAL_DEBUG
//     printf(". Done (");
//     printf(count, DEC);
//     printf(" read).\n");
// #endif

//     return count;
// }

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

    //readBytes(2 * nbOfAnalogDevices, rawData); //2 bytes par capteur (car valeur sur 10 bits (fig.11 datasheet p.16))
    //I2Cdev::readBytes(MAX11611_DEFAULT_ADDRESS, XXX, 6, buffer);

    for (int i = 0; i < (2 * nbOfAnalogDevices); i++)
    {
        //Pair = Prend seulement les 2 LSB (MSB du resultat), impair tout le byte est les LSB du resultat
        if (i % 2 == 0)
        {
            //Serial.print("\nValeur pair = ");Serial.print(rawData[i]);
            rawData[i] &= 0b00000011;            //Garde seulement les 2 LSB
            realData[(i) / 2] = rawData[i] << 8; //Deplace au 8 MSB du resultat
        }
        else
        {
            //Serial.print("\nValeur impair = ");Serial.print(rawData[i]);
            realData[(i - 1) / 2] += rawData[i];
        }
    }
    /*
	Serial.print("\n----RealData ----  "); Serial.print("\n");
	for(int i = 0; i<nbOfAnalogDevices;i++){
		Serial.print("i = "); Serial.print(i); Serial.print("\tdata = ");Serial.print(realData[i]); Serial.print("\n");
	}
	*/
}

/*
Test du bitshifting:

   uint8_t byte1;
   uint8_t byte2;
   uint16_t res;

   byte1 = 1;
   byte2 = 2;

   res = (byte1<<8) + byte2;
  Serial.print("\nRes bitshifting 8bits = "); Serial.print(res); Serial.print("\n");
*/
//bool MAX11611::writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* data) {
// bool MAX11611::writeBytes(uint8_t length, uint8_t *data)
// {
//     uint8_t status = 0;
// #if ((I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE && ARDUINO < 100) || I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_NBWIRE)
//     Wire.beginTransmission(devAddr);
//     //Wire.send((uint8_t) regAddr); // send address
// #elif (I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE && ARDUINO >= 100)
//     Wire.beginTransmission(devAddr);
//     //Wire.write((uint8_t) regAddr); // send address
// #elif (I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE)
//     Fastwire::beginTransmission(devAddr);
//     //Fastwire::write(regAddr);
// #endif
//     for (uint8_t i = 0; i < length; i++)
//     {
// #ifdef I2CDEV_SERIAL_DEBUG
//         Serial.print(data[i], HEX);
//         if (i + 1 < length)
//             Serial.print(" ");
// #endif
// #if ((I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE && ARDUINO < 100) || I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_NBWIRE)
//         Wire.send((uint8_t)data[i]);
// #elif (I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE && ARDUINO >= 100)
//         Wire.write((uint8_t)data[i]);
// #elif (I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE)
//         Fastwire::write((uint8_t)data[i]);
// #endif
//     }
// #if ((I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE && ARDUINO < 100) || I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_NBWIRE)
//     Wire.endTransmission();
// #elif (I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE && ARDUINO >= 100)
//     status = Wire.endTransmission();
// #elif (I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE)
//     Fastwire::stop();
//     //status = Fastwire::endTransmission();
// #endif
// #ifdef I2CDEV_SERIAL_DEBUG
//     Serial.println(". Done.");
// #endif
//     return status == 0;
// }
