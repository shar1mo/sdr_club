import numpy as np
import librosa
from pydub import AudioSegment

y, sr = librosa.load("smeshariki.mp3", sr=44100, mono=True)
pcm_data = (y * 32767).astype(np.int16)
pcm_data.tofile("smeshariki.pcm")
