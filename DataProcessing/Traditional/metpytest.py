import matplotlib.pyplot as plt
import numpy as np
import metpy.calc as mpcalc
from metpy.plots import SkewT
from metpy.units import units

import pandas as pd
data = pd.read_csv("../Spring2024Data.CSV", skipinitialspace=True, skiprows=1)

fig = plt.figure(figsize = (9,9))
skew = SkewT(fig=fig, aspect='auto')

raw_alt = np.array([float(i) for i in data["BME680 Approx Alt m"] if i != "-"])

raw_pres = np.array([float(i) for i in data["BME680 Pressure hPa"] if i != "-"]) * units.hPa
raw_temp = np.array([float(i) for i in data["BME680 Temp C"] if i != "-"]) * units.degC
raw_rel_hum = np.array([float(i)/100 for i in data["BME680 Humidity %"] if i != "-"]) * units.dimensionless
raw_dewpoint = mpcalc.dewpoint_from_relative_humidity(temperature=raw_temp, relative_humidity=raw_rel_hum)

cooked_pres = []
cooked_temp = []
cooked_dewpoint = []
lowest_pres = 100

min_pressure_index = list(raw_pres).index(np.min(raw_pres))
print(min_pressure_index, len(raw_pres))
cut_off_pres = 100
starting_alt = np.mean(raw_alt[0:10]) # roughly the ground 
for i in range(min_pressure_index,len(raw_pres)):

  # if(raw_alt[i] - starting_alt <= 10 or raw_temp[i].magnitude > 40): continue 
  if(raw_pres[i].magnitude <= cut_off_pres): continue

  if(raw_pres[i].magnitude > lowest_pres and i % 20 == 0):
    cooked_pres.append(raw_pres[i].magnitude)
    cooked_temp.append(raw_temp[i].magnitude)
    cooked_dewpoint.append(raw_dewpoint[i].magnitude)
    lowest_pres = raw_pres[i].magnitude

cooked_pres.reverse()
cooked_temp.reverse()
cooked_dewpoint.reverse(); 

cooked_pres = np.array(cooked_pres) * units.hPa
cooked_temp = np.array(cooked_temp) * units.degC
cooked_dewpoint = np.array(cooked_dewpoint) * units.degC

print(cooked_pres)
print(cooked_temp[0])
print(cooked_dewpoint[0])

prof = mpcalc.parcel_profile(cooked_pres, cooked_temp[0], cooked_dewpoint[0]).to('degC')
u = np.linspace(-10, 10, len(cooked_pres)) * units.knots
v = np.linspace(-20, 20, len(cooked_pres)) * units.knots

skew.plot(cooked_pres, cooked_temp, 'r')
skew.plot(cooked_pres, cooked_dewpoint, 'g')
skew.plot(cooked_pres, prof, 'k')
skew.plot_barbs(cooked_pres[::5], u[::5], v[::5])

skew.ax.set_xlim(-50, 50)
skew.ax.set_ylim(1000, min(cooked_pres))

skew.plot_dry_adiabats()
skew.plot_moist_adiabats()
skew.plot_mixing_lines()

plt.show()