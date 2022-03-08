import csv
import matplotlib
import matplotlib.pyplot as plt
from collections import namedtuple
from matplotlib.widgets import Slider
import numpy as np
import statistics
import bisect

file = "monitoring.3005.csv"
ReceptorSectionBoundaries = []

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

indLsection = 1

Lneu = [n for n in range(len(neusec[0])) if neusec[0][n] == indLsection]

for t in tact:
    print('tact ', t)
    for i in range(len(Lneu)):
        print('\tL neuron ', i)
        strong = [-1 - l.src for l in lin if l.tact == t and l.neu == Lneu[i] and l.W > 30]
        mea = [n.meaning for n in neu if n.tact == t and n.neu in strong]
        print(mea)