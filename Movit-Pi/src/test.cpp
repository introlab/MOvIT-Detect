//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

//Include : Drivers
#include "MPU6050.h"     //Implementation of Jeff Rowberg's driver
#include "MAX11611.h"    //10-Bit ADC
#include "MCP79410.h"    //Custom driver that uses I2Cdev.h to get RTC data

//Include : Modules
#include "init.h"         //variables and modules initialisation
#include "notif_module.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "force_module.h" //variables and modules initialisation
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation

#include <stdio.h>

//External functions and variables
//Variables
// extern SimpleTimer timer;
extern int blinkDuree;
extern double blinkFreq;
extern int nCompteur;
extern bool blink_enabled;
extern bool redLedEnabled;
extern bool greenLedEnabled;
// extern bool buzzerEnabled;
// extern bool currentLedState;

// extern bool sleeping;
extern bool testSequence;
extern bool affichageSerial;

extern float isMovingValue;

extern unsigned char dateTime[7]; //{s, m, h, w, d, date, month, year}

extern MPU6050 imuMobile; //Initialisation of the mobile MPU6050
extern MPU6050 imuFixe;   //Initialisation of the fixed MPU6050
// extern MAX11611 max11611; //Initialisation of the 10-bit ADC
// extern MCP79410 mcp79410; //Initialisation of the MCP79410

// extern int aRange;                                          //maximal measurable g-force
// extern int16_t ax, ay, az;                                  //axis accelerations
// extern double aRawFixe[3], aRawMobile[3];                   //raw acceleration
extern double aRealFixe[3], aRealMobile[3];                 //computed acceleration
extern double pitchFixe, rollFixe, pitchMobile, rollMobile; //pitch and roll angle values
extern int angle;                                           //Final angle computed between sensors
// extern int obs;                                             //Debug variable for getAngle function

extern uint16_t *max11611Data;                     //ADC 10-bit data variable
const uint8_t capteurForceNb = 9;                  //Total number of sensors
extern uint16_t max11611DataArray[capteurForceNb]; //Data table of size=total sensors
extern long max11611Voltage[capteurForceNb];       //ADC 10-bit data variable
extern long max11611Resistance[capteurForceNb];    //ADC 10-bit data variable
extern long max11611Conductance[capteurForceNb];   //ADC 10-bit data variable
extern long max11611Force[capteurForceNb];         //ADC 10-bit data variable

//Functions
// extern bool isSomeoneThere();
// extern bool isMoving();
extern bool left_shearing;
extern bool right_shearing;
extern bool front_shearing;
extern bool rear_shearing;

