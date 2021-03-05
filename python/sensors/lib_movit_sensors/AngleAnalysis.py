import json
import threading
import numpy as np
# import matplotlib.pyplot as plt
import math
from scipy import signal
from datetime import datetime
import time
import os
from enum import Enum, unique


import lib_movit_sensors.rmoutliers as rmo

@unique
class AngleAnalysisState(Enum):
    INIT = 0
    AA_ERROR = 1
    CHECK_CALIBRATION_VALID = 2
    CALIBRATION_WAIT_ZERO_TRIG = 3
    CALIBRATION_RUNNING_ZERO = 4
    CALIBRATION_WAIT_INCLINED_TRIG = 5
    CALIBRATION_RUNNING_INCLINED = 6
    CALIBRATION_WAIT_ROT_WORLD_CALC = 7
    CALIBRATION_DONE = 8
    CALIBRATION_TODO = 9


class paramIMU:
    def __init__(self):
        self.Facc_Zero = [0, 0, 0]
        self.Fgyr_Zero = [0, 0, 0]
        self.nF_Zero = 1

        self.Macc_Zero = [0, 0, 0]
        self.Mgyr_Zero = [0, 0, 0]
        self.nM_Zero = 1

        self.Facc_Inclined = [0, 0, 0]
        self.Fgyr_Inclined = [0, 0, 0]
        self.nF_Inclined = 1

        self.Macc_Inclined = [0, 0, 0]
        self.Mgyr_Inclined = [0, 0, 0]
        self.nM_Inclined = 1

        pass

    def storeParam(self, **kwargs):
        for kwarg in kwargs:
            self.switcher(kwarg, kwargs.get(kwarg))
        pass

    def switcher(self, arg, values):
        switcher = {
            "fixedIMU_Zero": self.setF_Zero,
            "mobileIMU_Zero": self.setM_Zero,
            "fixedIMU_Inclined": self.setF_Inclined,
            "mobileIMU_Inclined": self.setM_Inclined,
        }
        func = switcher.get(arg, [])
        func(values)

    def setF_Zero(self, fixedIMU_Zero):
        self.Facc_Zero = fixedIMU_Zero[:, 0:3].T
        self.Fgyr_Zero = fixedIMU_Zero[:, 3:6].T
        self.nF_Zero = self.Facc_Zero.shape[1]

    def setM_Zero(self, mobileIMU_Zero):
        self.Macc_Zero = mobileIMU_Zero[:, 0:3].T
        self.Mgyr_Zero = mobileIMU_Zero[:, 3:6].T
        self.nM_Zero = self.Macc_Zero.shape[1]

    def setF_Inclined(self, fixedIMU_Inclined):
        self.Facc_Inclined = fixedIMU_Inclined[:, 0:3].T
        self.Fgyr_Inclined = fixedIMU_Inclined[:, 3:6].T
        self.nF_Inclined = self.Facc_Inclined.shape[1]

    def setM_Inclined(self, mobileIMU_Inclined):
        self.Macc_Inclined = mobileIMU_Inclined[:, 0:3].T
        self.Mgyr_Inclined = mobileIMU_Inclined[:, 3:6].T
        self.nM_Inclined = self.Macc_Inclined.shape[1]


