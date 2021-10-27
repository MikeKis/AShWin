#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Oct 27 11:20:55 2021

@author: mike
"""

import os
import matplotlib.pyplot as plt
import numpy as np
import subprocess

# importing movie py libraries
from moviepy.editor import VideoClip
from moviepy.video.io.bindings import mplfig_to_npimage

N = 20
M = 20

lines = None

with open('passive.rec') as file:
 # lines = file.read().splitlines()
 lines = file.readlines()
 print('Number of lines:', len(lines))


 
fig, ax = plt.subplots(figsize=(5, 5))


def make_frame(t):
 t = int(t * 10)
 print(t)
 line = lines[t][:-1]

 line_numbers = []
 for k, i in enumerate(line):
  if i == '.':
   line_numbers.append(0)
  else:
   line_numbers.append(255)

 length = len(line_numbers) // 3
 # print(length, len(line))
 # print(line)

 rastr = np.array(line_numbers[:length]).reshape(N, M)
 delta_minus = np.array(line_numbers[length:2*length]).reshape(N, M)
 delta_plus = np.array(line_numbers[2*length:3*length]).reshape(N, M)

 img_final = np.array([rastr, delta_minus, delta_plus]).reshape(N, M, 3)

 return img_final
 # plt.imshow(img_final)

 # fname = f'_tmp{ln}.png'
 # print('Saving frame', fname)
 # plt.savefig(fname)
 # files.append(fname)


# creating animation
animation = VideoClip(make_frame, duration=20)
 
# displaying animation with auto play and looping
animation.ipython_display(fps=10, loop = True, autoplay = True)

# cleanup
'''for fname in files:
    os.remove(fname)'''