void program_test()
{
    //char inSerialChar = (char)Serial.read();
    char inSerialChar = getchar();

    //---------------------------------------------------------------------------------------
    // TEST DU MODULE NOTIFICATION : OK
    // DEL Red/Green, MoteurDC, Bouton poussoir
    //---------------------------------------------------------------------------------------

    if (inSerialChar == 't')
    {
        //Ne rien laisse d'important ici

        getData();

        //Force plates variables
        ForcePlate ForcePlate1;
        ForcePlate ForcePlate2;
        ForcePlate ForcePlate3;
        ForcePlate ForcePlate4;

        //Creation of the 4 ForcePlates
        ForcePlate1.CreateForcePlate(ForcePlate1, 4, 5, 2, 1);
        ForcePlate1.CreateForcePlate(ForcePlate2, 3, 4, 1, 9);
        ForcePlate1.CreateForcePlate(ForcePlate3, 7, 8, 5, 4);
        ForcePlate1.CreateForcePlate(ForcePlate4, 6, 7, 4, 3);

        //Global analysis of the 4 plates (treated as one plate)
        ForcePlate GlobalForcePlate;
        GlobalForcePlate.AnalyzeForcePlates(GlobalForcePlate, ForcePlate1, ForcePlate2, ForcePlate3, ForcePlate4);

        printf("ForcePlate1 COP along X-Axis: ");
        printf("%i\n", ForcePlate1.COPx);
        printf("ForcePlate1 COP along Y-Axis: ");
        printf("%i\n", ForcePlate1.COPy);

        printf("ForcePlate2 COP along X-Axis: ");
        printf("%i\n", ForcePlate2.COPx);
        printf("ForcePlate2 COP along Y-Axis: ");
        printf("%i\n", ForcePlate2.COPy);

        printf("ForcePlate3 COP along X-Axis: ");
        printf("%i\n", ForcePlate3.COPx);
        printf("ForcePlate3 COP along Y-Axis: ");
        printf("%i\n", ForcePlate3.COPy);

        printf("ForcePlate4 COP along X-Axis: ");
        printf("%i\n", ForcePlate4.COPx);
        printf("ForcePlate4 COP along Y-Axis: ");
        printf("%i\n", ForcePlate4.COPy);

        printf("Global COP along X-Axis: ");
        printf("%i\n", GlobalForcePlate.COPx);
        printf("Global COP along Y-Axis: ");
        printf("%i\n", GlobalForcePlate.COPy);

        inSerialChar = 'x';
    }

    else if (inSerialChar == 'g')
    {
        LightOn(GREEN_LED);
        printf("Fonction allumer la DEL verte activée.\n");
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'r')
    {
        LightOn(RED_LED);
        printf("Fonction allumer la DEL rouge activée.\n");
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'm')
    {
        StartBuzzer();
        printf("Fonction mettre en marche le moteur DC activée.\n");
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'b')
    {
        uint8_t btn_state = isPushed();
        printf("Fonction verifier l'etat du boutton poussoir activée.\n");
        printf("Etat du bouton = ");
        printf("%i\n", btn_state);
        printf("\n");
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'a')
    {
        blink_enabled = true;

        greenLedEnabled = true;
        redLedEnabled = true;
        blinkDuree = 10000;
        blinkFreq = 0.1;
        while (!isPushed() || (nCompteur >= (blinkFreq * blinkDuree)))
        {
            StartBuzzer();
            led_control();
        }
        StopBuzzer();
        blink_enabled = false;
        greenLedEnabled = false;
        redLedEnabled = false;
        led_control();
        printf("Fonction verifier l'alarme du module notification activée.\n");
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'c')
    {

        shearing_detection1();
        printf("Fonction detection de cisaillement activée.\n");
        printf("Cisaillement (L-R): ");
        if (right_shearing)
        {
            printf("vers la droite\n");
        }
        else if (left_shearing)
        {
            printf("vers la gauche\n");
        }
        else
        {
            printf("aucun\n");
        }
        printf("Cisaillement (F-R): ");
        if (front_shearing)
        {
            printf("vers l'avant\n");
        }
        else if (rear_shearing)
        {
            printf("vers l'arriere\n");
        }
        else
        {
            printf("aucun\n");
        }
    }

    else if (inSerialChar == 'z')
    {
        StopBuzzer();
        LightOff(GREEN_LED);
        LightOff(RED_LED);
        printf("Fonction pour etteindre toutes les DELs et arrêter le moteur.\n");
        inSerialChar = 'x';
    }

    //---------------------------------------------------------------------------------------
    // OPTION DE DEBUG : OK
    // Sequence de test ON-OFF, Output print ON-OFF
    //---------------------------------------------------------------------------------------
    else if (inSerialChar == 'd')
    {
        printf("Début de séquence de test en cours.\n");
        testSequence = true;
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'f')
    {
        printf("Fin de séquence de test en cours.\n");
        testSequence = false;
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'p')
    {
        printf("Ne pas afficher les sorties.\n");
        affichageSerial = false;
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'P')
    {
        printf("Afficher les sorties.\n");
        affichageSerial = true;
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'c')
    {
        calibrationProcess(imuFixe, 0);
        delay(50);
        calibrationProcess(imuMobile, 0);
        printf("Calibration des capteurs effectuée.\n");
        inSerialChar = 'x';
    }
    //---------------------------------------------------------------------------------------
    // A VALIDER
    //---------------------------------------------------------------------------------------

    //
    //              else if (inSerialChar == 's')
    //              {
    //                printf("Cette fonction doit permettre de calculer la vitesse de fauteuil.\n");
    //                inSerialChar = 'x';
    //              }
}

