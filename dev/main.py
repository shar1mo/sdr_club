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

filter = np.ones(10)

print(filter)

name2 = np.convolve(real, filter)

plt.figure(2)
plt.plot(name2)
plt.show()  

threshold = 100  
signal_present = np.abs(real) > threshold

if np.any(signal_present):
    start_idx = np.argmax(signal_present)
    end_idx = len(real) - np.argmax(signal_present[::-1])
    
    real_data = real[2165:3121]
    imag_data = imag[start_idx:end_idx]
    
    print(f"Data range: {start_idx} to {end_idx} (total: {end_idx - start_idx} samples)")
else:
    real_data = real
    imag_data = imag
    print("No significant signal found, showing all data")

step = 10
real_10 = real_data[::step]

plt.figure(figsize=(8, 8))
plt.scatter(real_10, real_10, alpha=0.5, s=5)
plt.title(f'Constellation (every {step}th point)')
plt.show()