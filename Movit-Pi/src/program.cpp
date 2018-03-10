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

//External functions and variables
//Variables
// extern SimpleTimer timer;
extern uint8_t blinkDuree;
extern uint8_t blinkFreq;
extern bool blink_enabled;
extern bool redLedEnabled;
extern bool greenLedEnabled;
extern bool buzzerEnabled;
extern bool currentLedState;

extern bool sleeping;
extern bool testSequence;
extern bool affichageSerial;

extern float isMovingValue;

extern unsigned char dateTime[7]; //{s, m, h, w, d, date, month, year}

extern MPU6050 imuMobile; //Initialisation of the mobile MPU6050
extern MPU6050 imuFixe;   //Initialisation of the fixed MPU6050
extern MAX11611 max11611; //Initialisation of the 10-bit ADC
extern MCP79410 mcp79410; //Initialisation of the MCP79410

extern int aRange;                                          //maximal measurable g-force
extern int16_t ax, ay, az;                                  //axis accelerations
extern double aRawFixe[3], aRawMobile[3];                   //raw acceleration
extern double aRealFixe[3], aRealMobile[3];                 //computed acceleration
extern double pitchFixe, rollFixe, pitchMobile, rollMobile; //pitch and roll angle values
extern int angle;                                           //Final angle computed between sensors
extern int obs;                                             //Debug variable for getAngle function

extern uint16_t max11611Data[9];                     //ADC 10-bit data variable
extern uint16_t max11611DataArray[9]; //Data table of size=total sensors

extern forceSensor sensorMatrix;

extern bool bIsBtnPushed;

//Functions
extern bool isSomeoneThere();
extern bool isMoving();
extern void printStuff();
extern void light_state();
extern void buzzer_state();


bool program_loop(Alarm &alarm)
{
    bIsBtnPushed = false;

    // timer.run();
    bool ret = program_test(alarm);

    //     if (sleeping)
    //     {
    //         if (isSomeoneThere() || testSequence == true)
    //         {
    //             imuFixe.setSleepEnabled(false);
    //             //imuMobile.setSleepEnabled(false);
    //             sleeping = false;
    //         }
    //         else
    //         {
    //             //Envoie tout de même les valeurs à la page périphériques pour le troobleshooting au besoin
    //             if (SerialUSB.available() > 1)
    //             {
    // #ifdef DEBUG_SERIAL
    //                 printf("Something has been detected on the SerialUSB port...\n");
    // #endif
    //                 // Get the command / anything that is on the port
    //                                       JsonObject &cmd = getCmd();

    //                 // Repond seulement pour les demandes de type captForceReq, devrait aussi repondre aux boutons de la page Peripheriques ?
    //                 if (cmd["type"] == "captForceReq")
    //                 {
    //                     //if the parsing of what was on port worked then send a positive response, else send error
    //                     bool error = false;
    //                     if (!cmd.success())
    //                     {
    //                         error = true;
    //                     }
    //                     sendData(cmd, isMoving(), error);
    //                 }
    //             }
    //         }
    //     }

    //     else if (!isSomeoneThere() && testSequence == false)
    //     {
    //         imuFixe.setSleepEnabled(true);
    //         //imuMobile.setSleepEnabled(false);
    //         sleeping = true;

    //         //Envoie une fois, "-" à toutes les valeurs en signe de "No Data" pour toutes les valeurs de real time data affichées
    //         if (SerialUSB.available() > 1)
    //         {
    //             DynamicJsonBuffer jsonBuffer;
    //             JsonObject &response = jsonBuffer.createObject();

    //             response["angle"] = "-";
    //             response["isMoving"] = "-";

    //             // Send response if the object was correctly create
    //             if (response.success())
    //             {
    //                 response.printTo(SerialUSB);
    //             }
    // #ifdef DEBUG_SERIAL
    //             if (response.success())
    //             {
    //                 //response.prettyPrintTo(Serial); //Accusé de réception
    //             }
    //             else
    //             {
    //                 printf("There was a problem when creating the object...\n");
    //             }
    // #endif

    //             sendData(cmd, isMoving(), error);
    //         }
    //     }
    //     else
    //     {
    //         if (SerialUSB.available() > 1)
    //         {
    // #ifdef DEBUG_SERIAL
    //             printf("Something has been detected on the SerialUSB port...\n");
    // #endif

    //             //Get the command/anything that is on the port
    //             // JsonObject &cmd = getCmd();

    //             //if the parsing of what was on port worked then send a positive response, else send error
    //             bool error = false;
    //             if (!cmd.success())
    //             {
    //                 error = true;
    //             }
    //             sendData(cmd, isMoving(), error);
    //         }
    //     }

    //     led_control();
    //     buzzer_state();

    return ret;
}

//--------------------------------------------------------------------------------------------------//
//Serial comm with the json library
//Get the command comminf from the linux side on the SerialUSB port
// string &getCmd()
// {
//     DynamicJsonBuffer jsonBuffer;
//     JsonObject &c = jsonBuffer.parse(SerialUSB);
//     return c;
// }

