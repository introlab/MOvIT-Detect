import numpy as np

def rmoutliers_percentiles(data,n_up,n_down):
      # n_up and n_down are scalar to set the std trust treshold
      data_temp=data
      data_std=np.std(data_temp,axis=1)
      data_mean=np.mean(data_temp,axis=1)

      outliers=[]
      for k in range(data_temp.shape[1]):
            test=False
            for h in range(data_temp.shape[0]):
                  test+=(data_temp[h][k]>=(data_mean[h]+(data_std[h]*n_up)))
            if test>0:
                  outliers.append(k)

            test=False
            for h in range(data_temp.shape[0]):
                  test+=(data_temp[h][k]<=(data_mean[h]-(data_std[h]*n_down)))
            if test>0:
                  outliers.append(k)

      outliers_=[]
      for x in outliers:
          if x not in outliers_:
              outliers_.append(x)

      outliers_.sort(reverse=True)

      TF = [False for i in range(data_temp.shape[1])]
      for k in outliers_:
            data_temp=np.delete(data_temp,k,1)
            TF[k]=True

      if data_temp.shape[1]==0:
          data_temp=data
          TF = [False for i in range(data_temp.shape[1])]

      return data_temp, TF