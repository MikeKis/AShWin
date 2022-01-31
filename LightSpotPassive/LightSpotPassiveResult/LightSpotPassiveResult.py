# coding=utf-8

import csv
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.colors import NoNorm
import math
import os
import subprocess
from collections import namedtuple
from matplotlib.widgets import Slider
from ipywidgets import interact, interactive, fixed, interact_manual
import ipywidgets as widgets
import copy
import sys
import numpy as nmp
from readpro import readpro

video_file = 'StableWeights.mp4'

id = 3003
nreceptors = 1200
ncopies = 3
nneu = 89
show_protocol = False


if show_protocol:

    start_tact = 2600000
    end_tact = 3000000

    pro = readpro(R"spikes.%d.txt" % id, start_tact, end_tact - start_tact, nneu)

    print("protocol read")

    phase_points = [[], [], [], []]
    with open("passive.csv", "r") as fil:
        csr = csv.reader(fil)
        tact = 0
        for row in csr:
            if tact >= start_tact:
                for i in range(4):
                    phase_points[i].append(float(row[i]))
            tact += 1

    print("phase points read")

    fig1, ax1 = plt.subplots()

    def DrawPhasePoints(neurons):
        X = []
        Y = []
        U = []
        V = []
        C = []
        cval = 0.5
        cstep = 1.001
        vrel = 30
        for i in neurons:
            for j in i:
                C.append(cval)
                X.append(phase_points[0][j])
                Y.append(phase_points[1][j])
                U.append(phase_points[2][j] * vrel)
                V.append(phase_points[3][j] * vrel)
            cval += cstep
            if cval > 1.:
                cstep *= 0.5
                cval = cstep * 0.5
        ax1.clear()
        ax1.set_xlim(-1., 1)
        ax1.set_ylim(-1., 1)
        ax1.set_box_aspect(1)
        ax1.grid(True)
        norm = NoNorm()
        Q = ax1.quiver(X, Y, U, V, C, units='x', cmap = 'hsv', norm = norm)

    DrawPhasePoints(pro[:1])

    axcolor = 'lightgoldenrodyellow'
    axfreq = plt.axes([0.25, 0.01, 0.65, 0.02], facecolor=axcolor)

    neuron_slider = Slider(ax = axfreq, label = 'neurons', valmin = 1, valmax = nneu - 1, valinit = 1, valfmt = "%d", valstep = 1)

    def update_sli(val):
        DrawPhasePoints(pro[:val])

    neuron_slider.on_changed(update_sli)

    plt.show()

    exit(0)

p = subprocess.Popen(["ArNIResults", "%d" % id])
p.communicate()

file = R"%d.secint.csv" % id
sec = []
CharTime = []
ThrDecayT = []
ThresholdExcessIncrementMultiplier = []
AbsRefT = []
WINC = []
RelWDec = []
Inh = []
secint = []
SectionIntensity = namedtuple('SectionIntensity', ['sec', 'tact', 'relfre'])
secsiz = []
SectionSize = namedtuple('SectionSize', ['sec', 'tact', 'size'])
lin = []
Link = namedtuple('Link', 'tact,neu,inh,indsyn,minW,maxW,Is_pla,delay,src,effw,W,WSTDP,LastSpike')
neusec = []
maxsec = 0

with open(file, "r") as fil:
    csr = csv.reader(fil)
    for row in csr:
        r = float(row[2])
        if r >= 0:
            secint.append(SectionIntensity(int(row[1]), int(row[0]), r))
            maxsec = max(maxsec, secint[-1].sec)        
        else:
            secsiz.append(SectionSize(int(row[1]), int(row[0]), -int(row[2])))

plt.figure(1)
for s in range(maxsec + 1):
    x = [t.tact for t in secint if t.sec == s]
    y = [t.relfre for t in secint if t.sec == s]
    plt.plot(x, y, label = "%d" % (s,), linewidth = 1)
leg = plt.legend(loc = 'best', ncol = 2, mode = "expand", fancybox = True)
leg.get_frame().set_alpha(0.5)
plt.title('Section relative activity')

