import matplotlib.pyplot as plt
import matplotlib.animation as animation
from PIL import Image
import numpy as np
import os

data_dir = "output/"

files = os.listdir(data_dir)

fig, ax = plt.subplots()


def animate(i):
    ax.clear()
    data = np.loadtxt(data_dir+str(i)+'.txt')
    img = Image.fromarray(np.uint8(data * 255), 'L')
    return plt.imshow(img, cmap='hot', animated=True),


ani = animation.FuncAnimation(fig, animate, frames=len(files), interval=33.333333, blit=True)
writervideo = animation.FFMpegWriter(fps=30)
ani.save("../test.mp4", writer=writervideo)
plt.close()
