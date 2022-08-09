import csv
import matplotlib
import matplotlib.pyplot as plt
from collections import namedtuple
from matplotlib.widgets import Slider
import numpy as np
import statistics
import bisect
import math
import os
import subprocess
from matplotlib.widgets import Slider
from ipywidgets import interact, interactive, fixed, interact_manual
import ipywidgets as widgets

video_file = 'StableWeights.mp4'

id = 2002
nreceptors = 1200
ndirections = 4
nneu = 10
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

file = R"%d.stable_links.csv" % id
receptor = [[] for i in range(len(tact))]
neuron = [[] for i in range(len(tact))]
weights = np.zeros((len(tact), ndirections, nneu, 20, 20, 3))
weightsind = [{} for i in tact]
with open(file, "r") as fil:
    csr = csv.reader(fil)
    for row in csr:
        rec = int(row[1])
        neu = int(row[2])
        if rec >= 0 and rec < nreceptors:
            indtact = tact.index(int(row[0]))
            channel = int(rec / (20 * 20)) 
            pixelno = rec - channel * 20 * 20
            picrow = int(pixelno / 20)
            if  neu < nneu * ndirections:
                receptor[indtact].append(rec)        
                neuron[indtact].append(neu)
                section = int(int(row[2]) / nneu)
                weights[indtact][section][int(row[2]) - section * nneu][picrow][pixelno - picrow * 20][channel] = 1
            weightsind[indtact].setdefault(neu, np.zeros((20, 20, 3)))
            weightsind[indtact][neu][picrow][pixelno - picrow * 20][channel] = 1

fig, ax = plt.subplots(ndirections, nneu, figsize=(42, 10))

def DrawStableWeigtsDVS(tac):
    indtact = tact.index(tac)
    for section in range(ndirections):
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

process()
print('Making movie animation.mpg - this may take a while')
subprocess.call(['ffmpeg', '-framerate', '8', '-i', 'tmp-%04d.png', '-r', '30', '-pix_fmt', 'yuv420p', video_file])

for fname in files:
    os.remove(fname)

os.system(video_file)

plt.show()


file = "monitoring.2001.csv"
ReceptorSectionBoundaries = [1200, 1201, 1202]

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
lin = []
Link = namedtuple('Link', 'tact,neu,type,indsyn,minW,maxW,Is_pla,delay,src,effw,basew,W')
neu = []
Neuron = namedtuple('Neuron', 'tact,neu,meaning')
tact = []
neusec = []
migrations = []

# It is guaranteed that all records are ordered by tact

with open(file, newline = '') as fil:
    bHeader = True
    csr = csv.reader(fil)
    for row in csr:
        if bHeader:
            bHeader = False
        elif row[0] == "secsta":
            sec.append(float(float(row[1])))
            CharTime.append(float(row[2]))
            ThrDecayT.append(float(row[3]))
            ThresholdExcessIncrementMultiplier.append(row[4])
            AbsRefT.append(float(row[5]))
            WINC.append(float(row[6]))
            RelWDec.append(float(row[7]))
            Inh.append(float(row[8]))
        elif row[0] == "secint":
            if float(row[2]) >= 0:
                secint.append(SectionIntensity(float(row[2]), float(row[1]), float(row[3])))
        elif row[0] == "lin":
            lin.append(Link(float(row[1]), int(row[2]), float(row[3]), float(row[4]), float(row[5]), float(row[6]), float(row[7]) != 0, float(row[8]), int(row[9]), float(row[10]), float(row[11]), float(row[12])))
        elif row[0] == "neu->sec":
            tac = int(row[1])
            n = int(row[2])
            s = int(row[3])

            # It is assumed that the 1st neu->sec section is ordered ny neu

            if len(tact) == 0 or tac != tact[-1]:
                tact.append(tac)
                neusec.append([] if tac == 0 else [-1 for i in range(len(neusec[0]))])
                migrations.append(0)
            if tac == 0:
                neusec[-1].append(s)
            else:
                neusec[-1][n] = s
        elif row[0] == "neudyn":
            migrations[-1] += 1
        elif row[0] == "neu":
            neu.append(Neuron(float(row[1]), int(row[2]), row[12]))

indLsection = 0

Lneu = [n for n in range(len(neusec[0])) if neusec[0][n] == indLsection]


MeaningDynamics = []
for t in tact:
    print('tact ', t)
    MeaningDynamics.append([])
    for i in range(len(Lneu)):
        print('\tL neuron ', i)
        strong = [-1 - l.src for l in lin if l.tact == t and l.neu == Lneu[i] and l.W > 30]
        mea = [n.meaning for n in neu if n.tact == t and n.neu in strong]
        print(mea)
        MeaningDynamics[-1].append(mea)

final = MeaningDynamics[-1]

def ParseReceptiveField(mea, type = ''):
    if type == '':
        return [ParseReceptiveField(mea, type = 'D'), ParseReceptiveField(mea, type = 'A'), ParseReceptiveField(mea, type = 'I')]
    ind = mea.find(type)
    if ind == -1:
        return []
    else:
        indbeg = mea.find('(', ind) + 1
        indend = mea.find(')', ind)
        str = mea[indbeg:indend]
        lstr = str.split(',')
        return [float(lstr[0]), float(lstr[1])]

fin = [[ParseReceptiveField(mea) for mea in m if mea.find('-') == -1] for m in final]

print(fin)

fin = fin[9:]

c = ['r', 'g', 'b']
fig, ax = plt.subplots(subplot_kw=dict(aspect='equal'))
ax.set(xlim=(0, 20), ylim=(0, 20), xticks=[], yticks=[])
for i in range (len(fin)):
    for a in fin[i]:
        if len(a[0]) == 0 and len(a[2]) == 0:
            circle = plt.Circle((a[1][0], a[1][1]), 0.2, color = c[i])
            ax.add_patch(circle)
        else:
            if len(a[0]) > 0 and len(a[1]) > 0:
                arr = plt.Arrow(a[0][0], a[0][1], a[1][0] - a[0][0], a[1][1] - a[0][1], width=0.1, ec = c[i], fc = c[i])
                ax.add_patch(arr)
            if len(a[2]) > 0 and len(a[1]) > 0:
                arr = plt.Arrow(a[1][0], a[1][1], a[2][0] - a[1][0], a[2][1] - a[1][1], width=0.1, ec = c[i], fc = c[i])
                ax.add_patch(arr)
plt.show()