file = R"%d.stable.csv" % id
nstable = {}
ndstable = {}
tact = [0]

with open(file, "r") as fil:
    csr = csv.reader(fil)
    for row in csr:
        curtact = int(row[0])
        if curtact != tact[-1]:
            tact.append(curtact)
        sec = int(row[1])
        nstable.setdefault(sec, [0] * (len(tact) - 2))
        ndstable.setdefault(sec, [0] * (len(tact) - 2))
        nstable[sec].append(int(row[2]))        
        ndstable[sec].append(int(row[3]))        

file = R"%d.stable_links.csv" % id
receptor = [[] for i in range(len(tact))]
neuron = [[] for i in range(len(tact))]
weights = nmp.zeros((len(tact), ncopies, nneu, 20, 20, 3))
with open(file, "r") as fil:
    csr = csv.reader(fil)
    for row in csr:
        rec = int(row[1])
        neu = int(row[2])
        if rec >= 0 and rec < nreceptors and neu < nneu * ncopies:
            indtact = tact.index(int(row[0]))
            receptor[indtact].append(rec)        
            neuron[indtact].append(neu)
            section = int(int(row[2]) / nneu)
            channel = int(rec / (20 * 20)) 
            pixelno = rec - channel * 20 * 20
            picrow = int(pixelno / 20)
            weights[indtact][section][int(row[2]) - section * nneu][picrow][pixelno - picrow * 20][channel] = 1

def DrawStableWeights():
    ax1 = plt.gca()
    for d in nstable.keys():
        ax1.plot(tact[1:], nstable[d], label = d, linewidth = 1)
    ax1.set_xlabel('tact')
    ax1.set_ylabel('n stable links')
    leg = plt.legend(loc = 'best', ncol = 2, mode = "expand", shadow = True, fancybox = True)
    leg.get_frame().set_alpha(0.5)

def DrawStableWeightsDif():
    ax1 = plt.gca()
    for d in nstable.keys():
        ax1.plot(tact[1:], ndstable[d], label = d, linewidth = 1)
    ax1.set_xlabel('tact')
    ax1.set_ylabel('stable link diff')
    leg = plt.legend(loc = 'best', ncol = 2, mode = "expand", shadow = True, fancybox = True)
    leg.get_frame().set_alpha(0.5)

plt.figure(2)
DrawStableWeights()

plt.figure(3)
DrawStableWeightsDif()


fig, ax = plt.subplots(ncopies, nneu, figsize=(42, 10))

def DrawStableWeigtsDVS(tac):
    indtact = tact.index(tac)
    for section in range(ncopies):
        for neuron in range(nneu):
            ax[section][neuron].axis('off')
            ax[section][neuron].imshow(weights[indtact][section][neuron])

files = []

def process():
    j = 1
    for i in range(tact[1], max(tact) + 1, tact[1]):
        DrawStableWeigtsDVS(i)
        fname = "tmp-%04d.png" % j
        print('Saving frame', fname)
        plt.savefig(fname)
        files.append(fname)
        j += 1

if 'video_file' not in globals():

    DrawStableWeigtsDVS(tact[1])

    axcolor = 'lightgoldenrodyellow'
    axfreq = plt.axes([0.25, 0.1, 0.65, 0.03], facecolor=axcolor)

    time_slider = Slider(ax = axfreq, label = 'tact', valmin = tact[1], valmax = max(tact), valinit = tact[1], valfmt = "%d", valstep = tact[1])

    def update_sli(val):
        tac = int(time_slider.val / tact[1]) * tact[1]
        DrawStableWeigtsDVS(tac)

    time_slider.on_changed(update_sli)

    plt.show()

else:
    process()
    print('Making movie animation.mpg - this may take a while')
    subprocess.call(['ffmpeg', '-framerate', '8', '-i', 'tmp-%04d.png', '-r', '30', '-pix_fmt', 'yuv420p', video_file])

    for fname in files:
        os.remove(fname)

    os.system(video_file)
    plt.show()

