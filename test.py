from sys import byteorder
from array import array
from struct import pack
import threading
import queue
import pyaudio
import wave
from numpy import fft
import numpy as np
import time



CHUNK_SIZE = 256
FORMAT = pyaudio.paInt16
RATE = 48000

last = 0 

write_head = 0
read_head = 0


one_sec = CHUNK_SIZE * int(RATE / CHUNK_SIZE)

buf_size = one_sec * 10
print(buf_size)
circular_buffer = array('h', [0] * buf_size)

freq = 50
mod1 = np.sin(np.linspace(-np.pi * freq, np.pi * freq, buf_size))
freq = 30
mod2 = np.sin(np.linspace(-np.pi * freq, np.pi * freq, buf_size))
freq = 10
mod3 = np.sin(np.linspace(-np.pi * freq, np.pi * freq, buf_size))


def onRead(in_data, frame_count, time_info, status_flags):
    global circular_buffer
    global write_head
 
    snd_data = array('h', in_data)

    for sample in snd_data:
        circular_buffer[write_head] = sample
        write_head += 1

    write_head = write_head % buf_size

    return (in_data, pyaudio.paContinue)


def onWrite(in_data, frame_count, time_info, status_flags):
    global last
    global circular_buffer
    global read_head
    global write_head

    while (write_head - read_head <= CHUNK_SIZE) and (write_head - read_head >= 0): 
        print("oops")
        time.sleep(0.04)
    
    chunk = array('h')
    for i in range(CHUNK_SIZE):
        sample = circular_buffer[read_head]
        sample += circular_buffer[(read_head - 240) % buf_size] * mod1[read_head]
        sample += circular_buffer[(read_head - 1000) % buf_size] * mod2[read_head]
        sample += circular_buffer[(read_head - 1920) % buf_size] * mod3[read_head]
        sample = int(sample * 0.25)
        chunk.append(max(min(sample, 32767), -32768))
        read_head += 1

    read_head = read_head % buf_size

    return (chunk.tostring(), pyaudio.paContinue)




p = pyaudio.PyAudio()
stream_1 = p.open(format=FORMAT, channels=1, rate=RATE,
    input=True, output=False,
    frames_per_buffer=CHUNK_SIZE,
    stream_callback=onRead)

stream_2 = p.open(format=FORMAT, channels=1, rate=RATE,
    input=False, output=True,
    frames_per_buffer=CHUNK_SIZE,
    stream_callback=onWrite)


stream_1.start_stream()
stream_2.start_stream()

time.sleep(5)

stream_2.stop_stream()
stream_1.stop_stream()

while stream_1.is_active() or stream_2.is_active():
    time.sleep(1)

stream_1.close()
stream_2.close()


p.terminate()





