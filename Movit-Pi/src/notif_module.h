#ifndef _NOTIF_MODULE_H_
#define _NOTIF_MODULE_H_

#include "PCA9536.h"  //Arduino 4 programmable 4-channel digital i-o



void LightOn(pin_t pin);
void LightOff(pin_t pin);
void StartBuzzer();
void StopBuzzer();
uint8_t isPushed();
void led_control();
void light_state() ;
void buzzer_state();

#endif /* _NOTIF_MODULE_H_ */
