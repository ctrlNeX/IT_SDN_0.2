# -*- coding: utf-8 -*-
from __future__ import unicode_literals
import os
import math
import numpy
import matplotlib.pyplot as plt
from operator import add, div, mul


def smallest_larger_square(x):
	return math.ceil(math.sqrt(x)) * math.ceil(math.sqrt(x))

# plot_files = (
# 	("/home/dori/Desktop/ETX_Energy_Simulation_Results/plot.csv","Plot Graph"),
# )

def plot(title, yLabel, xValues, yValues, sd, xLegends, figName, ylim0, ylim1):
	#############################################
	####### Plotting delivery rate  #############
	#############################################
	# Basic graph configs
	fig = plt.figure(figsize=(8, 5))
	# ax = fig.add_subplot(1, 1, 1)
	plt.grid(b=True, which='major', color='gray', linestyle='--', lw=0.1)
	legends = []

	# for fname, plotname in plot_files:
	# 	f = file(fname)
	# 	# removes header
	# 	f.readline()
	# 	print fname
	# 	legends += [plotname]

	# 	xs, ys, es = [], [], []



		# for line in f:
		# 	y,e = map(float, line.strip().split(";")[1:])
		# 	x = line.strip().split(";")[0]
		# 	xs += [x]
		# 	ys += [y]
		# 	es += [e]
		# 	print x,y,e
		
		# xs = numpy.array(xs)
		# ys = numpy.array(ys)
		# xs=[1,2,3,4,5,6]
		# ys=[5201.2, 4535.2222222222, 5153.7777777778, 4885, 5726.7777777778, 4509.3333333333]

		
		# Plot results
		# if len(xs) == 2:
		# 	plt.errorbar(xs, ys, es, ls="", marker="o")
		# else:
		# 	plt.errorbar(xs, ys, es)#, ls=lss[rc%len(lss)], color=colors[rc%len(colors)], marker=markers[rc%len(markers)])
	plt.xlim(0,7)
	plt.ylim(ylim0,ylim1)
	plt.errorbar(xValues, yValues, yerr=sd, fmt="-o") # linestyle='none', marker='o')
	# plt.plot(xs, ys)

	plt.xticks(xValues, xLegends)
	# locs, labels=plt.xticks()
	# plt.setp(labels, rotation=45)
	plt.ylabel(yLabel)

	plt.title(title)

	fig.autofmt_xdate()
		# fig['layout']['xaxis1'].update({'ticktext': ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100'],
	 #                                       'tickfont': {'size': 14, 'family':'Courier New, monospace'},
	 #                                       'tickangle': 60
	 #                                       })
		# plt.legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
		# axis(1, at=x, labels=xt)
		# plt.show()
		# setting axis limits
		# plt.margins(0.05)
		# axes = plt.gca()
		# axes.set_xlim([1.8,64*1.1])

		# Saving graph
		# f.close()
	# lgd = plt.legend(legends, loc='best', prop={'size':8})
		# plt.xlabel("Number of nodes")
	# plt.ylabel("Delivery rate [%]")
	plt.savefig(figName, format='eps', dpi=72, bbox_inches='tight') #, bbox_extra_artists=(lgd,) if lgd != None else None)

# title = "#1 Death Node"
# yLabel = "Time in seconds"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 4287.500000,3754.300000,4240.600000,3601.800000,4515.700000,4263.900000]
# sd = [ 964.017453,1323.392084,1470.634298,1148.873343,1357.698497,1268.838244]
# xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']
# figName = 'first_dead_node.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName);

# title = "Lan Partitioned"
# yLabel = "Time in seconds"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 5389.000000,5529.600000,5810.700000,5271.800000,5851.900000,6050.800000]
# sd = [ 1284.260721,714.217781,991.656397,466.924362,285.527389,277.065263]
# xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']
# figName = 'lan_partitioned.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName);

# title = "Data Sent"
# yLabel = "#Packets"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [1158.3,
# 976.2222222222,
# 1146.8888888889,
# 1164.7,
# 1220.1,
# 1104.2
# ]
# sd = [74.130532236,
# 196.8049361373,
# 131.465252206,
# 90.9355724543,
# 51.8859424353,
# 122.8483220626
# ]
# xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']
# figName = 'data_sent.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName);

