#include "ChairManager.h"
#include <chrono>
#include "NetworkManager.h"
#include "SysTime.h"


ChairManager::ChairManager(MosquittoBroker *mosquittoBroker, DeviceManager *deviceManager)
    : _mosquittoBroker(mosquittoBroker), _deviceManager(deviceManager)
{
}

ChairManager::~ChairManager()
{
}

void ChairManager::displaySensorData(SensorData sensorData) {
    printf("TimeStamp: %ld\n", sensorData.time);
    printf("MobileIMU: %14s", (sensorData.mIMUConnected) ? "\033[32mConnected\033[0m" : "\033[31mNot Connected\033[0m");
    if (sensorData.mIMUConnected)
    {
        printf("\tax: %5f, \tay: %5f, \taz: %5f\n", sensorData.mIMUAccX, sensorData.mIMUAccY, sensorData.mIMUAccZ);
        printf("%20s\tgx: %5f, \tgy: %5f, \tgz: %5f", "", sensorData.mIMUGyroX, sensorData.mIMUGyroY, sensorData.mIMUGyroZ);
    }
    printf("\n");

    printf(" FixedIMU: %14s", (sensorData.fIMUConnected) ? "\033[32mConnected\033[0m" : "\033[31mNot Connected\033[0m");
    if (sensorData.fIMUConnected)
    {
        printf("\tax: %5f, \tay: %5f, \taz: %5f\n", sensorData.fIMUAccX, sensorData.fIMUAccY, sensorData.fIMUAccZ);
        printf("%20s\tgx: %5f, \tgy: %5f, \tgz: %5f", "", sensorData.fIMUGyroX, sensorData.fIMUGyroY, sensorData.fIMUGyroZ);
    }
    printf("\n");

    printf("      ToF: %14s", (sensorData.tofConnected) ? "\033[32mConnected\033[0m" : "\033[31mNot Connected\033[0m");
    if (sensorData.tofConnected)
    {
        printf("\trange: %d", sensorData.tofRange);
    }
    printf("\n");

    printf("     Flow: %14s", (sensorData.flowConnected) ? "\033[32mConnected\033[0m" : "\033[31mNot Connected\033[0m");
    if (sensorData.flowConnected)
    {
        printf("\tx: %7d, \ty: %7d", sensorData.flowTravelX, sensorData.flowTravelY);
    }
    printf("\n");

    printf("    Alarm: %14s", (sensorData.alarmConnected) ? "\033[32mConnected\033[0m" : "\033[31mNot Connected\033[0m");
    if (sensorData.alarmConnected)
    {
        printf("\tRed: %s, Green: %s, Motor: %s, Button: %s", sensorData.alarmRedLedOn ? "true" : "false", sensorData.alarmGreenLedOn ? "true" : "false", sensorData.alarmDCMotorOn ? "true" : "false", sensorData.alarmButtonPressed ? "true" : "false");
    }
    printf("\n");

    printf("      Mat: %14s", (sensorData.matConnected) ? "\033[32mConnected\033[0m" : "\033[31mNot Connected\033[0m");
    if (sensorData.matConnected)
    {
        printf("\t[%6d %6d %6d]\n%24s[%6d %6d %6d]\n%24s[%6d %6d %6d]\n", sensorData.matData[6], sensorData.matData[3], sensorData.matData[0], "", sensorData.matData[7], sensorData.matData[4], sensorData.matData[1], "", sensorData.matData[8], sensorData.matData[5], sensorData.matData[2]);
    }
    printf("\n");

    //Pas cute mais efface les 9 dernieres lignes
    //printf("\r\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K%s%s%s", (sensorData.matConnected) ? "\033[A\33[2K\033[A\33[2K\033[A\33[2K" : "", (sensorData.mIMUConnected) ? "\033[A\33[2K" : "", (sensorData.fIMUConnected) ? "\033[A\33[2K" : "");
}

