import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from metpy.plots import SkewT
from metpy.units import units
from metpy.calc import dewpoint_from_relative_humidity, parcel_profile

# Fetch CSV flight data
data = pd.read_csv('Spring2025.csv')

# Parse & convert data to MetPy-readable format
pressure = (data['BMP390 Pressure (Pa)'].values * units.pascal).to('hPa')
temperature = (data['BMP390 Temp (C)'])