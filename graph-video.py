import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import os

data_dir = "cmake-build-debug/mags/"

files = os.listdir(data_dir)

fig = plt.figure(figsize=(10, 5))
ax = fig.gca()

test = np.loadtxt(data_dir+files[0], skiprows=1)


def animate(i):
    ax.clear()
    mags = np.zeros(test[:, 0].shape)
    for j in range(i+1):
        data = np.loadtxt(data_dir+files[j], skiprows=1)
        mags += data[:, 1]/(i+1)
    plt.ylabel("Normalised Magnetisation (A/m)")
    plt.xlabel("Temperature (K)")
    plt.title(f"Average Magnetisation vs Temperature, {i+1} Runs")
    ax.scatter(test[:, 0], mags)
    return fig,


ani = animation.FuncAnimation(fig, animate, frames=len(files), interval=33.333333, blit=True)
writervideo = animation.FFMpegWriter(fps=30)
ani.save("graph.mp4", writer=writervideo)
plt.close()