class RotWorld(paramIMU):
    def __init__(self):
        super().__init__()
        self.Cterrain = []
        self.Facc_offset = []
        self.Fgyr_offset = []
        self.Macc_offset = []
        self.Mgyr_offset = []

        self.Cfixe = []
        self.Cmobile = []

        self.isRotWorld = False

        self.loadFromJson()

    def computeRotWorld(self):
        ##
        Facc_calibNoOut, facc_out = rmo.rmoutliers_percentiles(self.Facc_Zero, 1.645, 1.645)
        Macc_calibNoOut, macc_out = rmo.rmoutliers_percentiles(self.Macc_Zero, 1.645, 1.645)
        Fgyr_calibNoOut, fgyr_out = rmo.rmoutliers_percentiles(self.Fgyr_Zero, 1.645, 1.645)
        Mgyr_calibNoOut, mgyr_out = rmo.rmoutliers_percentiles(self.Mgyr_Zero, 1.645, 1.645)

        # Localiser axe gravit� (approx)
        Facc_mean = np.mean(Facc_calibNoOut, axis=1)
        Macc_mean = np.mean(Macc_calibNoOut, axis=1)

        def normVec(V):
            v_norm = V / np.linalg.norm(V) #np.sqrt(np.dot(V,V))
            return v_norm

        Fgref = normVec(Facc_mean) #Facc_mean / np.dot(Facc_mean,Facc_mean)
        Mgref = normVec(Macc_mean) #Macc_mean / np.dot(Macc_mean,Macc_mean)

        # KL - 06nov: ajustement ax gravit�      
        Facc_offset = np.mean(Facc_calibNoOut, axis=1) - Fgref
        Macc_offset = np.mean(Macc_calibNoOut, axis=1) - Mgref
        Fgyr_offset = np.mean(Fgyr_calibNoOut, axis=1)
        Mgyr_offset = np.mean(Mgyr_calibNoOut, axis=1)

        # Remarque 1: ***Consid�rer le filtrage?***
        # Remarque 2: En calibrant comme �a, on assume que c'est du bruit de
        # capteur et que ce n'est pas une erreur d'alignement interne... mais
        # bon... on va tenter de palier � tout cela en faisant une certaine
        # it�ration...

        Macc_offset2 = [0, 0, 0]
        Mgyr_offset2 = [0, 0, 0]

        ##
        np.cosd = lambda x: np.cos(np.deg2rad(x))
        np.sind = lambda x: np.sin(np.deg2rad(x))

        ## création de Cterrain
        Cterrain = []
        angTerrain = [0, 0, 0]
        CterrainX = np.matrix([[1, 0, 0],
                                [0, np.cosd(angTerrain[0]), -np.sind(angTerrain[0])],
                                [0, np.sind(angTerrain[0]), np.cosd(angTerrain[0])]])
        CterrainY = np.matrix([[np.cosd(angTerrain[1]), 0, np.sind(angTerrain[1])],
                                [0, 1, 0],
                                [-np.sind(angTerrain[1]), 0, np.cosd(angTerrain[1])]])
        CterrainZ = np.matrix([[np.cosd(angTerrain[0]), -np.sind(angTerrain[0]), 0],
                                [np.sind(angTerrain[0]), np.cosd(angTerrain[0]), 0],
                                [0, 0, 1]])

        Cterrain = CterrainX * CterrainY * CterrainZ

        ## search offsets
        Cmobile = []

        for icalib in range(20):
            Macc_offset = Macc_offset + Macc_offset2
            Mgyr_offset = Mgyr_offset + Mgyr_offset2

            ## Cr�ation de vecteurs de l'essai
            Facc_Zero = (Cterrain * self.Facc_Zero).T - Facc_offset
            Macc_Zero = (Cterrain * self.Macc_Zero).T - Macc_offset

            # Fgyr_Zero = self.Fgyr_Zero.T - Fgyr_offset
            # Mgyr_Zero = self.Mgyr_Zero.T - Mgyr_offset

            # Facc_Inclined = (Cterrain * self.Facc_Inclined).T - Facc_offset
            Macc_Inclined = (Cterrain * self.Macc_Inclined).T - Macc_offset

            # Fgyr_Inclined = self.Fgyr_Inclined.T - Fgyr_offset
            Mgyr_Inclined = self.Mgyr_Inclined.T - Mgyr_offset

            ## CALIBRATION POSITIONNEMENT STATIQUE

            # (1) Mesure statique

            # Facc_stat = np.mean(Facc_Zero, axis=0)
            Macc_stat = np.mean(Macc_Zero, axis=0)

            # (2) Inclinaison initiale des IMUs:
            # ** IMU mobile **
            vecG = np.array([0,0,1])
            Macc_stat_norm = normVec(np.array(Macc_stat)[0])
            u = normVec(np.cross(vecG,Macc_stat_norm))

            def angRot2Vec(vector_1,vector_2):
                unit_vector_1 = vector_1 / np.linalg.norm(vector_1)
                unit_vector_2 = vector_2 / np.linalg.norm(vector_2)
                dot_product = np.dot(unit_vector_1, unit_vector_2)
                angle = np.arccos(dot_product)
                return angle
            angRot = angRot2Vec(vecG,Macc_stat_norm)

            c=np.cos(angRot)
            s=np.sin(angRot)
            
            C1_mobile = np.linalg.inv( np.matrix ([[u[0]**2 + (1-u[0]**2)*c, u[0]*u[1]*(1-c)-u[2]*s, u[0]*u[2]*(1-c)+u[1]*s],
                                                    [u[0]*u[1]*(1-c)+u[2]*s, u[1]**2 + (1-u[1]**2)*c, u[1]*u[2]*(1-c)-u[0]*s],
                                                    [u[0]*u[2]*(1-c)-u[1]*s, u[1]*u[2]*(1-c)+u[0]*s, u[2]**2 + (1-u[2]**2)*c]])
                                    )

            # Correction capteurs dans IMU mobile
            # Macc_cor = (C1_mobile*Macc.T).T
            # Mgyr_cor = (C1_mobile*Mgyr.T).T

            Macc_Zero_cor = (C1_mobile * Macc_Zero.T).T
            # Mgyr_Zero_cor = (C1_mobile * Mgyr_Zero.T).T

            Macc_Inclined_cor = (C1_mobile * Macc_Inclined.T).T
            Mgyr_Inclined_cor = (C1_mobile * Mgyr_Inclined.T).T

            # (3) Axe de rotation (IMU mobile) pendant la bascule

            # Remarque: Trouver de fa�on autonome la p�riode de bascule...
            Macc_Zero_Dir = np.mean(Macc_Zero_cor, axis=0)
            Macc_Inclined_Dir = np.mean(Macc_Inclined_cor, axis=0)
            def vecBasc(Macc_Zero_Dir,Macc_Inclined_Dir):
                vector_1 = np.array([Macc_Zero_Dir[0,0],Macc_Zero_Dir[0,1],Macc_Zero_Dir[0,2]])
                vector_2 = np.array([Macc_Inclined_Dir[0,0],Macc_Inclined_Dir[0,1],Macc_Inclined_Dir[0,2]])

                vecRot = np.cross(vector_1,vector_2)
                unit_vecRot = vecRot / np.linalg.norm(vecRot)
                return np.matrix(unit_vecRot)

            vecRot = vecBasc(Macc_Zero_Dir,Macc_Inclined_Dir)

            # La bonne nouvelle c'est que la composante en z est presque nulle...
            # donc l'alignement avec acc�l�ro tiens la route... la petite
            # composante est soit un r�siduel de l'alignement vs offset initial,
            # soit la vibration carr�ment... pour l'instant, je mets de c�t�.

            # Trouvons l'angle de rotation autour de z qui permettra d'avoir une
            # bascule purement en "y".
            ang_Rotz = math.atan2(vecRot[0,0], vecRot[0,1])
            Mat_Rotz = np.matrix([[np.cos(ang_Rotz), -np.sin(ang_Rotz), 0],
                                  [np.sin(ang_Rotz), np.cos(ang_Rotz), 0],
                                  [0, 0, 1]])

            # v�rification axe bascule:
            # test = Mat_Rotz * vecRot.T
            # print(test)
            # OK... pas mal �a... reste la petite composante en z mais...

            # Matrice alignement principale (mobile)
            Cmobile = Mat_Rotz * C1_mobile

            # On va essayer d'optimiser un peu en validant pour 3 vecteurs...
            # (i) vecteur statique initial, normalis�...
            vecIni = np.mean(Macc_Zero, axis=0)
            # vecIni = vecIni / sqrt(vecIni*vecIni')
            vecIni_al = Cmobile * vecIni.T
            # (iii) Vecteur de bascule
            vecBascule = vecRot
            vecBascule_al = Cmobile * vecBascule.T

            # Avecteur devrait s'approcher de Adesire
            Avecteur = np.concatenate((vecIni_al.T, vecBascule_al.T))
            Adesire = np.matrix([[0, 0, 1], [0, 1, 0]])

            Macc_offset2 = Avecteur[0, :] - Adesire[0, :]
            Mgyr_offset2 = Avecteur[1, :] - Adesire[1, :]

        ## (4) Optimisation du d�couplage correction angulaire vs offset pour le
        #    IMU fixe
        Fgyr_offset2 = [0, 0, 0]
        Facc_offset2 = [0, 0, 0]

        # # ORIGINAL
        # Facc_offset = np.mean(self.Facc_Zero, axis=1) - [0, 0, 1]
        # Fgyr_offset = np.mean(self.Fgyr_Zero, axis=1)

        # Filtre pour gyro
        # [B,A] = butter(3,0.3/0.5,"low") #a optimiser
        w = 0.3 / 0.5  # Normalize the frequency
        b, a = signal.butter(3, w, 'low')
        ntaps = max(len(a), len(b))

        Facc = (Cterrain * self.Facc_Inclined).T - Facc_offset

        signalToFilter = (Cterrain * self.Fgyr_Inclined)
        if signalToFilter.shape[1] > ntaps:
            Fgyr = signal.filtfilt(a, b, signalToFilter).T - Fgyr_offset
        else:
            Fgyr = signalToFilter.T - Fgyr_offset

        # Macc = (Cterrain * self.Macc_Inclined).T - Macc_offset

        # signalToFilter = (Cterrain * self.Mgyr_Inclined)
        # if signalToFilter.shape[1] > ntaps:
        #     Mgyr = signal.filtfilt(a, b, signalToFilter).T - Mgyr_offset
        # else:
        #     Mgyr = signalToFilter.T - Mgyr_offset

        ##
        Cfixe = []
        for j in range(20):
            # Fgyr_offset = Fgyr_offset + Fgyr_offset2
            Facc_offset = Facc_offset + Facc_offset2

            ## Cr�ation de vecteurs de l'essai
            Facc_Zero = (Cterrain * self.Facc_Zero).T - Facc_offset
            # Fgyr_Zero = self.Fgyr_Zero.T - Fgyr_offset

            # Inclinaison IMU fixe
            Facc_stat = np.mean(Facc_Zero, axis=0)

            vecG = np.array([0,0,1])
            Facc_stat_norm = normVec(np.array(Facc_stat)[0])
            u = normVec(np.cross(vecG,Macc_stat_norm))

            def angRot2Vec(vector_1,vector_2):
                unit_vector_1 = vector_1 / np.linalg.norm(vector_1)
                unit_vector_2 = vector_2 / np.linalg.norm(vector_2)
                dot_product = np.dot(unit_vector_1, unit_vector_2)
                angle = np.arccos(dot_product)
                return angle
            angRot = angRot2Vec(vecG,Facc_stat_norm)

            c=np.cos(angRot)
            s=np.sin(angRot)
            
            C1_fixe = np.linalg.inv( np.matrix ([[u[0]**2 + (1-u[0]**2)*c, u[0]*u[1]*(1-c)-u[2]*s, u[0]*u[2]*(1-c)+u[1]*s],
                                                    [u[0]*u[1]*(1-c)+u[2]*s, u[1]**2 + (1-u[1]**2)*c, u[1]*u[2]*(1-c)-u[0]*s],
                                                    [u[0]*u[2]*(1-c)-u[1]*s, u[1]*u[2]*(1-c)+u[0]*s, u[2]**2 + (1-u[2]**2)*c]])
                                    )

            # Correction capteurs dans IMU fixe
            Facc_cor = (C1_fixe * Facc_Zero.T).T
            # Fgyr_cor = (C1_fixe * Fgyr_Zero.T).T

            Facc_offset2 = np.mean(Facc_cor, axis=0) - [0, 0, 1]
            # Fgyr_offset2 = np.mean(Fgyr_cor, axis=0)
            Cfixe = C1_fixe

        self.Cterrain = Cterrain
        self.Facc_offset = Facc_offset
        self.Fgyr_offset = Fgyr_offset
        self.Macc_offset = Macc_offset
        self.Mgyr_offset = Mgyr_offset

        self.Cfixe = Cfixe
        self.Cmobile = Cmobile

        self.isRotWorld = True
        print(self.getRotWorld())
        self.saveToJson()

        self.computeAngle(self.Facc_Zero,
                          self.Fgyr_Zero,
                          self.Macc_Zero,
                          self.Mgyr_Zero)

        self.computeAngle(self.Facc_Inclined,
                          self.Fgyr_Inclined,
                          self.Macc_Inclined,
                          self.Mgyr_Inclined)

    def computeAngle(self, Facc_, Fgyr_, Macc_, Mgyr_):
        # Filtre pour gyro
        # [B,A] = butter(3,0.3/0.5,"low") #a optimiser
        # w = 0.3 / 0.5  # Normalize the frequency
        # b, a = signal.butter(3, w, 'low')
        # ntaps = max(len(a), len(b))

        Facc = np.dot(self.Cterrain, Facc_).T - self.Facc_offset

        # signalToFilter = np.dot(self.Cterrain, Fgyr_)
        # if signalToFilter.shape[1] > ntaps:
        #     Fgyr = signal.filtfilt(a, b, signalToFilter).T - self.Fgyr_offset
        # else:
        #     Fgyr = signalToFilter.T - self.Fgyr_offset

        Macc = np.dot(self.Cterrain, Macc_).T - self.Macc_offset

        # signalToFilter = np.dot(self.Cterrain, Mgyr_)
        # if signalToFilter.shape[1] > ntaps:
        #     Mgyr = signal.filtfilt(a, b, signalToFilter).T - self.Mgyr_offset
        # else:
        #     Mgyr = signalToFilter.T - self.Mgyr_offset

        Facc_cor = np.dot(self.Cfixe, Facc.T).T
        # Fgyr_cor = (self.Cfixe*Fgyr.T).T
        Macc_cor = np.dot(self.Cmobile, Macc.T).T

        # Mgyr_cor = (self.Cmobile*Mgyr.T).T

        def acosd(x):
            return np.degrees(np.arccos(x))

        gMobileStat1 = np.mean(Macc_cor, axis=0)
        gFixeStat1 = np.mean(Facc_cor, axis=0)
        angleSiege = acosd((gMobileStat1 * gFixeStat1.T) / (
                np.sqrt(gMobileStat1 * gMobileStat1.T) * np.sqrt(gFixeStat1 * gFixeStat1.T)))
        print(angleSiege)

        return angleSiege

    def getRotWorld(self):
        result = {
            'name': self.__class__.__name__,
            #     'timestamp': int(self.last_update.timestamp()),
            #     'connected': self.connected(),
            #     'values': self.values.tolist(),
            'offsets': {'Facc': self.Facc_offset.tolist(), 'Fgyr': self.Fgyr_offset.tolist(),
                        'Macc': self.Macc_offset.tolist(), 'Mgyr': self.Mgyr_offset.tolist()},
            'C': {'terrain': self.Cterrain.tolist(), 'fixe': self.Cfixe.tolist(), 'mobile': self.Cmobile.tolist()}
        }
        return result

    def to_json(self):
        return json.dumps(self.getRotWorld(), indent=4)

    def saveToJson(self):
        with open('configRotWorld.json', mode='w') as f:
            json.dump(self.getRotWorld(), f, indent=4)
            # f.write(self.getRotWorld())

    def loadFromJson(self):
        if os.path.exists('configRotWorld.json'):
            with open('configRotWorld.json', mode='r') as f:
                data=json.load(f)
                if data['name']==self.__class__.__name__:
                    print(data)

                    self.Cterrain = np.matrix(data["C"]["terrain"])
                    self.Facc_offset = np.matrix(data["offsets"]["Facc"])
                    self.Fgyr_offset = np.matrix(data["offsets"]["Fgyr"])
                    self.Macc_offset = np.matrix(data["offsets"]["Macc"])
                    self.Mgyr_offset = np.matrix(data["offsets"]["Mgyr"])

                    self.Cfixe = np.matrix(data["C"]["fixe"])
                    self.Cmobile = np.matrix(data["C"]["mobile"])

                    self.isRotWorld = True
                    return True
                else:
                    return False
        else:
            return False


