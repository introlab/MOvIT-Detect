//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

#include "MPU6050.h"  //Implementation of Jeff Rowberg's driver
#include "MAX11611.h" //10-Bit ADC
#include "MCP79410.h" //Custom driver that uses I2Cdev.h to get RTC data

#include "init.h"         //variables and modules initialisation
#include "notif_module.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "forceSensor.h" //variables and modules initialisation
#include "forcePlate.h" //variables and modules initialisation
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation


#include <stdio.h>
#include <unistd.h>

extern bool testSequence;
extern bool affichageSerial;

extern float isMovingValue;

extern unsigned char dateTime[7]; //{s, m, h, w, d, date, month, year}

extern MPU6050 imuMobile; //Initialisation of the mobile MPU6050
extern MPU6050 imuFixe;   //Initialisation of the fixed MPU6050
extern MCP79410 mcp79410; //Initialisation of the MCP79410

extern double aRealFixe[3], aRealMobile[3];                 //computed acceleration
extern double pitchFixe, rollFixe, pitchMobile, rollMobile; //pitch and roll angle values
extern int angle;                                           //Final angle computed between sensors
// extern int obs;                                             //Debug variable for getAngle function

extern uint16_t max11611Data[9];                     //ADC 10-bit data variable
extern uint16_t max11611DataArray[9]; //Data table of size=total sensors

extern forceSensor sensorMatrix;


