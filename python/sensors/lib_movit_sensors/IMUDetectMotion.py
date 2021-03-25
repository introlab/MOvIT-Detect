# Copyright (c) 2021 Adrien Pajon (adrien.pajon@gmail.com)
# 
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT


# import time
# import json
import configparser
import argparse
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
# import math
# from scipy import signal

class IMUDetectMotion:
    def __init__(self,**kwargs) -> None:
        """
        Initialize IMUDetectMotion class object

        Parameters kwargs
        ----------
        sizeSlidingWindow : integer > 0.
            Define the sliding window size to detect the motion

        thresholdAcc : float > 0.
            Define the threshold on acceleration to detect the motion

        thresholdGyro : float > 0.
            Define the threshold on gyroscope to detect the motion

        Returns
        -------
        None
        """
        self.sizeSlidingWindow = 1
        
        self.thresholdAcc = 0.001
        self.thresholdGyro = 2

        self.data=np.zeros(shape=(0,6))

        self.setAttributes(**kwargs)

        pass

    def setAttributes(self,**kwargs) -> None:
        """
        set the class attributes

        Parameters
        ----------
        sizeSlidingWindow : integer > 0, optionnal.
        thresholdAcc : float > 0, optionnal.
        thresholdGyro : float > 0, optionnal.

        Returns
        -------
        None

        See Also
        --------
        self.__init__
        """
        if 'sizeSlidingWindow' in kwargs:
            if kwargs['sizeSlidingWindow'] >1 and isinstance(kwargs['sizeSlidingWindow'], int):
                self.sizeSlidingWindow = kwargs['sizeSlidingWindow']
            else:
                Warning('sizeSlidingWindow must be: integer >0')

        if 'thresholdAcc' in kwargs:
            if kwargs['thresholdAcc'] > 0:
                self.thresholdAcc = kwargs['thresholdAcc']
            else:
                Warning('thresholdAcc must be: float >0')

        if 'thresholdGyro' in kwargs:
            if kwargs['thresholdGyro'] > 0:
                self.thresholdAcc = kwargs['thresholdGyro']
            else:
                Warning('thresholdGyro must be: float >0')

    def addData(self,value) -> None:
        """
        Add data to ``self.data`` and delete old value depending on ``self.sizeSlidingWindow``

        Args:
            value (array(6,)):  [f_ax f_ay f_az f_gx f_gy f_gz]
            f_a = fixed accelerometers
            f_g = fixed gyroscope

        """
        self.data = np.insert(self.data,[0],value,axis=0)
        while len(self.data) > self.sizeSlidingWindow:
            self.data = np.delete(self.data,len(self.data)-1,0)

    def addIMUData(self,IMUdata) -> None:
        """
        Extract IMU data from the class ``mpu6050`` measurement and then store them

        Args:
            IMUdata (mpu6050.get_all_data(raw=True)): IMU measures data from mpu6050
        """
        f_ax = IMUdata[0]['x']
        f_ay = IMUdata[0]['y']
        f_az = IMUdata[0]['z']
        f_gx = IMUdata[1]['x']
        f_gy = IMUdata[1]['y']
        f_gz = IMUdata[1]['z']

        self.addData([f_ax,f_ay,f_az,
                    f_gx,f_gy,f_gz])

    def isMotion(self):
        """
        Analyze if there is a detectable motion over ``self.data``.
        
        Based on the variance of the norm of the measures from the accelerometer and gyroscope.
        The detection is based on threshold in self.thresholdAcc`` and ``self.thresholdGyro``

        Returns:
            Bool: True if motion is detected , False if not
        """
        f_a_norm = np.linalg.norm(self.data[:,0:3],axis=1)
        f_g_norm = np.linalg.norm(self.data[:,3:6],axis=1)
        # f_a_norm = np.sqrt(self.data[:,0]**2 + self.data[:,1]**2 + self.data[:,2]**2)
        # f_g_norm = np.sqrt(self.data[:,3]**2 + self.data[:,4]**2 + self.data[:,5]**2)

        f_a = np.var(f_a_norm)
        f_g = np.var(f_g_norm)

        f_a_bool = f_a > self.thresholdAcc
        f_g_bool = f_g > self.thresholdGyro

        return  f_a_bool + f_g_bool


