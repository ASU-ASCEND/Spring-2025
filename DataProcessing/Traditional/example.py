import matplotlib.pyplot as plt
import pandas as pd
import os

csvPath = os.path.join("..", "Spring2024Data.CSV")

df = pd.read_csv(csvPath, skiprows=[0])
df = df.reset_index()

print(list(df.columns))

time = []
alt = []

for index, row in df.iterrows():
  if(row[" BME680 Approx Alt m"] != " -"):
    time.append(float(row["Millis"]))
    alt.append(float(row[" BME680 Approx Alt m"]))

plt.scatter(time, alt)
plt.savefig("example.svg")
plt.savefig("example.png")

plt.show()

