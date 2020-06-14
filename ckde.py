import pyarrow as pa
from pgm_dataset.factors.continuous import KDE
import numpy as np
import pandas as pd
from scipy.stats import gaussian_kde, norm, multivariate_normal
from scipy.special import logsumexp
import time

np.random.seed(1)

SIZE = 10000000

a_array = np.random.normal(3, 0.5, size=SIZE)
b_array = 2.5 + 1.65*a_array + np.random.normal(0, 2, size=SIZE)
c_array = -4.2 - 1.2*a_array + 3.2*b_array + np.random.normal(0, 0.75, size=SIZE)
d_array = 1.5 - 0.9*a_array + 5.6*b_array + 0.3 * c_array + np.random.normal(0, 0.5, size=SIZE)


df = pd.DataFrame({
                    'a': a_array,
                    'b': b_array,
                    'c': c_array,
                    'd': d_array
                    })



k = gaussian_kde(df.to_numpy().T)

covariance = k.covariance
cholesky = np.linalg.cholesky(covariance)

TRAIN_POINT = 1000
OFFSET_SHOW = 56123
SHOW_INSTANCES = 20

spk = gaussian_kde(df.iloc[:TRAIN_POINT, :].loc[:, ["a", "b"]].to_numpy().T)
start_time = time.time()
lp = spk.logpdf(df.iloc[(TRAIN_POINT + OFFSET_SHOW):(TRAIN_POINT + OFFSET_SHOW + SHOW_INSTANCES), :].loc[:, ["a", "b"]].to_numpy().T)
end_time = time.time()
print("Python time: " + str(end_time - start_time))
print(lp)

k = KDE(["a", "b"])
k.fit(df.iloc[:TRAIN_POINT])
start_time = time.time()
lc = k.logpdf(df.iloc[TRAIN_POINT:])
end_time = time.time()
print("c++ time: " + str(end_time - start_time))
print(lc[OFFSET_SHOW:(OFFSET_SHOW + SHOW_INSTANCES)])