class AngleAnalysis(RotWorld):
    def __init__(self):
        self.last_update = datetime.now()
        self.angleSiege = 0

        # define IMU saved datas
        RotWorld.__init__(self)
        # self.paramIMU=paramIMU()

        #init_state
        self.state = AngleAnalysisState.INIT

        pass

    def update(self,**kwargs):
        # need in kwargs:
        # calibTrigger
        # fixed_imu_data
        # mobile_imu_data
        # config
        if False:
            self.state = AngleAnalysisState.AA_ERROR
        else:
            if self.state == AngleAnalysisState.INIT:
                # Next step is to verify calib
                self.state = AngleAnalysisState.CHECK_CALIBRATION_VALID

            elif self.state == AngleAnalysisState.AA_ERROR:
                # Was in error (disconnected), go back to INIT state
                self.__init__()
                # self.state = AngleAnalysisState.INIT

            elif self.state == AngleAnalysisState.CHECK_CALIBRATION_VALID:
                # Verify is calibration is done
                if self.isRotWorld:
                    self.state = AngleAnalysisState.CALIBRATION_DONE
                else:
                    self.state = AngleAnalysisState.CALIBRATION_TODO
                    
            elif (self.state == AngleAnalysisState.CALIBRATION_WAIT_ZERO_TRIG):
                # Wait for calibration zero angle trigger
                if 'calibTrigger' in kwargs:
                    #start gathering data at Zero angle for calibration
                    self.fixedIMU = []
                    self.mobileIMU = []
                    self.calibTimer = time.time()
                    self.state = AngleAnalysisState.CALIBRATION_RUNNING_ZERO
                elif time.time()-self.calibTimer > kwargs['config'].getint('AngleAnalysis', 'calibrationTIMEOUT'):
                    # TIMEOUT if trigger are too long
                    self.state = AngleAnalysisState.AA_ERROR
                pass

            elif self.state == AngleAnalysisState.CALIBRATION_RUNNING_ZERO:
                # Wait
                if 'fixed_imu_data' in kwargs and 'mobile_imu_data' in kwargs and 'config' in kwargs:
                    try:
                        self.GetMeasureIMU(kwargs['fixed_imu_data'],kwargs['mobile_imu_data'])

                        # end the loop after getting n IMU measures
                        n = kwargs['config'].getint('AngleAnalysis', 'calibrationPeriod')
                        if len(self.fixedIMU) >= n and len(self.mobileIMU) >= n:
                            self.storeParam(fixedIMU_Zero=np.array(self.fixedIMU),
                                                mobileIMU_Zero=np.array(self.mobileIMU))
                            self.calibTimer = time.time()
                            self.state = AngleAnalysisState.CALIBRATION_WAIT_INCLINED_TRIG
                            pass
                        elif time.time()-self.calibTimer > kwargs['config'].getint('AngleAnalysis', 'calibrationTIMEOUT'):
                            self.state = AngleAnalysisState.AA_ERROR
                        else:
                            print(str(len(self.fixedIMU))+" / "+str(n))
                            pass
                    except:
                        self.state = AngleAnalysisState.AA_ERROR
                pass

            elif self.state == AngleAnalysisState.CALIBRATION_WAIT_INCLINED_TRIG:
                # Wait for calibration zero angle trigger
                if 'calibTrigger' in kwargs:
                    #start gathering data at Zero angle for calibration
                    self.fixedIMU = []
                    self.mobileIMU = []
                    self.calibTimer = time.time()
                    self.state = AngleAnalysisState.CALIBRATION_RUNNING_INCLINED
                elif time.time()-self.calibTimer > kwargs['config'].getint('AngleAnalysis', 'calibrationTIMEOUT'):
                    # TIMEOUT if trigger are too long
                    self.state = AngleAnalysisState.AA_ERROR
                pass

            elif self.state == AngleAnalysisState.CALIBRATION_RUNNING_INCLINED:
                if 'fixed_imu_data' in kwargs and 'mobile_imu_data' in kwargs and 'config' in kwargs:
                    try:
                        self.GetMeasureIMU(kwargs['fixed_imu_data'],kwargs['mobile_imu_data'])

                        # end the loop after getting n IMU measures
                        n = kwargs['config'].getint('AngleAnalysis', 'calibrationPeriod')
                        if len(self.fixedIMU) >= n and len(self.mobileIMU) >= n:
                            self.storeParam(fixedIMU_Inclined=np.array(self.fixedIMU),
                                                mobileIMU_Inclined=np.array(self.mobileIMU))
                            self.state = AngleAnalysisState.CALIBRATION_WAIT_ROT_WORLD_CALC
                            pass
                        elif time.time()-self.calibTimer > kwargs['config'].getint('AngleAnalysis', 'calibrationTIMEOUT'):
                            self.state = AngleAnalysisState.AA_ERROR
                        else:
                            print(str(len(self.fixedIMU))+" / "+str(n))
                            pass
                    except:
                        self.state = AngleAnalysisState.AA_ERROR
                pass

            elif self.state == AngleAnalysisState.CALIBRATION_WAIT_ROT_WORLD_CALC:
                try:
                    self.computeRotWorld()
                    if self.isRotWorld == True:
                        self.state = AngleAnalysisState.CALIBRATION_DONE
                    else:
                        self.state = AngleAnalysisState.AA_ERROR
                        pass
                except:
                    self.state = AngleAnalysisState.AA_ERROR
                pass

            elif self.state == AngleAnalysisState.CALIBRATION_DONE:
                # Wait a calibation trigger
                # else compute the measured angle
                if 'calibTrigger' in kwargs:
                    self.calibTimer = time.time()
                    self.state = AngleAnalysisState.CALIBRATION_WAIT_ZERO_TRIG
                else:
                    if 'fixed_imu_data' in kwargs and 'mobile_imu_data' in kwargs:
                        try:
                            self.fixedIMU = []
                            self.mobileIMU = []
                            self.GetMeasureIMU(kwargs['fixed_imu_data'],kwargs['mobile_imu_data'])
                            self.storeParam(fixedIMU_Inclined=np.array(self.fixedIMU),
                                                    mobileIMU_Inclined=np.array(self.mobileIMU))

                            self.angleSiege = self.computeAngle(self.Facc_Inclined,
                                                        self.Fgyr_Inclined,
                                                        self.Macc_Inclined,
                                                        self.Mgyr_Inclined)

                            self.last_update = datetime.now()

                            print(self.angleSiege)
                            print(self.to_json())
                        except:
                            self.state = AngleAnalysisState.AA_ERROR
                pass

            elif self.state == AngleAnalysisState.CALIBRATION_TODO:
                # Wait for a calibation trigger
                if 'calibTrigger' in kwargs:
                    self.calibTimer = time.time()
                    self.state = AngleAnalysisState.CALIBRATION_WAIT_ZERO_TRIG
                pass

        print('State AngleAnalysis: ', self.getStateName())

    def GetMeasureIMU(self,fixed_imu_data,mobile_imu_data):
        m_ax = mobile_imu_data[0]['x']
        m_ay = mobile_imu_data[0]['y']
        m_az = mobile_imu_data[0]['z']
        m_gx = mobile_imu_data[1]['x']
        m_gy = mobile_imu_data[1]['y']
        m_gz = mobile_imu_data[1]['z']
        self.mobileIMU.append([m_ax, m_ay, m_az, m_gx, m_gy, m_gz])

        f_ax = fixed_imu_data[0]['x']
        f_ay = fixed_imu_data[0]['y']
        f_az = fixed_imu_data[0]['z']
        f_gx = fixed_imu_data[1]['x']
        f_gy = fixed_imu_data[1]['y']
        f_gz = fixed_imu_data[1]['z']
        self.fixedIMU.append([f_ax, f_ay, f_az, f_gx, f_gy, f_gz])
        pass

    def getAngleAnalysis(self):
        result=self.getRotWorld()
        result['timestamp']= int(self.last_update.timestamp())
        result['angleSiege']= float(self.angleSiege)

        return result

    def to_json(self):
        return json.dumps(self.getAngleAnalysis())

    def getStateName(self):
        return self.state.name

    def getStateValue(self):
        return self.state.value