//This fonction analyse the command and send a response base on it
void sendData(string &request, bool state, bool e)
{
    //     String type = request["type"];
    //     unsigned char d[5];
    //     if (type == "setTime")
    //     {
    //         d[0] = request["time"][0]; // Minute
    //         d[1] = request["time"][1]; // Hour
    //         d[2] = request["time"][2]; // Date
    //         d[3] = request["time"][3]; // Month
    //         d[4] = request["time"][4]; // Year
    //     }
    //     if (type == "BlinkLed" && !e)
    //     {
    //         blinkDuree = request["duree"];
    //         blinkFreq = request["freq"];
    //         greenLedEnabled = request["greenLedEnabled"];
    //         redLedEnabled = request["redLedEnabled"];
    //         blink_enabled = 1;
    //     }
    //     if (type == "SetLedState" && !e)
    //     {
    //         redLedEnabled = request["redLedEnabled"];
    //         greenLedEnabled = request["greenLedEnabled"];
    //         blink_enabled = 0;
    //     }
    //     if (type == "SetBuzzerState" && !e)
    //     {
    //         buzzerEnabled = request["enabled"];
    //     }
    //     if (type == "SetLedsAndBuzzerState" && !e)
    //     {
    //         //Serial.println("Message de type SetLedsAndBuzzerState reçu, message: ");
    //         //request.prettyPrintTo(Serial);
    //         if (request["allAlarmsDevices"] == "ON")
    //         {
    //             //Alume les 2 leds qui flash alterné + le buzzer
    //             blink_enabled = true;
    //             redLedEnabled = true;
    //             greenLedEnabled = true;
    //             buzzerEnabled = true;
    //         }
    //         else if (request["allAlarmsDevices"] == "OFF")
    //         {
    //             //Serial.println("allAlarmsDevices == OFF reçu");
    //             blink_enabled = false;
    //             redLedEnabled = false;
    //             greenLedEnabled = false;
    //             buzzerEnabled = false;
    //             currentLedState = true; //Sera remit à false une fois les leds eteintes
    //         }
    //         else
    //         {
    //             //Put specific states for the devices
    //             blink_enabled = 0;
    //             redLedEnabled = request["redLedEnabled"];
    //             greenLedEnabled = request["greenLedEnabled"];
    //             buzzerEnabled = request["buzzerEnabled"];
    //         }
    //     }

    //     //Create the json object
    //     DynamicJsonBuffer jsonBuffer;
    //     JsonObject &response = jsonBuffer.createObject();

    //     if (type == "dataRequest" && !e)
    //     {
    //         //Get data
    //         if (!state)
    //         {
    //             getData();
    //         }
    //         //Fill the object
    //         response["type"] = "dataResponse";
    //         response["angle"] = angle;
    //         response["isSomeoneThere"] = isSomeoneThere(); //Change that name (?) //Ne devrait plus être utilisé (envoyé par l'arduino automatiquement suite à un changement)
    //         response["isMoving"] = isMoving();             //Change that name (?)
    //         JsonArray &rawDataRef = response.createNestedArray("rawDataRef");
    //         rawDataRef.add(aRawFixe[0]);
    //         rawDataRef.add(aRawFixe[1]);
    //         rawDataRef.add(aRawFixe[2]);
    //         JsonArray &rawDataRel = response.createNestedArray("rawDataRel");
    //         rawDataRel.add(aRawMobile[0]);
    //         rawDataRel.add(aRawMobile[1]);
    //         rawDataRel.add(aRawMobile[2]);

    //         JsonArray &fsr = response.createNestedArray("fsr");
    //         fsr.add(max11611Data[0]);
    //         fsr.add(max11611Data[1]);
    //         fsr.add(max11611Data[2]);
    //         fsr.add(max11611Data[3]);
    //         fsr.add(max11611Data[4]);
    //         fsr.add(max11611Data[5]);
    //         fsr.add(max11611Data[6]);
    //         fsr.add(max11611Data[7]);
    //         fsr.add(max11611Data[8]);

    //         mcp79410.getDateTime(dateTime); //{second, minute, heure, jour de la semaine, date, mois, année}
    //         formatDateTimeString();
    //         response["date"] = date;
    //         response["time"] = time;
    //         //Send response if the object was correctly create
    //         if (response.success())
    //         {
    //             response.printTo(SerialUSB);
    //         }
    // #ifdef DEBUG_SERIAL
    //         if (response.success())
    //         {
    //             //response.prettyPrintTo(Serial);
    //         }
    //         else
    //         {
    //             Serial.println("There was a problem when creating the object...");
    //         }
    // #endif
    //     }
    //     if (type == "setTime")
    //     {
    // #ifdef DEBUG_SERIAL
    //         Serial.print("Test date In: ");
    //         Serial.print("Year: ");
    //         Serial.println(d[4], HEX);
    //         Serial.print("Month: ");
    //         Serial.println(d[3], HEX);
    //         Serial.print("Date: ");
    //         Serial.println(d[2], HEX);
    //         Serial.print("Hours: ");
    //         Serial.println(d[1], HEX);
    //         Serial.print("Minutes: ");
    //         Serial.println(d[0], HEX);
    // #endif

    //         mcp79410.setDateTime(d);
    //         mcp79410.getDateTime(dateTime); //{second, minute, heure, jour de la semaine, date, mois, année}
    //         formatDateTimeString();
    //     }
    //     else if (type == "dataRequest" && e)
    //     {
    //         //Error
    //         response["type"] = "error";
    //         response["issue"] = "Parsing";
    //         response["to do"] = "ask again";
    //         Serial.println("No alert");
    //     }

    //     if (type == "periphStatusReq" && !e)
    //     {
    //         Serial.println("Receive a message of type 'periphStatusReq'");
    //         response["type"] = "periphStatus";

    //         //Accelerometer initialisation
    //         if (!imuFixe.testConnection())
    //         {
    //             response["IMUFixe"] = "false";
    //         }
    //         else
    //         {
    //             response["IMUFixe"] = "true";
    //         }

    //         if (!imuMobile.testConnection())
    //         {
    //             response["IMUMobile"] = "false";
    //         }
    //         else
    //         {
    //             response["IMUMobile"] = "true";
    //         }

    //         response["MAX11611"] = "true"; //Hardcodé a fin de test

    //         //Send response if the object was correctly create
    //         if (response.success())
    //         {
    //             response.printTo(SerialUSB);
    //             //Serial.println("Message has been sent by serial");
    //         }
    // #ifdef DEBUG_SERIAL
    //         if (response.success())
    //         {
    //             //response.prettyPrintTo(Serial);
    //         }
    //         else
    //         {
    //             Serial.println("There was a problem when creating the object...");
    //         }
    // #endif
    //     }
    //     if (type == "captForceReq" && !e)
    //     {
    //         response["type"] = "captForceValues";
    //         response["capt1"] = max11611Data[0];
    //         response["capt2"] = max11611Data[1];
    //         response["capt3"] = max11611Data[2];
    //         response["capt4"] = max11611Data[3];
    //         response["capt5"] = max11611Data[4];
    //         response["capt6"] = max11611Data[5];
    //         response["capt7"] = max11611Data[6];
    //         response["capt8"] = max11611Data[7];
    //         response["capt9"] = max11611Data[8];

    //         //Send response if the object was correctly create
    //         if (response.success())
    //         {
    //             response.printTo(SerialUSB);
    //             //Serial.println("Message has been sent by serial");
    //         }
    //     }
    //     if (type == "calibrationReq" && !e)
    //     {
    //         calibrationProcess(imuFixe, 0);
    //         delay(50);
    //         calibrationProcess(imuMobile, 0);
    //         Serial.println("Calibration des capteurs effectuée.");
    //     }
}