# title = "Data Received"
# yLabel = "#Packets"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [1025.2,
# 781.1,
# 976.7,
# 966.8,
# 1047.4,
# 814.4
# ] 
# sd = [97.379380454,
# 238.1312806634,
# 207.7073044493,
# 139.6094668194,
# 197.1668816977,
# 237.0991187972
# ]
# xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']
# figName = 'data_received.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName);

# title = "Changing Routes"
# yLabel = "Number of Routes"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 1.200000,2.600000,2.900000,4.300000,5.200000,5.600000]
# sd = [ .600000,.800000,1.513274,.900000,2.135415,2.059126]
# xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']
# figName = 'changing_routes.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName);

# title = "Data Delivery Rate"
# yLabel = "Delivery Rate [%]"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 99.385400,99.430000,98.619600,99.016400,94.444100,96.429600]
# sd = [ .921900,1.170400,3.209300,2.095200,9.993900,5.588300]
# xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']
# figName = 'delivery_rate.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName);

# title = "Control Overhead"
# yLabel = "Control / Total [%]"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 20.875500,36.197100,39.807300,39.352400,31.310700,35.995800]
# sd = [ 6.766800,11.613300,9.733900,6.449800,4.049600,5.186500]
# xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']
# figName = 'control_overhead.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName);





#Resultado usando TINYSDN sem modificação

# title = "1o. nó morto"
# yLabel = "Tempo em segundos"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 3777.300000,3748.500000,3701.100000,3601.800000,4234.400000,4041.300000]
# sd = [ 1562.354060,1317.587207,1880.614232,1148.873343,1619.002112,1431.186364]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'first_dead_node.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 3500, 4500);

# title = "Particionamento da Rede"
# yLabel = "Tempo em segundos"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 4861.600000,5523.800000,5769.800000,5271.800000,5412.500000,5629.000000]
# sd = [ 1759.480843,717.892721,984.983329,466.924362,1286.372282,1220.882631]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'lan_partitioned.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 3000, 7000);

# title = "Mudanças de Rotas"
# yLabel = "Número de Rotas"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 1.400000,2.500000,2.600000,4.300000,5.500000,5.700000]
# sd = [ .800000,.921954,1.743559,.900000,2.156385,2.002498]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'changing_routes.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 10);

# title = "Taxa de Entrega de Dados"
# yLabel = "Taxa de Entrega [%]"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 89.393200,89.447500,88.659500,99.016400,94.749000,97.965300]
# sd = [ 29.811400,29.812200,29.725000,2.095200,9.175500,3.745600]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'delivery_rate.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 90, 102);

# title = "Sobrecarga de Controle"
# yLabel = "Controle / Total [%]"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 63.874400,63.787000,70.685300,65.149200,63.592700,64.210000]
# sd = [ 4.558500,3.235700,4.310400,1.811000,1.523100,1.827500]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'control_overhead.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 100);








#Resultado usando TINYSDN com modificação
# title = "1o. nó morto"
# yLabel = "Tempo em segundos"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 5775.500000,5711.100000,5998.400000,6162.400000,6560.600000,6566.300000]
# sd = [ 403.715308,873.253851,816.356319,590.572637,536.792362,430.497862]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'first_dead_node.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 5500, 7500);

# title = "Particionamento da Rede"
# yLabel = "Tempo em segundos"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 7088.300000,7263.900000,7234.600000,7117.400000,7269.600000,7129.000000]
# sd = [ 271.614451,80.734688,70.050267,166.088048,45.314898,79.619093]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'lan_partitioned.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 6800, 8000);

# title = "Mudanças de Rotas"
# yLabel = "Número de Rotas"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 1.000000,2.900000,4.600000,4.500000,5.300000,5.300000]
# sd = [ 0,.943398,1.019803,.806225,1.486606,1.552417]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'changing_routes.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 10);

