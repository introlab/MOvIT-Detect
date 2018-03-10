#include "I2Cdev.h"   //I2C librairy
#include "MPU6050.h"  //Implementation of Jeff Rowberg's driver
#include "MCP79410.h" //Custom driver that uses I2Cdev.h to get RTC data
#include "MAX11611.h" //10-Bit ADC librairy

//Include : Modules
#include "init.h"         //variables and modules initialisation
#include "notif_module.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation

#include "forcePlate.h"
#include "forceSensor.h"
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation

#include <string>
#include <stdio.h>
#include <unistd.h>
#include "mosquitto_broker/mosquitto_broker.h"

using std::string;

forceSensor sensorMatrix;


#define SLEEP_TIME 2000000

void sendDataToWebServer(MosquittoBroker *mosquittoBroker)
{
    mosquittoBroker->sendBackRestAngle(10);
    mosquittoBroker->sendDistanceTraveled(1000);
}

MPU6050 imuMobile(0x68); //Initialisation of the mobile MPU6050
MPU6050 imuFixe(0x69);   //Initialisation of the fixed MPU6050
MCP79410 mcp79410;       //Initialisation of the MCP79410
MAX11611 max11611;       //Initialisation of the 10-bit ADC

unsigned char dateTime[7] = {0, 0, 0, 0, 0, 0, 0}; //{s, m, h, w, d, date, month, year}

uint16_t max11611Data[9]; //Data table of size=total sensors
long detectedPresence = 0;
long detectionThreshold = 0;
bool left_shearing = false;
bool right_shearing = false;
bool front_shearing = false;
bool rear_shearing = false;

//Movement detection variables
const float isMovingTrigger = 1.05;
float isMovingValue;

//Event variables
double tempArray[3], dump[3], tempAcc[3] = {0, 0, 0};
bool sleeping = false;
int alertNotification = 0;
bool testSequence = true;

//Debug variable
bool affichageSerial = true;

bool bIsBtnPushed = false;

int main()
{
    I2Cdev::initialize();

    // timer.setInterval(1000, callback);
    MosquittoBroker *mosquittoBroker = new MosquittoBroker("actionlistener");

    Alarm alarm(700, 0.1);
    init_accel();
    init_ADC();
    mcp79410.setDateTime();
    printf("Setup Done\n");

    bool done = false;
    while (!done)
    {
        done = program_loop(alarm);

        sendDataToWebServer(mosquittoBroker);
        usleep(SLEEP_TIME);
    }

    delete mosquittoBroker;
    return 0;
}

//---------------------------------------------------------------------------------------
// OPERATION CALLBACK DU TIMER A CHAQUE SECONDE
// on obtient les datas : date and time, capteur angle, capteur force
// on verifie si les led doivent blink, puis on les fait blink pour la duree demandee
// flagrunning : utilite a determiner
//---------------------------------------------------------------------------------------
 //void callback()
 //{
//     getData();     //Lit l'état des capteurs
//     led_control(); //Gère la LED du bouton, Etats: On,Off ou Blink

//     if (flagRunning == 0)
//     {
//         digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
//         flagRunning = 1;
//     }
//     else
//     {
//         digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
//         flagRunning = 0;
//     }
// }
