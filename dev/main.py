import numpy as np
import matplotlib.pyplot as plt
import math

name = "rx_buff.pcm"

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

        
# Инициализируем список для хранения данных


# fig, axs = plt.subplots(2, 1, layout='constrained')
plt.figure(1)

# axs[1].plot(count, np.abs(data),  color='grey')  # Используем scatter для диаграммы созвездия
plt.plot(count,(imag),'r-')  # Используем scatter для диаграммы созвездия
plt.plot(count,(real), 'b-')  # Используем scatter для диаграммы созвездия
plt.show()

plt.figure(2)
plt.plot(count,(absIQ), 'g-')  # Используем scatter для диаграммы созвездия
plt.show()  