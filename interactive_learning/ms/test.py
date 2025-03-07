#!/usr/bin/env python
# coding: utf-8

# In[1]:


import pandas as pd
import numpy as np


# In[2]:


df = pd.read_csv("asia10K.csv", sep=None, engine='python', na_values='?')
df = pd.read_csv("abalone.data", sep=None, engine='python', na_values='?')
index_constant = np.where(df.nunique() == 1)[0]
constant_columns = [df.columns[i] for i in index_constant]
df = df.drop(columns=constant_columns)
df = df.dropna()
cat_data = df.select_dtypes('object').astype('category')
for c in cat_data:
    df = df.assign(**{c: cat_data[c]})
float_data = df.select_dtypes('number').astype('float64')
for c in float_data:
    df = df.assign(**{c: float_data[c]})


# In[3]:


import pybnesian as pbn


# In[4]:


mskcmi = pbn.MixedKMutualInformation(df=df, k=100, seed=42, samples=10, scaling="normalized_rank",gamma_approx=True, adaptive_k=True)


# In[ ]:


df.columns


# In[ ]:


mskcmi.mi('Smoker','LungCancer', 'TuberculosisOrCancer')
# mskcmi.mi('Height', 'Length')