void ChairManager::displayChairState(ChairState chairState) {
    printf("TimeStamp: %ld\n", chairState.time);

    printf("       Angle: %6s\t%ld", angleFSM.getCurrentStateName(), angleFSM.getElapsedTime());
    printf("\tFixed: %03d, \tMobile: %03d, \tSeat:%d\n", chairState.fIMUAngle, chairState.mIMUAngle, chairState.seatAngle);

    printf("        Flow: %6s\t%ld", travelFSM.getCurrentStateName(), travelFSM.getElapsedTime());
    printf("\tDistance: %5d\n", (int)chairState.lastDistance);

    printf("Notification: %6s\t%ld\n", notificationFSM.getCurrentStateName(), notificationFSM.getElapsed());

    printf("     Seating: %6s\t%ld", seatingFSM.getCurrentStateName(), seatingFSM.getElapsedTime());
    if (chairState.isSeated)
    {
        printf("\t[(%4.2f, %4.2f)  (%4.2f, %4.2f)]\n%33s\t[       (%4.2f, %4.2f)       ]\n%33s\t[(%4.2f, %4.2f)  (%4.2f, %4.2f)]",chairState.centerOfGravityPerQuadrant[3].x,chairState.centerOfGravityPerQuadrant[3].y,chairState.centerOfGravityPerQuadrant[0].x,chairState.centerOfGravityPerQuadrant[0].y,"",chairState.centerOfGravity.x,chairState.centerOfGravity.y,"",chairState.centerOfGravityPerQuadrant[2].x,chairState.centerOfGravityPerQuadrant[2].y,chairState.centerOfGravityPerQuadrant[1].x,chairState.centerOfGravityPerQuadrant[1].y);
    }       
    printf("\n");

}

float ChairManager::calculatemIMUAngle(SensorData sd) {
    float y = -sd.mIMUAccX;
    float x = sqrtf(sd.mIMUAccY*sd.mIMUAccY + sd.mIMUAccZ*sd.mIMUAccZ);
    float angleRad = atan2f(y, x);
    float angleDeg = angleRad*(180.0f/M_PI);
    /*   
    angleDeg -= mIMUOffset;
    if(angleDeg >= 0) {
        angleDeg = fmod(angleDeg, 90.0f);
    } else {
        angleDeg = 90.0f - fmod(abs(angleDeg), 90.0f);
    }
    */
    return static_cast<float>(angleDeg);    
}

float ChairManager::calculatefIMUAngle(SensorData sd) {
    /*
        https://www.dfrobot.com/wiki/index.php/How_to_Use_a_Three-Axis_Accelerometer_for_Tilt_Sensing
    */
    float y = -sd.fIMUAccX;
    float x = sqrtf(sd.fIMUAccY*sd.fIMUAccY + sd.fIMUAccZ*sd.fIMUAccZ);
    float angleRad = atan2f(y, x);
    float angleDeg = angleRad*(180.0f/M_PI);

    return static_cast<float>(angleDeg);    
}

float ChairManager::calculateSeatAngle(ChairState cs) {
    float a = chairState.fIMUAngle - chairState.mIMUAngle;
    seatAngleUncorrected = a;
    a -= mIMUOffset;
    return -a;
}

void ChairManager::setAngleOffset(int fixedOffset, int mobileOffset) {
    fIMUOffset += fixedOffset;
    mIMUOffset = mobileOffset;
}

Coord_t ChairManager::calculateCenterOfGravity(SensorData sd) {
    Coord_t position = {0.0f,0.0f};
    float sum = 0;

    for(int i = 0; i < 9; i++) {
        position.x += sd.matData[i] * POSITION_LOOKUP[i].x;
        position.y += sd.matData[i] * POSITION_LOOKUP[i].y;
        sum += sd.matData[i];
    }
    if(sum == 0) {
        position.x = 0.0;
        position.y = 0.0;
    } else {
        position.x = position.x/sum;
        position.y = position.y/sum;
    }

    return position;
}
 
void ChairManager::calculateCenterOfGravityPerQuadrant(SensorData sd, Coord_t *quad) {

    int arr[4][4] = {{0,1,3,4}, {1,2,4,5}, {7,4,8,5}, {6,3,7,4}};
    float sum = 0;

    for(int j = 0; j < 4; j++) {
        quad[j] = {0.0f,0.0f};
        sum = 0;
        for(int i = 0; i < 4; i++) {

            quad[j].x += sd.matData[arr[j][i]] * POSITION_LOOKUP[arr[j][i]].x;
            quad[j].y += sd.matData[arr[j][i]] * POSITION_LOOKUP[arr[j][i]].y;
            sum += sd.matData[arr[j][i]];
        }
        if(sum == 0) {
            quad[j].x = 0.0;
            quad[j].y = 0.0;
        } else {
            quad[j].x = quad[j].x/sum;
            quad[j].y = quad[j].y/sum;
        }
    }
}

