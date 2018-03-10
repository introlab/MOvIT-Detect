//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

//Include : Drivers
#include "MPU6050.h"  //Implementation of Jeff Rowberg's driver
#include "MAX11611.h" //10-Bit ADC
#include "MCP79410.h" //Custom driver that uses I2Cdev.h to get RTC data

//Include : Modules
#include "init.h"         //variables and modules initialisation
#include "notif_module.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "forceSensor.h" //variables and modules initialisation
#include "forcePlate.h" //variables and modules initialisation
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation

#include <stdio.h>
#include <unistd.h>

// extern bool sleeping;
extern bool testSequence;
extern bool affichageSerial;

extern float isMovingValue;

extern unsigned char dateTime[7]; //{s, m, h, w, d, date, month, year}

extern MPU6050 imuMobile; //Initialisation of the mobile MPU6050
extern MPU6050 imuFixe;   //Initialisation of the fixed MPU6050
// extern MAX11611 max11611; //Initialisation of the 10-bit ADC
extern MCP79410 mcp79410; //Initialisation of the MCP79410

// extern int aRange;                                          //maximal measurable g-force
// extern int16_t ax, ay, az;                                  //axis accelerations
// extern double aRawFixe[3], aRawMobile[3];                   //raw acceleration
extern double aRealFixe[3], aRealMobile[3];                 //computed acceleration
extern double pitchFixe, rollFixe, pitchMobile, rollMobile; //pitch and roll angle values
extern int angle;                                           //Final angle computed between sensors
// extern int obs;                                             //Debug variable for getAngle function


const uint8_t capteurForceNb = 9;                  //Total number of sensors
extern uint16_t max11611Data[capteurForceNb];                     //ADC 10-bit data variable
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

forceSensor sensorMatrix;

bool program_test()
{
	Alarm alarm(700, 0.1);

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
        forcePlate forcePlate1;
        forcePlate forcePlate2;
        forcePlate forcePlate3;
        forcePlate forcePlate4;

        //Creation of the 4 forcePlates
        forcePlate1.CreateForcePlate(forcePlate1, sensorMatrix, 4, 5, 2, 1);
        forcePlate1.CreateForcePlate(forcePlate2, sensorMatrix, 3, 4, 1, 9);
        forcePlate1.CreateForcePlate(forcePlate3, sensorMatrix, 7, 8, 5, 4);
        forcePlate1.CreateForcePlate(forcePlate4, sensorMatrix, 6, 7, 4, 3);

        //Global analysis of the 4 plates (treated as one plate)
        forcePlate globalForcePlate;
        globalForcePlate.AnalyzeForcePlates(globalForcePlate, sensorMatrix, forcePlate1, forcePlate2, forcePlate3, forcePlate4);

        inSerialChar = 'x';
    }
    else if (inSerialChar == 'g')
    {
		printf("Allumer la DEL verte.\n");
		alarm.SetIsGreenLedEnabled(true);
		alarm.SetPinState(GREEN_LED, IO_HIGH, true);
        inSerialChar = 'x';
    }
    else if (inSerialChar == 'r')
    {
		printf("Allumer la DEL rouge.\n");
		alarm.SetIsRedLedEnabled(true);
		alarm.SetPinState(RED_LED, IO_HIGH, true);
        inSerialChar = 'x';
    }
    else if (inSerialChar == 'm')
    {
		printf("Activer le moteur DC.\n");
		alarm.SetIsDCMotorEnabled(true);
		int count = 0;
		while (count++ < 70)
		{
			alarm.SetPinState(DC_MOTOR, IO_HIGH, true);
			delay(100);
		}
		alarm.SetPinState(DC_MOTOR, IO_LOW, true);
        inSerialChar = 'x';
    }
    else if (inSerialChar == 'b')
    {
		printf("Verifier l'etat du bouton poussoir.\n");
        printf("Etat du bouton = %i\n", !alarm.GetPinState(PUSH_BUTTON));
        inSerialChar = 'x';
    }
    else if (inSerialChar == 'a')
    {
		printf("Activer une alarme sur le module de notifications.\n");
		alarm.SetIsGreenLedEnabled(true);
		alarm.SetIsRedLedEnabled(true);
		alarm.SetIsDCMotorEnabled(true);
		alarm.TurnOnBlinkLedsAlarm();
        inSerialChar = 'x';
    }
    else if (inSerialChar == 'c')
    {

    }
    else if (inSerialChar == 'z')
    {
		printf("Eteindre les DELs et arrêter le moteur DC.\n");
		alarm.SetPinState(DC_MOTOR, IO_LOW, true);
		alarm.SetPinState(RED_LED, IO_LOW, true);
		alarm.SetPinState(GREEN_LED, IO_LOW, true);
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
    else if (inSerialChar == 'n')
    {
        printf("LightOn(GREEN_LED);\n");
		    alarm.SetPinState(GREEN_LED, IO_HIGH, true);
		    printf("LightOn(RED_LED);\n");
		    alarm.SetPinState(RED_LED, IO_HIGH, true);
		    usleep(2000000);
        printf("LightOff(GREEN_LED);\n");
		    alarm.SetPinState(GREEN_LED, IO_LOW, true);
		    usleep(1000000);
        printf("LightOff(RED_LED);\n");
		    alarm.SetPinState(RED_LED, IO_LOW, true);
        inSerialChar = 'x';
    }
    else if (inSerialChar == 'v')
    {
        uint8_t mydateTime[7]; //{s, m, h, w, d, date, month, year}

        mcp79410.getDateTime(mydateTime);

        // Date and time
        printf("\nAffichage de la date + heure\n");
        printf("20");
        printf("%X\n", mydateTime[6]);
        printf(".");
        printf("%X\n", mydateTime[5]);
        printf(".");
        printf("%X\n", mydateTime[4]);
        printf(" ");
        printf("%X\n", mydateTime[2]);
        printf(":");
        printf("%X\n", mydateTime[1]);
        printf(":");
        printf("%X\n", mydateTime[0]);
        printf("\n");
    }
    else if (inSerialChar == 'q')
    {
        return true;
    }

    return false;
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

    //}
    //Voltage values
    printf("\nValeurs Capteurs de force : Voltage reading (mV)- DEBUG\n");
    //for (uint16_t capteurNo = 0; capteurNo < capteurForceNb; capteurNo++)
    //{


    //Force values
    printf("\nValeurs Capteurs de force : Force reading (N)- DEBUG\n");
    //for (uint16_t capteurNo = 0; capteurNo < capteurForceNb; capteurNo++)
    //{

    // Verifier si une personne est assise

    //Verifier le positionnement de la personne


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