class IMUloadMeasures(IMUDetectMotion):
    def __init__(self,**kwargs):
        super().__init__()
        self.fileName = []
        self.fh = []
        
        self.IMU_period = 0.1

        self.dataFromFile = np.array([])

        self.setAttributes(**kwargs)

        pass


    def setAttributes(self,**kwargs):  
        super().setAttributes(**kwargs)

        if 'fileName' in kwargs:
            if self.setFileName(fileName=kwargs['fileName']):
                if self.loadFile():
                    self.loadData()
        
        if 'IMU_period' in kwargs:
            if kwargs['IMU_period'] > 0:
                self.IMU_period = kwargs['IMU_period']
            else:
                Warning('IMU_period must be >0')
        


    def setFileName(self,**kwargs):
        if 'fileName' in kwargs:
            self.fileName = kwargs['fileName']
            return True
        else:
            Warning("Missing argument: fileName")
            return False

    def loadFile(self,**kwargs):
        if 'fileName' in kwargs:
            self.setFileName(kwargs['fileName'])
        
        try:
            self.fh = open(self.fileName,"r")
            return True
        except:
            return False

    def loadData(self):
        try:
            self.dataFromFile=np.genfromtxt(self.fh.name, delimiter=", ",names=True)
            return True
        except:
            return False

    def plotRawNormGyroAcc(self,IMU_period):
        ############
        #Encoder's resolution in mm per pulse
        #IMU_period = config.getfloat('Measures','samplingPeriod')
        print("IMU sampling period resolution : "+str(IMU_period))

        ############
        #search for the last logger file based on the indentation
        # filename="Logger_encoder_07.txt"
        # filename=logger.searchLoggerFile(config)
        # data=np.genfromtxt(self.fh.name, delimiter=", ",names=True)
        data = self.dataFromFile

        # #convert the number of pulse position change into mm
        # m_a_raw = np.sqrt(data['m_ax']*data['m_ax'] + data['m_ay']*data['m_ay'] + data['m_az']*data['m_az'])
        # m_g_raw = np.sqrt(data['m_gx']*data['m_gx'] + data['m_gy']*data['m_gy'] + data['m_gz']*data['m_gz'])
        f_a_raw = np.sqrt(data['f_ax']*data['f_ax'] + data['f_ay']*data['f_ay'] + data['f_az']*data['f_az'])
        f_g_raw = np.sqrt(data['f_gx']*data['f_gx'] + data['f_gy']*data['f_gy'] + data['f_gz']*data['f_gz'])

        sizeSlidingWindow = 20

        #cumsum
        # m_a = np.convolve(m_a_raw,np.ones(sizeSlidingWindow,dtype=int),'valid')/sizeSlidingWindow
        # m_g = np.convolve(m_g_raw,np.ones(sizeSlidingWindow,dtype=int),'valid')/sizeSlidingWindow
        # f_a = np.convolve(f_a_raw,np.ones(sizeSlidingWindow,dtype=int),'valid')/sizeSlidingWindow
        # f_g = np.convolve(f_g_raw,np.ones(sizeSlidingWindow,dtype=int),'valid')/sizeSlidingWindow

        # m_a_var = np.convolve(m_a_raw**2,np.ones(sizeSlidingWindow,dtype=int),'valid')/sizeSlidingWindow - np.convolve(m_a_raw, np.ones(sizeSlidingWindow)/sizeSlidingWindow, mode='valid')**2
        # m_g_var = np.convolve(m_g_raw**2,np.ones(sizeSlidingWindow,dtype=int),'valid')/sizeSlidingWindow - np.convolve(m_g_raw, np.ones(sizeSlidingWindow)/sizeSlidingWindow, mode='valid')**2
        f_a = np.convolve(f_a_raw**2,np.ones(sizeSlidingWindow,dtype=int),'valid')/sizeSlidingWindow - np.convolve(f_a_raw, np.ones(sizeSlidingWindow)/sizeSlidingWindow, mode='valid')**2
        f_g = np.convolve(f_g_raw**2,np.ones(sizeSlidingWindow,dtype=int),'valid')/sizeSlidingWindow - np.convolve(f_g_raw, np.ones(sizeSlidingWindow)/sizeSlidingWindow, mode='valid')**2

        m_a_bool = np.array([k> self.thresholdAcc for k in f_a])
        m_g = np.array([k> self.thresholdGyro for k in f_g])

        m_a = m_a_bool+m_g

        #recorded time when datas are received in s
        time = data['time'][0:f_a.shape[0]]/1000
        time-=time[0] #the beginning time at 0

        ############
        #initialize the plot
        fig, ax1 = plt.subplots()

        #plot the encoder velocity in time
        color = 'tab:blue'
        lns1=ax1.plot(time,m_g,label="Gyro IMU Mobile", color=color)
        lns3=ax1.plot(time,f_g,'--',label="Gyro IMU fixe", color=color)
        ax1.set_xlabel("time[s]")
        ax1.set_ylabel("Angular vel[°/s]", color=color)

        color = 'tab:blue'
        ax1.tick_params(axis='y', labelcolor=color)
        ax1.grid()

        #plot the encoder distance measured in m
        ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

        color = 'tab:red'
        ax2.set_ylabel('Acceleration[m/s^2]', color=color)  # we already handled the x-label with ax1
        lns2=ax2.plot(time, m_a, color=color,label="Acc IMU Mobile")
        lns4=ax2.plot(time, f_a, '--', color=color,label="Acc IMU fixe")
        ax2.tick_params(axis='y', labelcolor=color)

        plt.title("velocity and position measured by encoder \n in file : "+self.fh.name)

        # Legend manage if there is no missing value meaning lns3 does not exist
        lns = [lns1[0],lns3[0],lns2[0],lns4[0]]
        labs = (lns1[0].get_label(),lns3[0].get_label(),lns2[0].get_label(),lns4[0].get_label())
        ax1.legend(lns, labs)#, loc=0)
        fig.tight_layout()  # otherwise the right y-label is slightly clipped
        plt.show()

    def plotTestSliding(self):
        ############
        #Encoder's resolution in mm per pulse
        #IMU_period = config.getfloat('Measures','samplingPeriod')
        print("IMU sampling period resolution : "+str(self.IMU_period))

        ############
        data = self.dataFromFile

        # #convert the number of pulse position change into mm
        f_a_raw = np.sqrt(data['f_ax']*data['f_ax'] + data['f_ay']*data['f_ay'] + data['f_az']*data['f_az'])
        f_g_raw = np.sqrt(data['f_gx']*data['f_gx'] + data['f_gy']*data['f_gy'] + data['f_gz']*data['f_gz'])

        f_a = np.convolve(f_a_raw**2,np.ones(self.sizeSlidingWindow,dtype=int),'valid')/self.sizeSlidingWindow - np.convolve(f_a_raw, np.ones(self.sizeSlidingWindow)/self.sizeSlidingWindow, mode='valid')**2
        f_g = np.convolve(f_g_raw**2,np.ones(self.sizeSlidingWindow,dtype=int),'valid')/self.sizeSlidingWindow - np.convolve(f_g_raw, np.ones(self.sizeSlidingWindow)/self.sizeSlidingWindow, mode='valid')**2
        
        # m_a_bool = np.array([k> self.thresholdAcc for k in f_a])
        # m_g = np.array([k> self.thresholdGyro for k in f_g])
        m_a_bool = np.array([])
        for k in range(len(f_a_raw)):
            self.addData([data['f_ax'][k],data['f_ay'][k],data['f_az'][k],
                            data['f_gx'][k],data['f_gy'][k],data['f_gz'][k]])
            if len(self.data) >= self.sizeSlidingWindow:
                m_a_bool = np.insert(m_a_bool,len(m_a_bool),self.isMotion(),axis=0)
                
        # m_a_bool = np.array( for k in f_a_raw])
        # m_a = m_a_bool+m_g
        m_a = m_a_bool
        m_g = m_a

        #recorded time when datas are received in s
        time = data['time'][0:f_a.shape[0]]/1000
        time-=time[0] #the beginning time at 0

        ############
        #initialize the plot
        fig, ax1 = plt.subplots()

        #plot the encoder velocity in time
        color = 'tab:blue'
        lns1=ax1.plot(time,m_g,label="Gyro IMU Mobile", color=color)
        lns3=ax1.plot(time,f_g,'--',label="Gyro IMU fixe", color=color)
        ax1.set_xlabel("time[s]")
        ax1.set_ylabel("Angular vel[°/s]", color=color)

        color = 'tab:blue'
        ax1.tick_params(axis='y', labelcolor=color)
        ax1.grid()

        #plot the encoder distance measured in m
        ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

        color = 'tab:red'
        ax2.set_ylabel('Acceleration[m/s^2]', color=color)  # we already handled the x-label with ax1
        lns2=ax2.plot(time, m_a, color=color,label="Acc IMU Mobile")
        lns4=ax2.plot(time, f_a, '--', color=color,label="Acc IMU fixe")
        ax2.tick_params(axis='y', labelcolor=color)

        plt.title("velocity and position measured by encoder \n in file : "+self.fh.name)

        # Legend manage if there is no missing value meaning lns3 does not exist
        lns = [lns1[0],lns3[0],lns2[0],lns4[0]]
        labs = (lns1[0].get_label(),lns3[0].get_label(),lns2[0].get_label(),lns4[0].get_label())
        ax1.legend(lns, labs)#, loc=0)
        fig.tight_layout()  # otherwise the right y-label is slightly clipped
        plt.show()

if __name__ == "__main__":
    # Make sure current path is this file path
    import os

    abspath = os.path.abspath(__file__)
    dname = os.path.dirname(abspath)
    os.chdir(dname)

    # Look for arguments
    argument_parser = argparse.ArgumentParser(description='IMUrecord')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='config.cfg')
    args = argument_parser.parse_args()

    ############
    # import config file
    config = configparser.ConfigParser()

    print("opening configuration file : config.cfg")
    config.read('config.cfg')
    
    fileName='test_vibration_movit/Test_IMU_07.txt'
    imu_detect_motion = IMUloadMeasures(fileName=fileName,sizeSlidingWindow=20,IMU_period=0.01)

    # imu_detect_motion.plotRawNormGyroAcc(0.01)
    imu_detect_motion.plotTestSliding()

    # fh = open('test_vibration_movit/Test_IMU_06.txt',"r")
    # plotLast(fh,0.01)
    