uint32_t ChairManager::calculateDistance(SensorData sd) {

    if(sd.tofRange > 8190)  {
        printf("\nDetected range in ChairManager is higher than 8190; surface is out of reach of sensor.\n");
         return 0.0;
    }

    // The camera has a field of view of 42 degrees or 0.733038285 rad.
    // The sensor is 30 pixels by 30 pixels
    // We assume that the sensor is perpendicular to the ground.
    // Arc Length = fov_in_rad * height_from_the_ground
    const double numberOfPixels = 30.0f;
    const double fieldOfView = 0.733038285f;
    double travelInPixels = sqrt(static_cast<double>(((sd.flowTravelX * sd.flowTravelX) + (sd.flowTravelY * sd.flowTravelY))));
    return static_cast<uint32_t>(((travelInPixels * fieldOfView * sd.tofRange) / numberOfPixels)/10.0);
}

void ChairManager::UpdateDevices()
{
    _deviceManager->Update();
    sensorData = _deviceManager->getSensorData();
    
    
    //DL Modifierd Nov 26 2020, Update alarm state according to messages
    //from the FSM (python)
    if (_mosquittoBroker->getAlarmEnabled())
    {
        if (_mosquittoBroker->getAlarmAlternatingLedBlink()) {
            _deviceManager->getAlarm()->TurnOnAlternatingBlinkAlarmThread();
        }
  
        if (_mosquittoBroker->getAlarmGreenLedBlink()) {
            _deviceManager->getAlarm()->TurnOnBlinkGreenAlarmThread();
        }
        
        if (_mosquittoBroker->getAlarmRedLedOn()) {
            _deviceManager->getAlarm()->TurnOnRedLed();
        }
        else {
            _deviceManager->getAlarm()->TurnOffRedLed();
        }
        
        if (_mosquittoBroker->getAlarmGreenLedOn()) {
            _deviceManager->getAlarm()->TurnOnGreenLed();
        }
        else {
            _deviceManager->getAlarm()->TurnOffGreenLed();
        }
        
        if (_mosquittoBroker->getAlarmMotorOn()) {
            _deviceManager->getAlarm()->TurnOnDCMotor();
        }
        else {
            _deviceManager->getAlarm()->TurnOffDCMotor();
        }
    }
    else {
        _deviceManager->getAlarm()->TurnOffAlarm();
    }
    
    

#if 0
    chairState.time = sensorData.time;
    chairState.isSeated = verifyIfUserIsSeated();
    chairState.centerOfGravity = calculateCenterOfGravity(sensorData);
    calculateCenterOfGravityPerQuadrant(sensorData, chairState.centerOfGravityPerQuadrant);

    chairState.lastDistance = calculateDistance(sensorData);

    chairState.mIMUAngle = calculatemIMUAngle(sensorData);
    chairState.fIMUAngle = calculatefIMUAngle(sensorData);
    chairState.seatAngle = calculateSeatAngle(chairState);
    chairState.button = sensorData.alarmButtonPressed;

    bool blinkEnable, vibrationEnable, enabled;
    int snoozeTime;

    _mosquittoBroker->getNotificationSettings(&blinkEnable, &vibrationEnable, &snoozeTime, &enabled);



    if(notificationFSM.getSnoozeTime() != snoozeTime) {
        notificationFSM.setSnoozeTime(snoozeTime);
    }



    seatingFSM.updateState(chairState);
    travelFSM.updateState(chairState);
    angleFSM.updateState(chairState);
    notificationFSM.updateState(chairState, angleFSM, seatingFSM, travelFSM, enabled);



    if (notificationFSM.getCurrentState() == static_cast<int>(NotificationState::WAITING_FOR_TILT)) {
        // Clignotement alterné + Moteur
        if(lastNotificationState != notificationFSM.getCurrentState()) {
            //On vient de changer d'état
            lastNotificationState = notificationFSM.getCurrentState();
            _deviceManager->getAlarm()->TurnOffAlarm();
        }
        else {
            if(blinkEnable) {
                _deviceManager->getAlarm()->TurnOnAlternatingBlinkAlarmThread();
            }

            if(vibrationEnable) {
                _deviceManager->getAlarm()->TurnOnDCMotor();
            } else {
                _deviceManager->getAlarm()->TurnOffDCMotor();
            }
        }

    } else if (notificationFSM.getCurrentState() == static_cast<int>(NotificationState::IN_TILT)) {
        if(lastNotificationState != notificationFSM.getCurrentState()) {
            //On vient de changer d'état
            lastNotificationState = notificationFSM.getCurrentState();
            _deviceManager->getAlarm()->TurnOffAlarm();
        }
        else {
            if(chairState.seatAngle  < angleFSM.getTargetAngle() - 2) {
                //Sous l'angle voulu, Allume rouge
                if(blinkEnable) {
                    _deviceManager->getAlarm()->TurnOnRedLed();
                    _deviceManager->getAlarm()->TurnOffGreenLed();
                    _deviceManager->getAlarm()->TurnOffDCMotor();
                }
            } else if(chairState.seatAngle  > angleFSM.getTargetAngle() + 2) {
                //Au dessus de l'angle voulu, Allume vert
                if(blinkEnable) {
                    _deviceManager->getAlarm()->TurnOffRedLed();
                    _deviceManager->getAlarm()->TurnOnGreenLed();
                    _deviceManager->getAlarm()->TurnOffDCMotor();
                }
            } else {
                // +/- deux de l'angle voulu allume rouge et vert
                if(blinkEnable) {
                    _deviceManager->getAlarm()->TurnOnRedLed();
                    _deviceManager->getAlarm()->TurnOnGreenLed();
                    _deviceManager->getAlarm()->TurnOffDCMotor();
                }
            }
        }
    } else if (notificationFSM.getCurrentState() == static_cast<int>(NotificationState::TILT_DURATION_OK)) {
        //Durée correcte, led verte clignote + Moteur
        if(lastNotificationState != notificationFSM.getCurrentState()) {
            //On vient de changer d'état
            lastNotificationState = notificationFSM.getCurrentState();
            _deviceManager->getAlarm()->TurnOffAlarm();
        }
        else {
            if(blinkEnable) {
                _deviceManager->getAlarm()->TurnOnBlinkGreenAlarmThread();
            }

            if(vibrationEnable) {
                _deviceManager->getAlarm()->TurnOnDCMotor();
            } else {
                _deviceManager->getAlarm()->TurnOffDCMotor();
            }
        }
    } else {
        _deviceManager->getAlarm()->TurnOffAlarm();
    }
    
    
#endif

    _mosquittoBroker->SendSensorsData(sensorData);

#if 0
    _mosquittoBroker->SendChairState(chairState);
    _mosquittoBroker->SendAngleFSM(angleFSM);
    _mosquittoBroker->SendTravelFSM(travelFSM);
    _mosquittoBroker->SendSeatingFSM(seatingFSM);
    _mosquittoBroker->SendNotificationFSM(notificationFSM);
    ReadFromServer();
#endif

    //DL
    //displaySensorData(sensorData);
    //displayChairState(chairState);
    //printf("\r\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K%s%s%s", (sensorData.matConnected) ? "\033[A\33[2K\033[A\33[2K\033[A\33[2K" : "", (sensorData.mIMUConnected) ? "\033[A\33[2K" : "", (sensorData.fIMUConnected) ? "\033[A\33[2K" : "");
    //printf("\r\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A\33[2K\033[A%s", (chairState.isSeated) ? "\033[A\33[2K\033[A\33[2K" : "");
    
}

