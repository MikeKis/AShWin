import os
import matplotlib.pyplot as plt
import numpy as np
import subprocess

N = 20
M = 20

tactStart = 400
tactEnd = 500

video_file = 'DVSspikes.mp4'
files = []

def process(file, n, n1st):
    file.seek(n * tactStart)

    fig, ax = plt.subplots(figsize=(5, 5))

    for i in range(tactStart, tactEnd):
        line = file.readline()
        if len(line) != n1st:
            return False
        line = line[:-1]

        line_numbers = []
        for k, ch in enumerate(line):
            if ch == '.':
                line_numbers.append(0)
            else:
                line_numbers.append(255)

        length = len(line_numbers) // 3

        rastr = np.array(line_numbers[:length])
        delta_plus = np.array(line_numbers[length:2*length])
        delta_minus = np.array(line_numbers[2*length:3*length])

        img_final = np.array([rastr, delta_plus, delta_minus]).transpose().reshape(N, M, 3)
 

        plt.imshow(img_final)

        fname = "tmp-%d.png" % i
        print('Saving frame', fname)
        plt.savefig(fname)
        files.append(fname)
    return True

with open('passive.rec') as file:
    str = file.readline()
    n = len(str)
    if not process(file, n, n) and not process(file, n + 1, n):
        print("Corrupt rec file!")
        exit(-1)

print('Making movie animation.mpg - this may take a while')
subprocess.call(['ffmpeg', '-framerate', '8', '-start_number', f'{tactStart}', '-i', 'tmp-%d.png', '-r', '30', '-pix_fmt', 'yuv420p', video_file])

for fname in files:
    os.remove(fname)

os.system(video_file)


