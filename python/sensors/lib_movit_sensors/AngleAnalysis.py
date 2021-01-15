import json
import threading
import numpy as np
# import matplotlib.pyplot as plt
import math
from scipy import signal
from datetime import datetime
import os


import lib_movit_sensors.rmoutliers as rmo


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
        # Localiser axe gravit� (approx)
        Facc_mean = np.mean(self.Facc_Zero, axis=1)
        Macc_mean = np.mean(self.Macc_Zero, axis=1)
        FaxG = np.where(Facc_mean == np.amax(Facc_mean))[0][0]
        MaxG = np.where(Macc_mean == np.amax(Macc_mean))[0][0]

        Fgref = [0, 0, 0]
        Fgref[FaxG] = 1

        Mgref = [0, 0, 0]
        Mgref[MaxG] = 1

        ##
        Facc_calibNoOut, facc_out = rmo.rmoutliers_percentiles(self.Facc_Zero, 1.645, 1.645)
        Macc_calibNoOut, macc_out = rmo.rmoutliers_percentiles(self.Macc_Zero, 1.645, 1.645)
        Fgyr_calibNoOut, fgyr_out = rmo.rmoutliers_percentiles(self.Fgyr_Zero, 1.645, 1.645)
        Mgyr_calibNoOut, mgyr_out = rmo.rmoutliers_percentiles(self.Mgyr_Zero, 1.645, 1.645)

        # KL - 06nov: ajustement ax gravit�
        Facc_offset = np.mean(Facc_calibNoOut, axis=1) - Fgref
        Macc_offset = np.mean(Macc_calibNoOut, axis=1) - Mgref
        Fgyr_offset = np.mean(Fgyr_calibNoOut, axis=1)
        Mgyr_offset = np.mean(Mgyr_calibNoOut, axis=1)

        # Remarque 1: ***Consid�rer le filtrage?***
        # Remarque 2: En calibrant comme �a, on assume que c'est du bruit de
        # capteur et que ce n'Est pas une erreur d'alignement interne... mais
        # bon... on va tenter de palier � tout cela en faisant une certaine
        # it�ration...

        Macc_offset2 = [0, 0, 0]
        Mgyr_offset2 = [0, 0, 0]

        ## search offsets
        Cterrain = []
        Cmobile = []
        np.cosd = lambda x: np.cos(np.deg2rad(x))
        np.sind = lambda x: np.sin(np.deg2rad(x))

        for icalib in range(20):
            Macc_offset = Macc_offset + Macc_offset2
            Mgyr_offset = Mgyr_offset + Mgyr_offset2

            ## Cr�ation de vecteurs de l'essai
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

            # Facc = (Cterrain*self.initIMUParameters.Facc_Inclined).T - Facc_offset
            # Macc = (Cterrain*self.initIMUParameters.Macc_Inclined).T - Macc_offset

            # Fgyr = self.initIMUParameters.Fgyr_Inclined.T - Fgyr_offset
            # Mgyr = self.initIMUParameters.Mgyr_Inclined.T - Mgyr_offset

            Facc_Zero = (Cterrain * self.Facc_Zero).T - Facc_offset
            Macc_Zero = (Cterrain * self.Macc_Zero).T - Macc_offset

            Fgyr_Zero = self.Fgyr_Zero.T - Fgyr_offset
            Mgyr_Zero = self.Mgyr_Zero.T - Mgyr_offset

            Facc_Inclined = (Cterrain * self.Facc_Inclined).T - Facc_offset
            Macc_Inclined = (Cterrain * self.Macc_Inclined).T - Macc_offset

            Fgyr_Inclined = self.Fgyr_Inclined.T - Fgyr_offset
            Mgyr_Inclined = self.Mgyr_Inclined.T - Mgyr_offset

            ## CALIBRATION POSITIONNEMENT STATIQUE

            # (1) D�finir la p�riode statique
            #     figure
            #     plot(fixed.time, Macc)
            #     grid on grid minor

            Facc_stat = np.mean(Facc_Zero, axis=0)
            Macc_stat = np.mean(Macc_Zero, axis=0)

            # (2) Inclinaison initiale des IMUs:

            # ** IMU fixe **
            theta = math.atan2(Facc_stat[0, 0], Facc_stat[0, 2])
            C1_fixe = np.matrix([[np.cos(theta), 0, np.sin(theta)],
                                 [0, 1, 0],
                                 [-np.sin(theta), 0, np.cos(theta)]]).T

            # Correction capteurs dans IMU fixe
            # Facc_cor = (C1_fixe*Facc.T).T
            # Fgyr_cor = (C1_fixe*Fgyr.T).T

            Facc_Zero_cor = (C1_fixe * Facc_Zero.T).T
            Fgyr_Zero_cor = (C1_fixe * Fgyr_Zero.T).T

            Facc_Inclined_cor = (C1_fixe * Facc_Inclined.T).T
            Fgyr_Inclined_cor = (C1_fixe * Fgyr_Inclined.T).T

            # ** IMU mobile **
            phi = math.atan2(Macc_stat[0, 0], Macc_stat[0, 2])
            C1_mobile = np.matrix([[np.cos(phi), 0, np.sin(phi)],
                                   [0, 1, 0],
                                   [-np.sin(phi), 0, np.cos(phi)]]).T

            # Correction capteurs dans IMU mobile
            # Macc_cor = (C1_mobile*Macc.T).T
            # Mgyr_cor = (C1_mobile*Mgyr.T).T

            Macc_Zero_cor = (C1_mobile * Macc_Zero.T).T
            Mgyr_Zero_cor = (C1_mobile * Mgyr_Zero.T).T

            Macc_Inclined_cor = (C1_mobile * Macc_Inclined.T).T
            Mgyr_Inclined_cor = (C1_mobile * Mgyr_Inclined.T).T

            # (3) Axe de rotation (IMU mobile) pendant la bascule

            # Remarque: Trouver de fa�on autonome la p�riode de bascule...
            Mgyr_basc = Mgyr_Inclined_cor

            Mgyr_bascMod = np.sqrt(np.sum(np.multiply(Mgyr_basc, Mgyr_basc), axis=1))
            Mgyr_bascN = np.divide(Mgyr_basc, Mgyr_bascMod * np.ones([1, 3]))
            Mgyr_Dir = np.mean(Mgyr_bascN, axis=0)
            # La bonne nouvelle c'est que la composante en z est presque nulle...
            # donc l'alignement avec acc�l�ro tiens la route... la petite
            # composante est soit un r�siduel de l'alignement vs offset initial,
            # soit la vibration carr�ment... pour l'instant, je mets de c�t�.

            # Trouvons l'angle de rotation autour de z qui permettra d'avoir une
            # bascule purement en "y".
            vecBascule = Mgyr_Dir / np.sqrt(Mgyr_Dir * Mgyr_Dir.T)
            ang_Rotz = math.atan2(vecBascule[0, 0], vecBascule[0, 1])
            Mat_Rotz = np.matrix([[np.cos(ang_Rotz), -np.sin(ang_Rotz), 0],
                                  [np.sin(ang_Rotz), np.cos(ang_Rotz), 0],
                                  [0, 0, 1]])

            # v�rification axe bascule:
            # test = Mat_Rotz * vecBascule.T
            # print(test)
            # OK... pas mal �a... reste la petite composante en z mais...

            # Matrice alignement principale (mobile)
            Cmobile = Mat_Rotz * C1_mobile

            # On va essayer d'optimiser un peu en validant pour 3 vecteurs...
            # (i) vecteur statique initial, normalis�...
            vecIni = np.mean(Macc_Zero, axis=0)
            # vecIni = vecIni / sqrt(vecIni*vecIni')
            vecIni_al = Cmobile * vecIni.T
            # #(ii) vecteur statique final, normalis� (normalement, le m�me
            # #     mais si j'ai introduit une erreur, on va le voir...
            # vecFin = np.mean(Macc[Macc.shape[0]-1-10:Macc.shape[0]-1,:],axis=0)
            # #vecFin = vecFin / sqrt(vecFin*vecFin')
            # vecFin_al = Cmobile*vecFin.T
            # (iii) Vecteur de bascule
            vecBascule = Mgyr_Dir
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

        # ORIGINAL
        Facc_offset = np.mean(self.Facc_Zero, axis=1) - [0, 0, 1]
        Fgyr_offset = np.mean(self.Fgyr_Zero, axis=1)

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

        Macc = (Cterrain * self.Macc_Inclined).T - Macc_offset

        signalToFilter = (Cterrain * self.Mgyr_Inclined)
        if signalToFilter.shape[1] > ntaps:
            Mgyr = signal.filtfilt(a, b, signalToFilter).T - Mgyr_offset
        else:
            Mgyr = signalToFilter.T - Mgyr_offset

        ##
        Cfixe = []
        for j in range(20):
            Fgyr_offset = Fgyr_offset + Fgyr_offset2
            Facc_offset = Facc_offset + Facc_offset2

            # Inclinaison IMU fixe
            Facc_stat = np.mean(Facc_Zero, axis=0)
            theta = math.atan2(Facc_stat[0, 0], Facc_stat[0, 2])
            C1_fixe = np.matrix([[np.cos(theta), 0, np.sin(theta)],
                                 [0, 1, 0],
                                 [-np.sin(theta), 0, np.cos(theta)]]).T

            # Correction capteurs dans IMU fixe
            Facc_cor = (C1_fixe * Facc.T).T
            Fgyr_cor = (C1_fixe * Fgyr.T).T

            Facc_offset2 = np.mean(Facc_cor, axis=0) - [0, 0, 1]
            Fgyr_offset2 = np.mean(Fgyr_cor, axis=0)
            Cfixe = C1_fixe

        # Facc_cor = (Cfixe*Facc.T).T
        # Fgyr_cor = (Cfixe*Fgyr.T).T
        # Macc_cor = (Cmobile*Macc.T).T
        # Mgyr_cor = (Cmobile*Mgyr.T).T

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
                    self.Facc_offset = data["offsets"]["Facc"]
                    self.Fgyr_offset = data["offsets"]["Fgyr"]
                    self.Macc_offset = data["offsets"]["Macc"]
                    self.Mgyr_offset = data["offsets"]["Mgyr"]

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
        # BOOL for state
        self.isGetInitIMU = False

        self.askAtZero = False  # ask the trigger for seat at zero orientation
        self.isAtZero = False  # trigger for seat at zero orientation
        self.askInclined = False  # ask the trigger for seat to be tilted
        self.isInclined = False  # waiting trigger for seat to be tilted

        self.isMeasuringRealTime = False
        self.askMeasuringRealTime = False

        self.last_update = datetime.now()
        self.angleSiege = 0

        # define IMU saved datas
        RotWorld.__init__(self)
        self.paramIMU=paramIMU()
        pass

    def createClient(self, clientName,config):
        import paho.mqtt.client as mqtt
        #create client connected to mqtt broker
        
        #create new instance
        print("MQTT creating new instance named : "+clientName)
        client = mqtt.Client(clientName) 
        
        #set username and password
        print("MQTT setting  password")
        client.username_pw_set(username=config.get('MQTT','usr'),password=config.get('MQTT','pswd'))
        
        #connection to broker
        broker_address=config.get('MQTT','broker_address')
        print("MQTT connecting to broker : "+broker_address)
        client.connect(broker_address) #connect to broker

        return client


    def intializeIMU(self, clientMQTT, config):
        self.isGetInitIMU = False
        self.isRotWorld = False

        # x = threading.Thread(target=self.getCalibrationIMU, args=(clientMQTT,config,))
        # x.start()

        # Create another client here...
        client = self.createClient("AngleAnalysis", config)
        client.loop_start()

        # x = threading.Thread(target=self.calibrateIMU, args=(clientMQTT, config,))
        # USING NEW CLIENT!
        x = threading.Thread(target=self.calibrateIMU, args=(client, config,))
        x.start()

    def calibrateIMU(self, clientMQTT, config):
        self.getCalibrationIMU(clientMQTT, config)

        self.computeRotWorld()

    @staticmethod
    def on_message_GetIMU(client, userdata, message):
        data = str(message.payload.decode("utf-8"))
        info = json.loads(data)

        # time = info['time']
        m_ax = info['mobile_imu']['accelerometers']['x']
        m_ay = info['mobile_imu']['accelerometers']['y']
        m_az = info['mobile_imu']['accelerometers']['z']
        m_gx = info['mobile_imu']['gyros']['x']
        m_gy = info['mobile_imu']['gyros']['y']
        m_gz = info['mobile_imu']['gyros']['z']
        #             mimu  = (time, m_ax, m_ay, m_az, m_gx, m_gy, m_gz)
        client.mobileIMU.append([m_ax, m_ay, m_az, m_gx, m_gy, m_gz])

        # print(client.mobileIMU_zero)

        f_ax = info['fixed_imu']['accelerometers']['x']
        f_ay = info['fixed_imu']['accelerometers']['y']
        f_az = info['fixed_imu']['accelerometers']['z']
        f_gx = info['fixed_imu']['gyros']['x']
        f_gy = info['fixed_imu']['gyros']['y']
        f_gz = info['fixed_imu']['gyros']['z']
        #             fimu  = (time, f_ax, f_ay, f_az, f_gx, f_gy, f_gz)
        client.fixedIMU.append([f_ax, f_ay, f_az, f_gx, f_gy, f_gz])

        print(len(client.fixedIMU))

    @staticmethod
    def loopGetIMU(clientMQTT, config):
        # Loop to get IMU meaures

        # initialize loop parameters
        clientMQTT.fixedIMU = []
        clientMQTT.mobileIMU = []

        # subscribe topic
        topic_websocket = config.get('MQTT', 'topic_subscribe')
        print("Subscribing to topic", topic_websocket)
        clientMQTT.subscribe(topic_websocket)

        # start the loop
        clientMQTT.loop_start()

        # end the loop after getting n IMU measures
        n = config.getint('AngleAnalysis', 'calibrationPeriod')
        while len(clientMQTT.fixedIMU) < n and len(clientMQTT.mobileIMU) < n:
            import time
            time.sleep(0.1)
        else:
            clientMQTT.loop_stop()
            clientMQTT.unsubscribe(topic_websocket)
            fixedIMU = clientMQTT.fixedIMU
            mobileIMU = clientMQTT.mobileIMU
            clientMQTT.fixedIMU = []
            clientMQTT.mobileIMU = []

        return fixedIMU, mobileIMU

    def getCalibrationIMU(self, clientMQTT, config):
        self.askAtZero = False
        self.askInclined = False
        self.isAtZero = False
        self.isInclined = False

        # Essai de calibration (tout � 0)
        clientMQTT.on_message = self.on_message_GetIMU

        # get IMU measures when seat at zero
        self.askAtZero = True
        while not self.isAtZero:
            # pass
            import time
            time.sleep(0.1)
        else:
            fixedIMU_Zero, mobileIMU_Zero = self.loopGetIMU(clientMQTT, config)
            self.askAtZero = False
            self.isAtZero = False

        # get IMU measures when seat is inclined
        self.askInclined = True
        while not self.isInclined:
            # pass
            import time
            time.sleep(0.1)
        else:
            fixedIMU_Inclined, mobileIMU_Inclined = self.loopGetIMU(clientMQTT, config)
            self.askInclined = False
            self.isInclined = False

        fixedIMU_Zero = np.array(fixedIMU_Zero)
        mobileIMU_Zero = np.array(mobileIMU_Zero)
        fixedIMU_Inclined = np.array(fixedIMU_Inclined)
        mobileIMU_Inclined = np.array(mobileIMU_Inclined)

        ##
        paramIMU.__init__(self)
        self.storeParam(fixedIMU_Zero=fixedIMU_Zero,
                   mobileIMU_Zero=mobileIMU_Zero,
                   fixedIMU_Inclined=fixedIMU_Inclined,
                   mobileIMU_Inclined=mobileIMU_Inclined)

        self.isGetInitIMU = True

    def startGetAngle(self, clientMQTT, config):
        self.isMeasuringRealTime = False
        self.askMeasuringRealTime = False

        if self.isRotWorld:
            x = threading.Thread(target=self.getAngle, args=(clientMQTT, config,))
            x.start()
        else:
            print("IMU not calibrated")

    def getAngle(self, clientMQTT, config):

        clientMQTT.on_message = self.on_message_GetIMU

        # initialize loop parameters
        clientMQTT.fixedIMU = []
        clientMQTT.mobileIMU = []
        paramIMU.__init__(self)

        # subscribe topic
        topic_websocket = config.get('MQTT', 'topic_subscribe')
        print("Subscribing to topic", topic_websocket)
        clientMQTT.subscribe(topic_websocket)

        # start the loop
        clientMQTT.loop_start()

        self.askMeasuringRealTime = True
        while self.askMeasuringRealTime:
            if len(clientMQTT.fixedIMU) < 1 and len(clientMQTT.mobileIMU) < 1:
                # pass
                import time
                time.sleep(0.1)
            else:
                fixedIMU_Inclined = np.array(clientMQTT.fixedIMU)
                mobileIMU_Inclined = np.array(clientMQTT.mobileIMU)
                clientMQTT.fixedIMU = []
                clientMQTT.mobileIMU = []

                ##
                self.storeParam(fixedIMU_Inclined=fixedIMU_Inclined,
                                  mobileIMU_Inclined=mobileIMU_Inclined)

                angleSiege = self.computeAngle(self.Facc_Inclined,
                                               self.Fgyr_Inclined,
                                               self.Macc_Inclined,
                                               self.Mgyr_Inclined)

                self.last_update = datetime.now()
                self.angleSiege=angleSiege

                print(angleSiege)
                print(self.to_json())

            pass
        else:
            clientMQTT.loop_stop()
            clientMQTT.unsubscribe(topic_websocket)
            clientMQTT.fixedIMU = []
            clientMQTT.mobileIMU = []

            self.isMeasuringRealTime = False

    def getAngleAnalysis(self):
        result=self.getRotWorld()
        result['timestamp']= int(self.last_update.timestamp())
        result['angleSiege']= float(self.angleSiege)

        return result
    def to_json(self):
        return json.dumps(self.getAngleAnalysis())