//---------------------------------------------------------------------------------------
// FONCTION POUR RETOURNER LES DATAS DES CAPTEURS
// Date and time, angle (centrales inertielles), pression au coussin (capteurs force)
//---------------------------------------------------------------------------------------

void getData()
{
    // Data: Date and time
    mcp79410.getDateTime(dateTime);

    // Data: Angle (centrales intertielles mobile/fixe)
    getMPUAccData(imuMobile, aRawMobile, aRealMobile);
    getMPUypr(&pitchMobile, &rollMobile, aRealMobile);
    getMPUAccData(imuFixe, aRawFixe, aRealFixe);
    getMPUypr(&pitchFixe, &rollFixe, aRealFixe);
    getAngle();

    // Data: Capteur de force
    max11611.getData(sensorMatrix.sensorCount, max11611Data);
    for (int i = 0; i < sensorMatrix.sensorCount; i++)
    {
        sensorMatrix.SetAnalogData(max11611Data[i], i);
    }
    sensorMatrix.GetForceSensorData(sensorMatrix);

    if (affichageSerial)
    {
        //printStuff();
    }
}

void formatDateTimeString()
{
    string date;
    string mytime;

    string year, month, day, hour, minute, second;

    year = "20";
    if (dateTime[6] < 10)
    {
        year += "0";
    }
    year += std::to_string(int(dateTime[6]));
    month = std::to_string(int(dateTime[5]));
    if (dateTime[5] < 10)
    {
        month = "0" + month;
    }
    day = std::to_string(int(dateTime[4]));
    if (dateTime[4] < 10)
    {
        day = "0" + day;
    }

    hour = std::to_string(int(dateTime[2]));
    if (dateTime[2] < 10)
    {
        hour = "0" + hour;
    }
    minute = std::to_string(int(dateTime[1]));
    if (dateTime[1] < 10)
    {
        minute = "0" + minute;
    }
    second = std::to_string(int(dateTime[0]));
    if (dateTime[0] < 10)
    {
        second = "0" + second;
    }

    date = year + "-" + month + "-" + day;
    mytime = hour + ":" + minute + ":" + second;

#ifdef DEBUG_SERIAL
    printf("%s", date.c_str());
    printf(" - ");
    printf("%s\n", mytime.c_str());
#endif
}
