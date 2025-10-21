import numpy as np
import librosa
from pydub import AudioSegment

pcm_data = np.fromfile("smeshariki_after_rx_inter.pcm", dtype=np.int16)
audio = AudioSegment(
    data=pcm_data.tobytes(),
    sample_width=4,
    frame_rate=44100,
    channels=1
)

audio.export("smehariki_after_rx_inter.mp3", format="mp3", bitrate="192k")