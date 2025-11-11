import numpy as np
import matplotlib.pyplot as plt
import math

name = "symb_after_rx.pcm"

data = []
imag = []
real = []
count = []
absIQ = []
counter = 0
with open(name, "rb") as f:
    index = 0
    while (byte := f.read(2)):
        if(index % 2 == 0):
            Q = int.from_bytes(byte, byteorder='little', signed=True)
            real.append(Q)
            counter += 1
            count.append(counter)
        else:
            I = int.from_bytes(byte, byteorder='little', signed=True)
            imag.append(I)
        index += 1
for i in range(len(imag)):
    abs = math.sqrt(imag[i] * imag[i] + real[i] * real[i])
    absIQ.append(abs)


plt.figure(1)

plt.plot(count,(imag),'r-')
plt.plot(count,(real), 'b-')
plt.show()

plt.figure(2)
plt.plot(count,(absIQ), 'g-')
plt.show()  