void ChairManager::ReadFromServer()
{
    if (_mosquittoBroker->CalibPressureMatRequired())
    {
        printf("ChairManager::ReadFromServer(), calib pressure mat enabled.\n");

        _deviceManager->CalibratePressureMat();
        //Reset flag
        _mosquittoBroker->SetCalibPressurMatRequired(false);
    }


    if (_mosquittoBroker->CalibIMURequired()) {
        setAngleOffset(chairState.fIMUAngle, seatAngleUncorrected);
        _mosquittoBroker->SendAngleOffset(mIMUOffset, fIMUOffset);
        _mosquittoBroker->SetCalibIMURequired(false);
    }

    if (_mosquittoBroker->GoalHasChanged()) {
        int frequencyGoal, durationGoal, angleGoal;
        _mosquittoBroker->getGoal(&frequencyGoal,&durationGoal,&angleGoal);
        angleFSM.setParameter(frequencyGoal, durationGoal, angleGoal);
        _mosquittoBroker->SetGoalHasChanged(false);
    }

    if(_mosquittoBroker->offsetChanged()) {
        int mobileIMUOffset, fixedIMUOffset;
        _mosquittoBroker->getOffsets(&mobileIMUOffset, &fixedIMUOffset);
        setAngleOffset(fixedIMUOffset, mobileIMUOffset);
        _mosquittoBroker->setOffsetChanged(false);
    }
}

void ChairManager::CheckNotification()
{
    return;
}

