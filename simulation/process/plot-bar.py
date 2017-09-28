# -*- coding: utf-8 -*-
from __future__ import unicode_literals
import matplotlib
matplotlib.use("Agg")
import os
import math
import matplotlib.pyplot as plt
#from operator import add, div, mul
import numpy as np

plt.style.use('ggplot')

def smallest_larger_square(x):
  return math.ceil(math.sqrt(x)) * math.ceil(math.sqrt(x))

def plot():

  title = "1o. nó sem energia / particionamento da rede"
  yLabel = "Tempo em segundos"

  xValues = np.arange(6)# [1, 2, 3, 4, 5, 6]

  xValues = [1, 2, 3, 4, 5, 6]

  # first dead node
  yValues = [0,0,0,0,0,0]
  sd = [0,0,0,0,0,0]

  #lan partitioned
  y2Values = [0,0,0,0,0,0]
  sd2 = [0,0,0,0,0,0]

  xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
  figName = 'lan_partitioned_node_energyless.eps'

  fig = plt.figure(figsize=(8, 5))

  plt.grid(b=True, which='major', color='gray', linestyle='--', lw=0.1)
  legends = []

  plt.xlim(0,7)
  plt.ylim(1000,4500)
  
  graph1=plt.errorbar(xValues, yValues, yerr=sd, fmt="-o", linestyle='--') # linestyle='none', marker='o')
  graph2=plt.errorbar(xValues, y2Values, yerr=sd2, fmt="-o", linestyle='-.') # linestyle='none', marker='o')

  plt.legend([graph1, graph2], ['1o. nó sem energia', 'particionamento da rede'])

  plt.xticks(xValues, xLegends)

  plt.ylabel(yLabel)

  plt.title(title)

  fig.autofmt_xdate()

  plt.ylabel(yLabel)

  fig.autofmt_xdate()

  plt.savefig(figName, format='eps', dpi=72, bbox_inches='tight')
  #plt.show()

def plot2():


  title = "Relação pacotes de dados / controle"
  yLabel = "Pacotes enviados / recebidos"
  xValues = np.arange(6)# [1, 2, 3, 4, 5, 6]

  data_packets_sent = [0,0,0,0,0,0]
  data_packets_delivered = [0,0,0,0,0,0]
  control_packets_sent = [4857.000000,0,0,0,0,0]
  control_packets_delivered = [4857.000000,0,0,0,0,0]

  xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
  figName = 'data_control.eps'

  fig, ax = plt.subplots()

  width = 0.35         # the width of the bars

  p1 = (ax.bar(xValues, data_packets_sent, width, color='#aaaaaa', bottom=0, edgecolor='black'), \
        ax.bar(xValues +0.04, data_packets_delivered, width -0.08, bottom=0, color='#888888', edgecolor='black'))#,  hatch='#'

  p2 = (ax.bar(xValues + width +0.04, control_packets_sent, width, color='#444444',  bottom=0, edgecolor='black'), \
        ax.bar(xValues + width +0.04 +0.04, control_packets_delivered, width -0.08, bottom=0, color='#666666', edgecolor='black'))#,  hatch='#'

  ax.set_title(title)
  ax.set_xticks(xValues + width / 2)
  ax.set_xticklabels(xLegends)

  ax.legend((p1[0], p1[1], p2[0], p2[1]), ('dados enviados', 'dados recebidos', 'controles enviados', 'controles recebidos'))

  ax.autoscale_view()

  plt.ylabel(yLabel)


  plt.ylim(0,9000)

  fig.autofmt_xdate()

  plt.savefig(figName, format='eps', dpi=72, bbox_inches='tight')


plot();
plot2();