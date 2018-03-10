//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

//Include : Drivers
#include "MPU6050.h"  //Implementation of Jeff Rowberg's driver
#include "MAX11611.h" //10-Bit ADC
#include "MCP79410.h" //Custom driver that uses I2Cdev.h to get RTC data

//Include : Modules
#include "init.h"
#include "notif_module.h"
#include "accel_module.h"
#include "ForceSensor.h"
#include "ForcePlate.h"
#include "program.h"
#include "test.h"

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

// extern uint16_t *max11611Data;                     //ADC 10-bit data variable
// extern uint16_t max11611DataArray[sensorCount]; //Data table of size=total sensors
// extern long max11611Voltage[sensorCount];       //ADC 10-bit data variable
// extern long max11611Resistance[sensorCount];    //ADC 10-bit data variable
// extern long max11611Conductance[sensorCount];   //ADC 10-bit data variable
// extern long max11611Force[sensorCount];         //ADC 10-bit data variable

extern ForceSensor sensorMatrix;

extern long detectedPresence;
extern long detectionThreshold;

extern bool left_shearing;
extern bool right_shearing;
extern bool front_shearing;
extern bool rear_shearing;

bool program_test(Alarm &alarm)
{
    // Alarm alarm(700, 0.1);

    //char inSerialChar = (char)Serial.read();
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
    }
    else if (inSerialChar == 'c')
    {
        printf("\nDEBUG CALIBRATION FORCE SENSORS START\n");
        sensorMatrix.CalibrateForceSensor(sensorMatrix);

        printf("\n.-.--..---.-.OFFSET INITIAUX.--.---.--.-");
        printf("\n");
        printf("Sensor Number\t");
        printf("Analog value\n");
        printf("Sensor No: 1 \t");
        printf("%li\n", sensorMatrix.getAnalogOffset(0));
        printf("Sensor No: 2 \t");
        printf("%li\n", sensorMatrix.getAnalogOffset(1));
        printf("Sensor No: 3 \t");
        printf("%li\n", sensorMatrix.getAnalogOffset(2));
        printf("Sensor No: 4 \t");
        printf("%li\n", sensorMatrix.getAnalogOffset(3));
        printf("Sensor No: 5 \t");
        printf("%li\n", sensorMatrix.getAnalogOffset(4));
        printf("Sensor No: 6 \t");
        printf("%li\n", sensorMatrix.getAnalogOffset(5));
        printf("Sensor No: 7 \t");
        printf("%li\n", sensorMatrix.getAnalogOffset(6));
        printf("Sensor No: 8 \t");
        printf("%li\n", sensorMatrix.getAnalogOffset(7));
        printf("Sensor No: 9 \t");
        printf("%li\n", sensorMatrix.getAnalogOffset(8));
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\n");

        printf("\n.-.--..---.-.OFFSET INITIAUX.--.---.--.-");
        printf("\n");
        printf("Total sensor mean : \t");
        printf("%i\n", sensorMatrix.getTotalSensorMean());
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\n");

        printf("\n.-.--..---DERNIERE MESURE DES CAPTEURS EN TEMPS REEL.--.---.--.-");
        printf("\n");
        printf("Sensor Number\t");
        printf("Analog value\t");
        printf("Voltage (mV)\t");
        printf("Force (N)\n");
        printf("Sensor No: 1 \t");
        printf("%i", sensorMatrix.getAnalogData(0));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(0));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(0));
        printf("Sensor No: 2 \t");
        printf("%i", sensorMatrix.getAnalogData(1));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(1));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(1));
        printf("Sensor No: 3 \t");
        printf("%i", sensorMatrix.getAnalogData(2));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(2));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(2));
        printf("Sensor No: 4 \t");
        printf("%i", sensorMatrix.getAnalogData(3));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(3));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(3));
        printf("Sensor No: 5 \t");
        printf("%i", sensorMatrix.getAnalogData(4));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(4));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(4));
        printf("Sensor No: 6 \t");
        printf("%i", sensorMatrix.getAnalogData(5));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(5));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(5));
        printf("Sensor No: 7 \t");
        printf("%i", sensorMatrix.getAnalogData(6));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(6));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(6));
        printf("Sensor No: 8 \t");
        printf("%i", sensorMatrix.getAnalogData(7));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(7));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(7));
        printf("Sensor No: 9 \t");
        printf("%i", sensorMatrix.getAnalogData(8));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(8));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(8));
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\n");
    }
    else if (inSerialChar == 'v')
    {
        printf("\n.-.--..---MESURE DES CAPTEURS DE FORCE--.---.--.-");
        printf("\n");
        printf("Sensor Number\t");
        printf("Analog value\t");
        printf("Voltage (mV)\t");
        printf("Force (N)\n");
        printf("Sensor No: 1 \t");
        printf("%i", sensorMatrix.getAnalogData(0));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(0));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(0));
        printf("Sensor No: 2 \t");
        printf("%i", sensorMatrix.getAnalogData(1));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(1));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(1));
        printf("Sensor No: 3 \t");
        printf("%i", sensorMatrix.getAnalogData(2));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(2));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(2));
        printf("Sensor No: 4 \t");
        printf("%i", sensorMatrix.getAnalogData(3));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(3));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(3));
        printf("Sensor No: 5 \t");
        printf("%i", sensorMatrix.getAnalogData(4));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(4));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(4));
        printf("Sensor No: 6 \t");
        printf("%i", sensorMatrix.getAnalogData(5));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(5));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(5));
        printf("Sensor No: 7 \t");
        printf("%i", sensorMatrix.getAnalogData(6));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(6));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(6));
        printf("Sensor No: 8 \t");
        printf("%i", sensorMatrix.getAnalogData(7));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(7));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(7));
        printf("Sensor No: 9 \t");
        printf("%i", sensorMatrix.getAnalogData(8));
        printf("\t\t");
        printf("%li", sensorMatrix.getVoltageData(8));
        printf("\t\t");
        printf("%f\n", sensorMatrix.getForceData(8));
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\n");

        printf("\n.-.--..---DETECTION DUNE PERSONNE SUR LA CHAISE--.---.--.-");
        printf("\n");
        printf("Detected Presence : ");
        printf("%li\n", detectedPresence);
        printf("Detection Threshold : ");
        printf("%li\n", detectionThreshold);
        printf("Presence verification result : ");
        if (sensorMatrix.isPresenceDetected(sensorMatrix))
        {
            printf("User detected\n");
        }
        else
        {
            printf("No user detected\n");
        }
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\n");

        printf("\n.-.--..---MESURE DES CENTRES DE PRESSION.--.---.--.-");
        printf("\n");
        //Force plates variables
        ForcePlate ForcePlate1;
        ForcePlate ForcePlate2;
        ForcePlate ForcePlate3;
        ForcePlate ForcePlate4;

        //Creation of the 4 ForcePlates
        ForcePlate1.CreateForcePlate(ForcePlate1, sensorMatrix, 5, 2, 1, 4);
        ForcePlate1.CreateForcePlate(ForcePlate2, sensorMatrix, 8, 5, 4, 7);
        ForcePlate1.CreateForcePlate(ForcePlate3, sensorMatrix, 6, 3, 2, 5);
        ForcePlate1.CreateForcePlate(ForcePlate4, sensorMatrix, 9, 6, 5, 8);

        //Global analysis of the 4 plates (treated as one plate)
        ForcePlate GlobalForcePlate;
        GlobalForcePlate.AnalyzeForcePlates(GlobalForcePlate, sensorMatrix, ForcePlate1, ForcePlate2, ForcePlate3, ForcePlate4);

        printf("ForcePlate1 COP along X-Axis: ");
        printf("%f\n", ForcePlate1.getCOPx());
        printf("                along Y-Axis: ");
        printf("%f\n", ForcePlate1.getCOPy());
        printf("ForcePlate2 COP along X-Axis: ");
        printf("%f\n", ForcePlate2.getCOPx());
        printf("                along Y-Axis: ");
        printf("%f\n", ForcePlate2.getCOPy());
        printf("ForcePlate3 COP along X-Axis: ");
        printf("%f\n", ForcePlate3.getCOPx());
        printf("                along Y-Axis: ");
        printf("%f\n", ForcePlate3.getCOPy());
        printf("ForcePlate4 COP along X-Axis: ");
        printf("%f\n", ForcePlate4.getCOPx());
        printf("                along Y-Axis: ");
        printf("%f\n", ForcePlate4.getCOPy());
        printf("Global COP along X-Axis: ");
        printf("%f\n", GlobalForcePlate.getCOPx());
        printf("           along Y-Axis: ");
        printf("%f\n", GlobalForcePlate.getCOPy());
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\n");
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
    else if (inSerialChar == 'c')
    {
        calibrationProcess(imuFixe, 0);
        usleep(50000);
        calibrationProcess(imuMobile, 0);
        printf("Calibration des capteurs effectuée.\n");
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
    printf("\n.-.--..---.-.-.--.--.--.---.--.-");
    printf("\n");
    printf("Sensor Number\t");
    printf("Analog value\t");
    printf("Voltage (mV)\t");
    printf("Force (N)\n");
    printf("Sensor No: 1 \t");
    printf("%i", sensorMatrix.getAnalogData(0));
    printf("\t\t");
    printf("%li", sensorMatrix.getVoltageData(0));
    printf("\t\t");
    printf("%f\n", sensorMatrix.getForceData(0));
    printf("Sensor No: 2 \t");
    printf("%i", sensorMatrix.getAnalogData(1));
    printf("\t\t");
    printf("%li", sensorMatrix.getVoltageData(1));
    printf("\t\t");
    printf("%f\n", sensorMatrix.getForceData(1));
    printf("Sensor No: 3 \t");
    printf("%i", sensorMatrix.getAnalogData(2));
    printf("\t\t");
    printf("%li", sensorMatrix.getVoltageData(2));
    printf("\t\t");
    printf("%f\n", sensorMatrix.getForceData(2));
    printf("Sensor No: 4 \t");
    printf("%i", sensorMatrix.getAnalogData(3));
    printf("\t\t");
    printf("%li", sensorMatrix.getVoltageData(3));
    printf("\t\t");
    printf("%f\n", sensorMatrix.getForceData(3));
    printf("Sensor No: 5 \t");
    printf("%i", sensorMatrix.getAnalogData(4));
    printf("\t\t");
    printf("%li", sensorMatrix.getVoltageData(4));
    printf("\t\t");
    printf("%f\n", sensorMatrix.getForceData(4));
    printf("Sensor No: 6 \t");
    printf("%i", sensorMatrix.getAnalogData(5));
    printf("\t\t");
    printf("%li", sensorMatrix.getVoltageData(5));
    printf("\t\t");
    printf("%f\n", sensorMatrix.getForceData(5));
    printf("Sensor No: 7 \t");
    printf("%i", sensorMatrix.getAnalogData(6));
    printf("\t\t");
    printf("%li", sensorMatrix.getVoltageData(6));
    printf("\t\t");
    printf("%f\n", sensorMatrix.getForceData(6));
    printf("Sensor No: 8 \t");
    printf("%i", sensorMatrix.getAnalogData(7));
    printf("\t\t");
    printf("%li", sensorMatrix.getVoltageData(7));
    printf("\t\t");
    printf("%f\n", sensorMatrix.getForceData(7));
    printf("Sensor No: 9 \t");
    printf("%i", sensorMatrix.getAnalogData(8));
    printf("\t\t");
    printf("%li", sensorMatrix.getVoltageData(8));
    printf("\t\t");
    printf("%f\n", sensorMatrix.getForceData(8));
    printf(".-.--..---.-.-.--.--.--.---.--.-");
    printf("\n");

    printf("\n.-.--..---DETECTION DUNE PERSONNE SUR LA CHAISE--.---.--.-");
    printf("\n");
    printf("Presence verification result : ");
    if (sensorMatrix.isPresenceDetected(sensorMatrix))
    {
        printf("User detected\n");
    }
    else
    {
        printf("No user detected\n");
    }
    printf(".-.--..---.-.-.--.--.--.---.--.-");
    printf("\n");

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
