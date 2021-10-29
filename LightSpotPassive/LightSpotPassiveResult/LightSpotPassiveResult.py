# coding=utf-8

import csv
import matplotlib as mpl
import matplotlib.pyplot as plt
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

id = 2000
nreceptors = 1200
ncopies = 3
nneu = 60

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
nstable = []
ndstable = []
tact = [0]

with open(file, "r") as fil:
    csr = csv.reader(fil)
    for row in csr:
        tact.append(int(row[0]))
        nstable.append(int(row[1]))        
        ndstable.append(int(row[2]))

#file = R"%d.stable_links.csv" % id
#receptor = [[] for i in range(len(tact))]
#neuron = [[] for i in range(len(tact))]
#MNISTweights = nmp.zeros((len(tact), ncopies, nneu, 28, 28))
#with open(file, "r") as fil:
#    csr = csv.reader(fil)
#    for row in csr:
#        rec = int(row[1])
#        if rec >= 0 and rec < nreceptors:
#            indtact = tact.index(int(row[0]))
#            receptor[indtact].append(rec)        
#            neuron[indtact].append(int(row[2]))
#            section = int(int(row[2]) / nneu)
#            MNISTrow = int(rec / 28)
#            MNISTweights[indtact][section][int(row[2]) - section * nneu][MNISTrow][rec - MNISTrow * 28] = 1

def DrawStableWeights():
    ax1 = plt.gca()

    color = 'tab:red'
    ax1.set_xlabel('tact')
    ax1.set_ylabel('n stable links', color=color)
    ax1.plot(tact[1:], nstable, linewidth = 0.3, color=color)
    ax1.tick_params(axis='y', labelcolor=color)

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

    color = 'tab:blue'
    ax2.set_ylabel('stable link diff', color=color)  # we already handled the x-label with ax1
    ax2.plot(tact[1:], ndstable, linewidth = 0.3, color=color)
    ax2.tick_params(axis='y', labelcolor=color)

    plt.title('Weight change dynamics')

plt.figure(2)
DrawStableWeights()

#fig, ax = plt.subplots(ncopies, nneu, figsize=(42, 4))

#def DrawStableWeigtsMNIST(tac):
#    indtact = tact.index(tac)
#    for section in range(ncopies):
#        for neuron in range(nneu):
#            ax[section][neuron].axis('off')
#            ax[section][neuron].imshow(MNISTweights[indtact][section][neuron], cmap = 'binary')

#DrawStableWeigtsMNIST(tact[1])

#axcolor = 'lightgoldenrodyellow'
#axfreq = plt.axes([0.25, 0.1, 0.65, 0.03], facecolor=axcolor)

#time_slider = Slider(ax = axfreq, label = 'tact', valmin = tact[1], valmax = max(tact), valinit = tact[1], valfmt = "%d", valstep = tact[1])

#def update_sli(val):
#    tac = int(time_slider.val / tact[1]) * tact[1]
#    DrawStableWeigtsMNIST(tac)

#time_slider.on_changed(update_sli)

plt.show()
