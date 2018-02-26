//---------------------------------------------------------------------------------------
// TITRE
// Description
//---------------------------------------------------------------------------------------

//Include : Drivers
#include "PCA9536.h" //Arduino 4 programmable 4-channel digital i-o

//Include : Modules
#include "init.h"         //variables and modules initialisation
#include "notif_module.h" //variables and modules initialisation
#include "accel_module.h" //variables and modules initialisation
#include "force_module.h" //variables and modules initialisation
#include "program.h"      //variables and modules initialisation
#include "test.h"         //variables and modules initialisation

//External functions and variables
//Variables
extern PCA9536 pca9536; //Construct a new PCA9536 instance
// extern int blinkDuree;
// extern double blinkFreq;
// extern bool blink_enabled;
// extern int nCompteur;
// extern bool redLedEnabled;
// extern bool greenLedEnabled;
// extern bool buzzerEnabled;
// extern bool currentLedState;
extern bool bIsBtnPushed;

//Notification module variables
int blinkDuree = 0;
double blinkFreq = 0;
bool blink_enabled = false;
int nCompteur = 0;
bool redLedEnabled = false;
bool greenLedEnabled = false;
bool buzzerEnabled = false;
bool currentLedState = false;

//---------------------------------------------------------------------------------------
// FONCTIONS DE CONTROLE DU MODULE DE NOTIFICATION
// LightON-OFF, BuzzerON-OFF, isPushed
//---------------------------------------------------------------------------------------

//PCA9536 function for the Red/Green LEDs
void LightOn(pin_t pin)
{
    pca9536.setState(pin, IO_HIGH);
}
void LightOff(pin_t pin)
{
    pca9536.setState(pin, IO_LOW);
}

//PCA9536 function for the DC motor (buzzer)
void StartBuzzer()
{
    pca9536.setState(DC_MOTOR, IO_HIGH);
}
void StopBuzzer()
{
    pca9536.setState(DC_MOTOR, IO_LOW);
}

//PCA9536 function for push-button state
uint8_t isPushed()
{
    uint8_t res = !pca9536.getState(PUSH_BUTTON);
    return res;
}

//---------------------------------------------------------------------------------------
// FONCTIONS DE BLINK DES LEDS
// Blink de LED verte, rouge
//---------------------------------------------------------------------------------------

void led_control()
{

    //LED Blinking start
    if (blink_enabled)
    {

        if (redLedEnabled || greenLedEnabled)
        {
            nCompteur += 1;

            //Green LED blinking
            if (!greenLedEnabled)
            {
                LightOff(GREEN_LED);

                if (currentLedState == false)
                {
                    LightOn(RED_LED);
                    currentLedState = true;
                }
                else
                {
                    LightOff(RED_LED);
                    currentLedState = false;
                }
            }

            //Red LED blinking
            else if (!redLedEnabled)
            {
                LightOff(RED_LED);

                if (currentLedState == false)
                {
                    LightOn(GREEN_LED);
                    currentLedState = true;
                }
                else
                {
                    LightOff(GREEN_LED);
                    currentLedState = false;
                }
            }

            //Both LED blinking
            else
            {
                if (currentLedState == false)
                {
                    LightOff(RED_LED);
                    LightOn(GREEN_LED);
                    currentLedState = true;
                }
                else
                {
                    LightOff(GREEN_LED);
                    LightOn(RED_LED);
                    currentLedState = false;
                }
            }
        }

        delay(blinkFreq * 1000);

        //LED blinking end
        if (nCompteur >= (blinkFreq * blinkDuree))
        {

            printf("NoCompteur: ");
            printf("%i\n", nCompteur);
            printf("blinkFreq: ");
            printf("%f\n", blinkFreq);
            printf("blinkDuree: ");
            printf("%i\n", blinkDuree);
            printf("LED blink end\n");
            LightOff(GREEN_LED);
            LightOff(RED_LED);
            currentLedState = false;
            greenLedEnabled = false;
            redLedEnabled = false;
            blink_enabled = false;
            nCompteur = 0; //Reset le compteur
        }
    }

    //No LED blinking start
    else
    {

        //Green LED ON
        if (greenLedEnabled && !redLedEnabled)
        {
            LightOn(GREEN_LED);
            LightOff(RED_LED);
            currentLedState = true;
        }
        else if (!greenLedEnabled && redLedEnabled)
        {
            LightOn(RED_LED);
            LightOff(GREEN_LED);
            currentLedState = true;
        }
        else if (!greenLedEnabled && !redLedEnabled && currentLedState == true)
        {
            LightOff(GREEN_LED);
            LightOff(RED_LED);
            currentLedState = false;
        }
    }
}

//---------------------------------------------------------------------------------------
// FONCTIONS DE CONTROLE DU MOTEUR DC
//
//---------------------------------------------------------------------------------------

void buzzer_state()
{
    // Etat du bouton et du buzzer
    bIsBtnPushed = isPushed(); //Prend l'état actuel du bouton
    //Gère le buzzer, peut aussi être arreté par l'appuie du bouton
    if (buzzerEnabled && !bIsBtnPushed)
    {
        StartBuzzer();
    }
    else if (bIsBtnPushed)
    {
        buzzerEnabled = false;
        StopBuzzer();
    }
}
