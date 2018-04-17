//---------------------------------------------------------------------------------------
// HEADER DE FICHIER
// Description
//---------------------------------------------------------------------------------------

#include "HumiditySensor.h"
#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define MAXTIMINGS 85
#define PIN RPI_GPIO_P1_07
int dht11Data[5] = {0, 0, 0, 0, 0};

HumiditySensor::HumiditySensor() {}
HumiditySensor::~HumiditySensor() {}

void HumiditySensor::DetectHumidity()
{
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    float f;

    dht11Data[0] = dht11Data[1] = dht11Data[2] = dht11Data[3] = dht11Data[4] = 0;

    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);
    //pinMode( DHTPIN, OUTPUT );
    bcm2835_gpio_write(PIN, LOW);
    //digitalWrite( DHTPIN, LOW );
    bcm2835_delay(18);
    //delay( 18 );
    bcm2835_gpio_write(PIN, HIGH);
    //digitalWrite( DHTPIN, HIGH );
    bcm2835_delayMicroseconds(40);
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
    //pinMode( DHTPIN, INPUT );

    for (i = 0; i < MAXTIMINGS; i++)
    {
        counter = 0;
        bcm2835_gpio_set_pud(PIN, BCM2835_GPIO_PUD_UP);
        while (bcm2835_gpio_lev(PIN) == laststate)
        {
            counter++;
            bcm2835_delayMicroseconds(1);
            if (counter == 255)
            {
                break;
            }
        }
        laststate = bcm2835_gpio_lev(PIN);

        if (counter == 255)
        {
            break;
        }

        if ((i >= 4) && (i % 2 == 0))
        {
            dht11Data[j / 8] <<= 1;
            if (counter > 16)
            {
                dht11Data[j / 8] |= 1;
            }

            j++;
        }
    }

    if ((j >= 40) &&
        (dht11Data[4] == ((dht11Data[0] + dht11Data[1] + dht11Data[2] + dht11Data[3]) & 0xFF)))
    {
        f = dht11Data[2] * 9. / 5. + 32;
        printf("Humidity = %d.%d %% Temperature = %d.%d C (%.1f F)\n",
               dht11Data[0], dht11Data[1], dht11Data[2], dht11Data[3], f);
    }
    else
    {
        printf("Data not good, skip\n");
    }
}
