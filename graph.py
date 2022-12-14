import matplotlib.pyplot as plt
import numpy as np
import os

data = "cmake-build-debug/mags/"
files = os.listdir(data)
test = np.loadtxt(data+files[0], skiprows=1)
mags = np.zeros(test[:, 0].shape)

for file in files:
    arr = np.loadtxt(data+file, skiprows=1)
    mags += arr[:, 1]

mags /= len(files)

plt.figure(figsize=(10, 5))
plt.scatter(arr[:, 0], mags)
plt.ylabel("Normalised Magnetisation (A/m)")
plt.xlabel("Temperature (K)")
plt.title(f"Average Magnetisation vs Temperature, {len(files)} Runs")
plt.savefig("Basic Conditions.pdf")
plt.show()