bool program_test(Alarm &alarm)
{
    // Alarm alarm(700, 0.1);

    //char inSerialChar = (char)Serial.read();
		printf("Enter user command and press enter \n");
    char inSerialChar = getchar();

    //---------------------------------------------------------------------------------------
    // TEST DU MODULE NOTIFICATION : OK
    // DEL Red/Green, MoteurDC, Bouton poussoir
    //---------------------------------------------------------------------------------------

    if (inSerialChar == 'g')
    {
        printf("Allumer la DEL verte.\n");
        alarm.SetIsGreenLedEnabled(true);
        alarm.SetPinState(GREEN_LED, IO_HIGH, true);
    }
    else if (inSerialChar == 'r')
    {
        printf("Allumer la DEL rouge.\n");
        alarm.SetIsRedLedEnabled(true);
        alarm.SetPinState(RED_LED, IO_HIGH, true);
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
    }
    else if (inSerialChar == 'b')
    {
        printf("Verifier l'etat du bouton poussoir.\n");
        printf("Etat du bouton = %i\n", !alarm.GetPinState(PUSH_BUTTON));
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
			printf("\nDEBUG CALIBRATION FORCE SENSORS START");
			//Calibration
	    sensorMatrix.CalibrateForceSensor(sensorMatrix);
			//Last sensed presence analog reading to compare with calibration
			long sensedPresence = 0;
			for (int i = 0; i < sensorMatrix.sensorCount; i++)
			{
				sensedPresence += sensorMatrix.GetAnalogData(i);
			}

	    printf("\n.-.--..---DERNIERE MESURE DES CAPTEURS EN TEMPS REEL.--.---.--.-");printf("\n");
	    printf("Sensor Number \t Analog value \t Voltage (mV) \t Force (N) \n");
	    printf("Sensor No: 1 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(0), sensorMatrix.GetVoltageData(0), sensorMatrix.GetForceData(0));
	    printf("Sensor No: 2 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(1), sensorMatrix.GetVoltageData(1), sensorMatrix.GetForceData(1));
	    printf("Sensor No: 3 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(2), sensorMatrix.GetVoltageData(2), sensorMatrix.GetForceData(2));
	    printf("Sensor No: 4 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(3), sensorMatrix.GetVoltageData(3), sensorMatrix.GetForceData(3));
	    printf("Sensor No: 5 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(4), sensorMatrix.GetVoltageData(4), sensorMatrix.GetForceData(4));
	    printf("Sensor No: 6 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(5), sensorMatrix.GetVoltageData(5), sensorMatrix.GetForceData(5));
	    printf("Sensor No: 7 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(6), sensorMatrix.GetVoltageData(6), sensorMatrix.GetForceData(6));
	    printf("Sensor No: 8 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(7), sensorMatrix.GetVoltageData(7), sensorMatrix.GetForceData(7));
	    printf("Sensor No: 9 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(8), sensorMatrix.GetVoltageData(8), sensorMatrix.GetForceData(8));
			printf(".-.--..---.-.-.--.--.--.---.--.-\n");

	    printf("\n.-.--..---.-.OFFSET INITIAUX.--.---.--.-\n");
	    printf("Sensor Number\t");
			printf("Analog value\n");

	    printf("Sensor No: 1 \t %lu \n", sensorMatrix.GetAnalogOffset(0));
	    printf("Sensor No: 2 \t %lu \n", sensorMatrix.GetAnalogOffset(1));
	    printf("Sensor No: 3 \t %lu \n", sensorMatrix.GetAnalogOffset(2));
	    printf("Sensor No: 4 \t %lu \n", sensorMatrix.GetAnalogOffset(3));
	    printf("Sensor No: 5 \t %lu \n", sensorMatrix.GetAnalogOffset(4));
	    printf("Sensor No: 6 \t %lu \n", sensorMatrix.GetAnalogOffset(5));
	    printf("Sensor No: 7 \t %lu \n", sensorMatrix.GetAnalogOffset(6));
	    printf("Sensor No: 8 \t %lu \n", sensorMatrix.GetAnalogOffset(7));
	    printf("Sensor No: 9 \t %lu \n", sensorMatrix.GetAnalogOffset(8));
	    printf(".-.--..---.-.-.--.--.--.---.--.-\n");

	    printf("\n.-.--..---.-.OFFSET INITIAUX.--.---.--.-");printf("\n");
	    printf("Total sensor mean : \t %i \n", sensorMatrix.GetTotalSensorMean());
	    printf(".-.--..---.-.-.--.--.--.---.--.-\n");

			printf("\n.-.--..---DETECTION DUNE PERSONNE SUR LA CHAISE--.---.--.-\n");
	    printf("Detected Presence : %lu \n", sensedPresence);
	    printf("Detection Threshold : %f \n", sensorMatrix.GetDetectionThreshold());
	    printf("Presence verification result : ");
	    if(sensorMatrix.IsUserDetected(sensorMatrix))
	    {
	       printf("User detected \n");
	    }
	    else
	    {
	      printf("No user detected \n");
	    }
	    printf(".-.--..---.-.-.--.--.--.---.--.-\n\n");

			inSerialChar = 'x';
    }

		else if (inSerialChar == 'v')
		{
			getData();

			printf("\nDEBUG CALIBRATION FORCE SENSORS START\n");
			//Force plates variables
			forcePlate forcePlate1;
			forcePlate forcePlate2;
			forcePlate forcePlate3;
			forcePlate forcePlate4;

			//Creation of the 4 ForcePlates
			forcePlate1.CreateForcePlate(forcePlate1, sensorMatrix, 4, 1, 0, 3);
			forcePlate2.CreateForcePlate(forcePlate2, sensorMatrix, 7, 4, 3, 6);
			forcePlate3.CreateForcePlate(forcePlate3, sensorMatrix, 5, 2, 1, 4);
			forcePlate4.CreateForcePlate(forcePlate4, sensorMatrix, 8, 5, 4, 7);

			//Global analysis of the 4 plates (treated as one plate)
			forcePlate globalForcePlate;
			globalForcePlate.AnalyzeForcePlates(globalForcePlate, sensorMatrix, forcePlate1, forcePlate2, forcePlate3, forcePlate4);

			printf("\n.-.--..---MESURE DES CAPTEURS DE FORCE--.---.--.-");printf("\n");
			printf("Sensor Number \t Analog value \t Voltage (mV) \t Force (N) \n");
	    printf("Sensor No: 1 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(0), sensorMatrix.GetVoltageData(0), sensorMatrix.GetForceData(0));
	    printf("Sensor No: 2 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(1), sensorMatrix.GetVoltageData(1), sensorMatrix.GetForceData(1));
	    printf("Sensor No: 3 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(2), sensorMatrix.GetVoltageData(2), sensorMatrix.GetForceData(2));
	    printf("Sensor No: 4 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(3), sensorMatrix.GetVoltageData(3), sensorMatrix.GetForceData(3));
	    printf("Sensor No: 5 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(4), sensorMatrix.GetVoltageData(4), sensorMatrix.GetForceData(4));
	    printf("Sensor No: 6 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(5), sensorMatrix.GetVoltageData(5), sensorMatrix.GetForceData(5));
	    printf("Sensor No: 7 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(6), sensorMatrix.GetVoltageData(6), sensorMatrix.GetForceData(6));
	    printf("Sensor No: 8 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(7), sensorMatrix.GetVoltageData(7), sensorMatrix.GetForceData(7));
	    printf("Sensor No: 9 \t %i \t\t %lu \t\t  %f \n", sensorMatrix.GetAnalogData(8), sensorMatrix.GetVoltageData(8), sensorMatrix.GetForceData(8));
			printf(".-.--..---.-.-.--.--.--.---.--.-\n");

			printf("\n.-.--..---MESURE DES CENTRES DE PRESSION.--.---.--.-");printf("\n");
			printf("Position relative au centre des quadrants sur le siege (cm) \n");
			printf("COP Axis \t forcePlate1 \t\t forcePlate2 \t\t forcePlate3 \t\t forcePlate4 \n");
			printf("COP (X): \t %f \t\t %f \t\t %f \t\t %f \n", forcePlate1.GetCOPx(), forcePlate2.GetCOPx(), forcePlate3.GetCOPx(), forcePlate4.GetCOPx());
			printf("COP (Y): \t %f \t\t %f \t\t %f \t\t %f \n", forcePlate1.GetCOPy(), forcePlate2.GetCOPy(), forcePlate3.GetCOPy(), forcePlate4.GetCOPy());

			printf("\nPosition globale relative au centre du siege (cm) \n");
			printf("COP Axis \t globalForcePlate \n");
			printf("COP (X): \t %f \n", globalForcePlate.GetCOPx());
			printf("COP (Y): \t %f \n", globalForcePlate.GetCOPy());
			printf(".-.--..---.-.-.--.--.--.---.--.-");printf("\n");
		}

    else if (inSerialChar == 'z')
    {
		printf("Eteindre les DELs et arrêter le moteur DC.\n");
		alarm.SetPinState(DC_MOTOR, IO_LOW, true);
		alarm.SetPinState(RED_LED, IO_LOW, true);
		alarm.SetPinState(GREEN_LED, IO_LOW, true);
    }
    //---------------------------------------------------------------------------------------
    // OPTION DE DEBUG : OK
    // Sequence de test ON-OFF, Output print ON-OFF
    //---------------------------------------------------------------------------------------
    else if (inSerialChar == 'd')
    {
        printf("Début de séquence de test en cours.\n");
        testSequence = true;
    }
    else if (inSerialChar == 'f')
    {
        printf("Fin de séquence de test en cours.\n");
        testSequence = false;
    }
    else if (inSerialChar == 'p')
    {
        printf("Ne pas afficher les sorties.\n");
        affichageSerial = false;
    }
    else if (inSerialChar == 'P')
    {
        printf("Afficher les sorties.\n");
        affichageSerial = true;
    }

    // else if (inSerialChar == 'c')
    // {
    //     calibrationProcess(imuFixe, 0);
    //     delay(50);
    //     calibrationProcess(imuMobile, 0);
    //     printf("Calibration des capteurs effectuée.\n");
    //     inSerialChar = 'x';
    // }

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
        printf("MOTOR ON\n");
        alarm.StartBuzzer();
        usleep(1000000);
        printf("MOTOR OFF\n");
        alarm.StopBuzzer();
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
    else if (inSerialChar == 'l')
    {
        getData();
    }
    else if (inSerialChar == 'q')
    {
        return true;
    }

    return false;
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
