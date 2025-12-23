import numpy as np
import matplotlib.pyplot as plt
import math

name = "symb_after_rx.pcm"

real = []
imag = []
count = []


with open(name, "rb") as f:
    index = 0
    counter = 0
    while (byte := f.read(2)):
        value = int.from_bytes(byte, byteorder='little', signed=True)
        if index % 2 == 0:
            real.append(value)
            counter += 1
            count.append(counter)
        else:
            imag.append(value)
        index += 1


N = min(len(real), len(imag))
real = np.array(real[:N])
imag = np.array(imag[:N])
count = count[:N]

absIQ = np.sqrt(real**2 + imag**2)

plt.figure(figsize=(8, 8))
plt.scatter(real, imag, s=1, alpha=0.5)
plt.title("IQ before filtering")
plt.xlabel("I")
plt.ylabel("Q")
plt.grid()
plt.axis("equal")
plt.show()


plt.figure()
plt.plot(real, label="I")
plt.plot(imag, label="Q")
plt.legend()
plt.grid()
plt.show()

plt.figure()
plt.plot(absIQ)
plt.title("|IQ|")
plt.grid()
plt.show()
