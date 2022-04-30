import numpy as np
import pandas as pd
import scipy.stats as stats

class IqrFilter:
    def __init__(self, humidities, temperatures):
        self.temperatures = np.array(temperatures)
        self.humidities = np.array(humidities)

    def clean_data(self):
        data_frame = pd.DataFrame(np.array([self.humidities, self.temperatures]).T, columns = ['Humidity', 'Temperature'])

        q1 = data_frame.quantile(q=.25)
        q3 = data_frame.quantile(q=.75)

        iqr = data_frame.apply(stats.iqr)

        clean_data = data_frame[~((data_frame < (q1 - 1.5 * iqr))).any(axis = 1)]

        return clean_data
