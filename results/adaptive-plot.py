import numpy as np
import matplotlib.pyplot as plt

data = np.genfromtxt(fname="results/adaptive-history.tsv", delimiter="\t", skip_header=1, filling_values=1)[:,:4]
data = np.concatenate((np.array([[.25, .25, .25, .25]]), data))

x = np.arange(0, data.shape[0])

stride = 100
cutoff = data.shape[0]

x = x[:cutoff:stride]
data = data[:cutoff:stride,:]

plt.plot(x, data[:, 0], label="00")
plt.plot(x, data[:, 1], label="01")
plt.plot(x, data[:, 2], label="10")
plt.plot(x, data[:, 3], label="11")

plt.legend()

plt.show()