//---------------------------------------------------------------------------------------
// FONCTION D'IMPRESSION SUR LA CONSOLE POUR DEBUG
// Date and time, capteurs de force, personne assise, fauteuil en mouvement, angle de la chaise
// A quoi servent les CASE tu penses ? - LP
//---------------------------------------------------------------------------------------

void printStuff()
{
    // Date and time
    printf("\nAffichage de la date + heure\n");
    printf("20");
    printf("%X\n", dateTime[6]);
    printf(".");
    printf("%X\n", dateTime[5]);
    printf(".");
    printf("%X\n", dateTime[4]);
    printf(" ");
    printf("%X\n", dateTime[2]);
    printf(":");
    printf("%X\n", dateTime[1]);
    printf(":");
    printf("%X\n", dateTime[0]);
    printf("\n");

    // Capteurs de force
    //Analog values
    printf("\nValeurs Capteurs de force : Analog reading- DEBUG\n");
    //for (uint16_t capteurNo = 0; capteurNo < capteurForceNb; capteurNo++)
    //{
    printf("Capt.No:");
    printf("2");
    printf(" - ");
    printf("%i\n", max11611Data[1]);
    printf("\t");
    printf("Capt.No:");
    printf("1");
    printf(" - ");
    printf("%i\n", max11611Data[0]);
    printf("\t");
    printf("Capt.No:");
    printf("9");
    printf(" - ");
    printf("%i\n", max11611Data[8]);
    printf("\n");
    printf("Capt.No:");
    printf("5");
    printf(" - ");
    printf("%i\n", max11611Data[4]);
    printf("\t");
    printf("Capt.No:");
    printf("4");
    printf(" - ");
    printf("%i\n", max11611Data[3]);
    printf("\t");
    printf("Capt.No:");
    printf("3");
    printf(" - ");
    printf("%i\n", max11611Data[2]);
    printf("\n");
    printf("Capt.No:");
    printf("8");
    printf(" - ");
    printf("%i\n", max11611Data[7]);
    printf("\t");
    printf("Capt.No:");
    printf("7");
    printf(" - ");
    printf("%i\n", max11611Data[6]);
    printf("\t");
    printf("Capt.No:");
    printf("6");
    printf(" - ");
    printf("%i\n", max11611Data[5]);
    printf("\n");
    //}
    //Voltage values
    printf("\nValeurs Capteurs de force : Voltage reading (mV)- DEBUG\n");
    //for (uint16_t capteurNo = 0; capteurNo < capteurForceNb; capteurNo++)
    //{
    printf("Capt.No:");
    printf("2");
    printf(" - ");
    printf("%li\n", max11611Voltage[1]);
    printf("\t\t");
    printf("Capt.No:");
    printf("1");
    printf(" - ");
    printf("%li\n", max11611Voltage[0]);
    printf("\t\t");
    printf("Capt.No:");
    printf("9");
    printf(" - ");
    printf("%li\n", max11611Voltage[8]);
    printf("\n");
    printf("Capt.No:");
    printf("5");
    printf(" - ");
    printf("%li\n", max11611Voltage[4]);
    printf("\t\t");
    printf("Capt.No:");
    printf("4");
    printf(" - ");
    printf("%li\n", max11611Voltage[3]);
    printf("\t\t");
    printf("Capt.No:");
    printf("3");
    printf(" - ");
    printf("%li\n", max11611Voltage[2]);
    printf("\n");
    printf("Capt.No:");
    printf("8");
    printf(" - ");
    printf("%li\n", max11611Voltage[7]);
    printf("\t\t");
    printf("Capt.No:");
    printf("7");
    printf(" - ");
    printf("%li\n", max11611Voltage[6]);
    printf("\t\t");
    printf("Capt.No:");
    printf("6");
    printf(" - ");
    printf("%li\n", max11611Voltage[5]);
    printf("\n");
    //}

    //Force values
    printf("\nValeurs Capteurs de force : Force reading (N)- DEBUG\n");
    //for (uint16_t capteurNo = 0; capteurNo < capteurForceNb; capteurNo++)
    //{
    printf("Capt.No:");
    printf("2");
    printf(" - ");
    printf("%li\n", max11611Force[1]);
    printf("\t");
    printf("Capt.No:");
    printf("1");
    printf(" - ");
    printf("%li\n", max11611Force[0]);
    printf("\t");
    printf("Capt.No:");
    printf("9");
    printf(" - ");
    printf("%li\n", max11611Force[8]);
    printf("\n");
    printf("Capt.No:");
    printf("5");
    printf(" - ");
    printf("%li\n", max11611Force[4]);
    printf("\t");
    printf("Capt.No:");
    printf("4");
    printf(" - ");
    printf("%li\n", max11611Force[3]);
    printf("\t");
    printf("Capt.No:");
    printf("3");
    printf(" - ");
    printf("%li\n", max11611Force[2]);
    printf("\n");
    printf("Capt.No:");
    printf("8");
    printf(" - ");
    printf("%li\n", max11611Force[7]);
    printf("\t");
    printf("Capt.No:");
    printf("7");
    printf(" - ");
    printf("%li\n", max11611Force[6]);
    printf("\t");
    printf("Capt.No:");
    printf("6");
    printf(" - ");
    printf("%li\n", max11611Force[5]);
    printf("\n");
    //}

    // Verifier si une personne est assise
    printf("\nDetection d'une personne sur la chaise\n");
    if (isSomeoneThere())
    {
        printf("Someone is on the chair\n");
    }
    else
    {
        printf("No one is on the chair\n");
    }

    //Verifier le positionnement de la personne
    printf("Cisaillement (L-R): ");
    if (right_shearing)
    {
        printf("vers la droite\n");
    }
    else if (left_shearing)
    {
        printf("vers la gauche\n");
    }
    else
    {
        printf("aucun\n");
    }
    printf("Cisaillement (F-R): ");
    if (front_shearing)
    {
        printf("vers l'avant\n");
    }
    else if (rear_shearing)
    {
        printf("vers l'arriere\n");
    }
    else
    {
        printf("aucun\n");
    }

    // Verifier si le fauteuil est en mouvement
    printf("\nDetection si la chaise est en mouvement");
    if (isMoving())
    {
        printf("The chair is moving\n");
        printf("Value m/s^2:");
        printf("\t");
        printf("%f\n", isMovingValue);
    }
    else
    {
        printf("\nThe chair is not moving\n");
    }

    // Detection de l'angle de la chaise
    printf("\nValeurs centrales inertielles et angle de la chaise - DEBUG\n");

    printf("Acceleration Mobile: \t");
    printf("%f", aRealMobile[0]);
    printf("\t");
    printf("%f", aRealMobile[1]);
    printf("\t");
    printf("%f\n", aRealMobile[2]);

    printf("Acceleration Fixe: \t");
    printf("%f", aRealFixe[0]);
    printf("\t");
    printf("%f", aRealFixe[1]);
    printf("\t");
    printf("%f\n", aRealFixe[2]);

    printf("pitch/roll Mobile \t");
    printf("%f", pitchMobile);
    printf("\t");
    printf("%f\n", rollMobile);

    printf("pitch/roll Fixe \t");
    printf("%f", pitchFixe);
    printf("\t");
    printf("%f\n", rollFixe);

    printf("Angle entre les capteurs : ");
    printf("%i\n", angle);

    printf("\n.-.--..---.-.-.--.--.--.---.--.-");
    printf("\n");
    printf("CASE 1");
    printf("\t");
    printf("%f\n", pitchFixe - pitchMobile);
    printf("CASE 2");
    printf("\t");
    printf("%f\n", rollFixe - pitchMobile);
    printf("CASE 3");
    printf("\t");
    printf("%f\n", 90.0f - pitchFixe - pitchMobile);
    printf("CASE 4");
    printf("\t");
    printf("%f\n", rollFixe - rollMobile);
    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
}
