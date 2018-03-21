//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

#include "MPU6050.h"  //Implementation of Jeff Rowberg's driver
#include "MAX11611.h" //10-Bit ADC

#include "init.h"         //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "forceSensor.h"  //variables and modules initialisation
#include "forcePlate.h"   //variables and modules initialisation
#include "humiditySensor.h"
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation

#include <stdio.h>
#include <unistd.h>

extern bool testSequence;
extern bool affichageSerial;

extern unsigned char dateTime[7]; //{s, m, h, w, d, date, month, year}

extern MCP79410 mcp79410; //Initialisation of the MCP79410


bool program_test(Alarm &alarm, DateTimeRTC *datetimeRTC, BackSeatAngleTracker &imu, uint16_t* max11611Data, forceSensor &sensorMatrix, forcePlate &globalForcePlate)
{
		printf("\n.-.--..--- TEST MENU .--.---.--.-\n");
		printf("Select one of the following options.\n");

		printf("\n\nFunction testing :");
		printf("\n\tTestID\tDescription");
		printf("\n\t a \t Activate GREEN LED on notification module");
		printf("\n\t b \t Activate RED LED on notification module");
		printf("\n\t c \t Activate DC Motor on notification module");
		printf("\n\t d \t Activate blink leds alarm.");
		printf("\n\t e \t Activate red alarm.");
		printf("\n\t f \t Activate force sensors calibration");
		printf("\n\t g \t Check force sensors centers of pressure");
		printf("\n\t h \t De-activate all R&G LED + DC Motor");
		printf("\n\t i \t Activate IMU calibration");
		printf("\n\t j \t Detect relative pressure in quadrants");

		printf("\n\nTest options :");
		printf("\n\t p \t Stop printing outputs (function print_stuff())");
		printf("\n\t P \t Start printing outpus (function print_stuff()");
		printf("\n\t z \t Test sequence start (UNUSED ?)");
		printf("\n\t y \t Test sequence end (UNUSED ?");
		printf("\n\t x \t Print date and time");
		printf("\n\t q \t Return true (USEFULL?)");

		printf("\n\nType in TestID then press the return key\n");
		//char inSerialChar = (char)Serial.read();
    char inSerialChar = getchar();

    //---------------------------------------------------------------------------------------
    // TEST DU MODULE NOTIFICATION : OK
    // DEL Red/Green, MoteurDC, Bouton poussoir
    //---------------------------------------------------------------------------------------

    if (inSerialChar == 'a')
    {
        printf("Allumer la DEL verte.\n");
		    alarm.TurnOnGreenLed();
    }
    else if (inSerialChar == 'b')
    {
        printf("Allumer la DEL rouge.\n");
		    alarm.TurnOnRedLed();
    }
    else if (inSerialChar == 'c')
    {
        printf("Activer le moteur DC.\n");
		int count = 0;
        while (count++ < 70)
        {
			alarm.TurnOnDCMotor();
			usleep(0.1 * 1000 * 1000);
		}
		alarm.TurnOffDCMotor();
	}
    else if (inSerialChar == 'd')
    {
		printf("Activer une alarme <rouge/verte>.\n");
		alarm.TurnOnBlinkLedsAlarm();
		inSerialChar = 'x';
    }
    else if (inSerialChar == 'e')
    {
		printf("Activer une alarme <rouge/moteur>.\n");
		alarm.TurnOnRedAlarm();
        inSerialChar = 'x';
    }
    else if (inSerialChar == 'f')
    {
			printf("\nDEBUG CALIBRATION FORCE SENSORS START");
			printf("\nFunction(s) under test:");
			printf("\n CalibrateForceSensor()");
			printf("\n IsUserDetected()");

			//Calibration
			getData(max11611Data, sensorMatrix);
	    sensorMatrix.CalibrateForceSensor(max11611Data, sensorMatrix);
			//Last sensed presence analog reading to compare with calibration
			long sensedPresence = 0;
			for (int i = 0; i < sensorMatrix.sensorCount; i++)
			{
				sensedPresence += sensorMatrix.GetAnalogData(i);
			}
			if(sensorMatrix.sensorCount != 0)
			{
				sensedPresence /= sensorMatrix.sensorCount;
			}

	    printf("\n.-.--..---DERNIERE MESURE DES CAPTEURS EN TEMPS REEL.--.---.--.-\n");
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

	    printf("\n.-.--..---.-.OFFSET INITIAUX.--.---.--.-\n");
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

		else if (inSerialChar == 'g')
		{
			printf("\nDEBUG CENTER OF PRESSURE FORCE SENSORS START");
			printf("\nFunction(s) under test:");
			printf("\n DetectCenterOfPressure()");
			printf("\n\t CreateForcePlate()");
			printf("\n\t AnalyseForcePlate()");

			getData(max11611Data, sensorMatrix);
			globalForcePlate.DetectCenterOfPressure(globalForcePlate, sensorMatrix);

			printf("\n.-.--..---MESURE DES CAPTEURS DE FORCE--.---.--.-\n");
			printf("Sensor Number \t Analog value \t Voltage (mV) \t Force (N) \n");
	    printf("Sensor No: 1 \t %i \t\t %lu \t\t %f \n", sensorMatrix.GetAnalogData(0), sensorMatrix.GetVoltageData(0), sensorMatrix.GetForceData(0));
	    printf("Sensor No: 2 \t %i \t\t %lu \t\t %f \n", sensorMatrix.GetAnalogData(1), sensorMatrix.GetVoltageData(1), sensorMatrix.GetForceData(1));
	    printf("Sensor No: 3 \t %i \t\t %lu \t\t %f \n", sensorMatrix.GetAnalogData(2), sensorMatrix.GetVoltageData(2), sensorMatrix.GetForceData(2));
	    printf("Sensor No: 4 \t %i \t\t %lu \t\t %f \n", sensorMatrix.GetAnalogData(3), sensorMatrix.GetVoltageData(3), sensorMatrix.GetForceData(3));
	    printf("Sensor No: 5 \t %i \t\t %lu \t\t %f \n", sensorMatrix.GetAnalogData(4), sensorMatrix.GetVoltageData(4), sensorMatrix.GetForceData(4));
	    printf("Sensor No: 6 \t %i \t\t %lu \t\t %f \n", sensorMatrix.GetAnalogData(5), sensorMatrix.GetVoltageData(5), sensorMatrix.GetForceData(5));
	    printf("Sensor No: 7 \t %i \t\t %lu \t\t %f \n", sensorMatrix.GetAnalogData(6), sensorMatrix.GetVoltageData(6), sensorMatrix.GetForceData(6));
	    printf("Sensor No: 8 \t %i \t\t %lu \t\t %f \n", sensorMatrix.GetAnalogData(7), sensorMatrix.GetVoltageData(7), sensorMatrix.GetForceData(7));
	    printf("Sensor No: 9 \t %i \t\t %lu \t\t %f \n", sensorMatrix.GetAnalogData(8), sensorMatrix.GetVoltageData(8), sensorMatrix.GetForceData(8));
			printf(".-.--..---.-.-.--.--.--.---.--.-\n");

			printf("\n.-.--..---MESURE DES CENTRES DE PRESSION.--.---.--.-\n");
			printf("Position relative au centre des quadrants sur le siege (cm) \n");
			printf("COP Axis \t forcePlate1 \t forcePlate2 \t forcePlate3 \t forcePlate4 \n");
			printf("COP (X): \t %f \t %f \t %f \t %f \n", globalForcePlate.GetFp1COPx(), globalForcePlate.GetFp2COPx(), globalForcePlate.GetFp3COPx(), globalForcePlate.GetFp4COPx());
			printf("COP (Y): \t %f \t\%f \t %f \t %f \n", globalForcePlate.GetFp1COPy(), globalForcePlate.GetFp2COPy(), globalForcePlate.GetFp3COPy(), globalForcePlate.GetFp4COPy());

			printf("\nPosition globale relative au centre du siege (cm) \n");
			printf("COP Axis \t globalForcePlate \n");
			printf("COP (X): \t %f \n", globalForcePlate.GetCOPx());
			printf("COP (Y): \t %f \n", globalForcePlate.GetCOPy());
			printf(".-.--..---.-.-.--.--.--.---.--.-");printf("\n");
		}

    else if (inSerialChar == 'h')
    {
		printf("Eteindre les DELs et arrêter le moteur DC.\n");
		// alarm.TurnOffAlarm();
		imu.GetBackSeatAngle();
    }
		else if (inSerialChar == 'i')
    {
        calibrationProcess(imuFixe, 0);
        delay(50);
        calibrationProcess(imuMobile, 0);
        printf("Calibration des capteurs effectuée.\n");
        inSerialChar = 'x';
    }
		else if (inSerialChar == 'j')
		{
			printf("\nDEBUG RELATIVE PRESSURE IN QUADRANTS START");
			printf("\nFunction(s) under test:");
			printf("\n DetectRelativePressure()\n");

			getData(max11611Data, sensorMatrix);

			printf("\n.-.--..---MESURE DES PRESSIONS RELATIVES DES QUADRANTS--.---.--.-\n");
			int *relativePressureLevel = sensorMatrix.DetectRelativePressure(sensorMatrix);
			for (int i = 0; i < 4; i++)
			{
					printf("\nQuadrant %d : ", (i + 1));
					//printf(" %d \n", *(relativePressureLevel + i));
					if (*(relativePressureLevel + i) == 1)
					{
						printf("Really low");
					}
					else if (*(relativePressureLevel + i) == 2)
					{
						printf("Low");
					}
					else if (*(relativePressureLevel + i) == 3)
					{
						printf("Normal");
					}
					else if (*(relativePressureLevel + i) == 4)
					{
						printf("High");
					}
					else if (*(relativePressureLevel + i) == 5)
					{
						printf("Really high");
					}
					else if (*(relativePressureLevel + i) == 0)
					{
						printf("No pressure data to read");
					}
					else
					{
						//Reading error
					}
			}
			printf("\n");
		}


    //---------------------------------------------------------------------------------------
    // OPTION DE DEBUG : OK
    // Sequence de test ON-OFF, Output print ON-OFF
    //---------------------------------------------------------------------------------------
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

    else if (inSerialChar == 'z')
    {
        printf("Début de séquence de test en cours.\n");
        testSequence = true;
    }
    else if (inSerialChar == 'y')
    {
        printf("Fin de séquence de test en cours.\n");
        testSequence = false;
    }
    else if (inSerialChar == 'i')
    {
        //imu.SetCalibrationArray(fixedImu, 0);
        delay(50);
        //imu.SetCalibrationArray(imuMobile, 0);
        printf("Calibration des capteurs effectuée.\n");
        inSerialChar = 'x';
    }
    else if (inSerialChar == 'x')
    {
        std::string dt = datetimeRTC->getFormattedDateTime();

        // Date and time
        printf("Affichage de la date + heure: %s\n", dt.c_str());
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
    // printf("\nAffichage de la date + heure\n");
    // printf("20");
    // printf("%X\n", dateTime[6]);
    // printf(".");
    // printf("%X\n", dateTime[5]);
    // printf(".");
    // printf("%X\n", dateTime[4]);
    // printf(" ");
    // printf("%X\n", dateTime[2]);
    // printf(":");
    // printf("%X\n", dateTime[1]);
    // printf(":");
    // printf("%X\n", dateTime[0]);
    // printf("\n");

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
    /*
	if (imu.IsMoving())
    {
        printf("The chair is moving.\n");    }
    else
    {
        printf("\nThe chair is not moving.\n");
    }
	*/
    // Detection de l'angle de la chaise
    printf("\nValeurs centrales inertielles et angle de la chaise - DEBUG\n");

	/*
    printf("Acceleration Mobile: \t");
    printf("%f", realMobileAccelerations[0]);
    printf("\t");
    printf("%f", realMobileAccelerations[1]);
    printf("\t");
    printf("%f\n", realMobileAccelerations[2]);

    printf("Acceleration Fixe: \t");
    printf("%f", realFixedAccelerations[0]);
    printf("\t");
    printf("%f", realFixedAccelerations[1]);
    printf("\t");
    printf("%f\n", realFixedAccelerations[2]);
	*/

    printf("Angle entre les capteurs : ");
    // printf("%i\n", imu.GetBackSeatAngle());
}
