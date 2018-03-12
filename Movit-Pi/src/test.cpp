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
#include "forceSensor.h"  //variables and modules initialisation
#include "forcePlate.h"   //variables and modules initialisation
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

bool program_test(Alarm &alarm, uint16_t* max11611Data, forceSensor &sensorMatrix, forcePlate &globalForcePlate)
{
    // Alarm alarm(700, 0.1);
		printf("\n.-.--..--- TEST MENU .--.---.--.-\n");
		printf("Select one of the following options.\n");

		printf("\n\nFunction testing :");
		printf("\n\tTestID\tDescription");
		printf("\n\t a \t Activate GREEN LED on notification module");
		printf("\n\t b \t Activate RED LED on notification module");
		printf("\n\t c \t Activate DC Motor on notification module");
		printf("\n\t d \t Check Push-button state");
		printf("\n\t e \t Activate alarm on notification module");
		printf("\n\t f \t Activate force sensors calibration");
		printf("\n\t g \t Check force sensors centers of pressure");
		printf("\n\t h \t De-activate all R&G LED + DC Motor");
		printf("\n\t i \t Activate IMU calibration");
		printf("\n\t j \t Activate all on notification module");

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
        alarm.SetIsGreenLedEnabled(true);
        alarm.SetPinState(GREEN_LED, IO_HIGH, true);
    }
    else if (inSerialChar == 'b')
    {
        printf("Allumer la DEL rouge.\n");
        alarm.SetIsRedLedEnabled(true);
        alarm.SetPinState(RED_LED, IO_HIGH, true);
    }
    else if (inSerialChar == 'c')
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
    else if (inSerialChar == 'd')
    {
        printf("Verifier l'etat du bouton poussoir.\n");
        printf("Etat du bouton = %i\n", !alarm.GetPinState(PUSH_BUTTON));
    }
    else if (inSerialChar == 'e')
    {
		printf("Activer une alarme sur le module de notifications.\n");
		alarm.SetIsGreenLedEnabled(true);
		alarm.SetIsRedLedEnabled(true);
		alarm.SetIsDCMotorEnabled(true);
		alarm.TurnOnBlinkLedsAlarm();
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

		else if (inSerialChar == 'g')
		{
			printf("\nDEBUG CENTER OF PRESSURE FORCE SENSORS START");
			printf("\nFunction(s) under test:");
			printf("\n DetectCOP()");
			printf("\n\t CreateForcePlate()");
			printf("\n\t AnalyseForcePlate()");

			getData(max11611Data, sensorMatrix);
			globalForcePlate.DetectCOP(globalForcePlate, sensorMatrix);

			printf("\n.-.--..---MESURE DES CAPTEURS DE FORCE--.---.--.-");printf("\n");
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

			printf("\n.-.--..---MESURE DES CENTRES DE PRESSION.--.---.--.-");printf("\n");
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
		alarm.SetPinState(DC_MOTOR, IO_LOW, true);
		alarm.SetPinState(RED_LED, IO_LOW, true);
		alarm.SetPinState(GREEN_LED, IO_LOW, true);
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
        calibrationProcess(imuFixe, 0);
        delay(50);
        calibrationProcess(imuMobile, 0);
        printf("Calibration des capteurs effectuée.\n");
        inSerialChar = 'x';
    }

    else if (inSerialChar == 'j')
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
    else if (inSerialChar == 'x')
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