# title = "Taxa de Entrega de Dados"
# yLabel = "Taxa de Entrega [%]"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 99.813800,99.865200,99.843700,99.827300,99.878500,99.869100]
# sd = [ 0,0,0,0,0,0]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'delivery_rate.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 90, 102);

# title = "Sobrecarga de Controle"
# yLabel = "Controle / Total [%]"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 33.307000,37.125600,39.271100,38.207700,37.597900,39.122700]
# sd = [ .574400,1.135700,2.649500,.648000,.300000,.774500]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'control_overhead.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 100);











# title = "1o. nó morto"
# yLabel = "Tempo em segundos"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 5775.500000,5711.100000,5998.400000,6162.400000,6560.600000,6566.300000]
# sd = [ 403.715308,873.253851,816.356319,590.572637,536.792362,430.497862]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'first_dead_node.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 5500, 7500);

# title = "Particionamento da Rede"
# yLabel = "Tempo em segundos"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 7088.300000,7263.900000,7234.600000,7117.400000,7269.600000,7129.000000]
# sd = [ 271.614451,80.734688,70.050267,166.088048,45.314898,79.619093]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'lan_partitioned.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 6800, 8000);

# title = "Mudança de Rotas"
# yLabel = "Número de Rotas"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 1.000000,2.900000,4.600000,4.500000,5.300000,5.300000]
# sd = [ 0,.943398,1.019803,.806225,1.486606,1.552417]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'changing_routes.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 10);

# title = "Taxa de Entrega de Dados"
# yLabel = "Taxa de Entrega [%]"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 99.813800,99.865200,99.843700,99.827300,99.878500,99.869100]
# sd = [ 0,0,0,0,0,0]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'delivery_rate.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 90, 102);

# title = "Sobrecarga de Controle"
# yLabel = "Controle / Total [%]"
# xValues = [1, 2, 3, 4, 5, 6]
# yValues = [ 33.307000,37.125600,39.271100,38.207700,37.597900,39.122700]
# sd = [ .574400,1.135700,2.649500,.648000,.300000,.774500]
# xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
# figName = 'control_overhead.eps'

# plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 100);





# title = "Pacotes de Controle"
# yValues = [ 723.300000,782.700000,880.400000,886.000000,876.900000,958.900000]
# sd = [ 60.144908,47.931305,140.177886,81.399017,65.561345,67.354955]
# title = "Pacotes de Dados"
# yValues = [ 1443.600000,1472.600000,1475.400000,1460.300000,1473.900000,1456.100000]
# sd = [ 48.077437,32.041223,13.200000,35.000142,37.532519,15.927648]





title = "1o. nó morto"
yLabel = "Tempo em segundos"
xValues = [1, 2, 3, 4, 5, 6]
yValues = [ 0,0,0,0,0,2522.200000]
sd = [ 0,0,0,0,0,158.233877]
xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
figName = 'first_dead_node.eps'

plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 5500, 7500);

title = "Particionamento da Rede"
yLabel = "Tempo em segundos"
xValues = [1, 2, 3, 4, 5, 6]
yValues = [ 0,0,0,0,0,3390.800000]
sd = [ 0,0,0,0,0,401.534755]
xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
figName = 'lan_partitioned.eps'

plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 6800, 8000);

title = "Mudanças de Rotas"
yLabel = "Número de Rotas"
xValues = [1, 2, 3, 4, 5, 6]
yValues = [ 0,0,0,0,0,0]
sd = [ 0,0,0,0,0,0]
xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
figName = 'changing_routes.eps'

plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 10);

title = "Taxa de Entrega de Dados"
yLabel = "Taxa de Entrega [%]"
xValues = [1, 2, 3, 4, 5, 6]
yValues = [ 0,0,0,0,0,61.755000]
sd = [ 0,0,0,0,0,6.608300]
xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
figName = 'delivery_rate.eps'

plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 90, 102);

title = "Sobrecarga de Controle"
yLabel = "Controle / Total [%]"
xValues = [1, 2, 3, 4, 5, 6]
yValues = [ 0,0,0,0,0,47.872800]
sd = [ 0,0,0,0,0,.632400]
xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']
figName = 'control_overhead.eps'


plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 100);

