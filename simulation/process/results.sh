#!/bin/bash

set -o errtrace

yvalues_first_dead_node=""
sd_first_dead_node=""

yvalues_lan_partitioned=""
sd_lan_partitioned=""	

yvalues_counter_routes=""
sd_counter_routes=""	

yvalues_delivery_rate_data=""
sd_delivery_rate_data=""	

yvalues_control_overhead=""
sd_control_overhead=""	

files=(ETX_100_ENERGY_0.txt ETX_80_ENERGY_20.txt ETX_60_ENERGY_40.txt ETX_40_ENERGY_60.txt ETX_20_ENERGY_80.txt ETX_0_ENERGY_100.txt)

for ((file = 0; file < ${#files[@]}; file++)); do

	lastValue=""
	report=""

	# SAVE CSV FILE
	# echo "#1 dead node;lan partioned;pacotes de dados enviados;pacote de dados recebidos;pacotes de controle enviados; rotas;" > result.csv

	r1=""
	r2=""
	r3=""
	r4=""
	r5=""
	r6=""

	counter=-1

	total_simulations=1

	sum_first_dead_node=0
	first_dead_node=""

	sum_received_data=0
	received_data=""

	sum_sent_data=0
	sent_data=""

	sum_sent_control=0
	sent_control=""

	sum_received_control=0
	received_control=""

	sum_lan_partitioned=0
	lan_partitioned=""

	sum_delivery_rate_data=0
	delivery_rate_data=""

	sum_control_overhead=0
	control_overhead=""

	sum_energy_consumed=0
	energy_consumed=""

	sum_counter_routes=0
	sim_counter_routes=0

	change_simulation=0


	while read a b c d e f g h i j k l m; do
		# echo "$a $b $c $d $e $f ## $g"
		if [ "$a" == "Simulation:" ]; then
			timer=0
			((counter++))
			if [ "$r6" != "" ]; then
				# SAVE CSV FILE
				# echo "${r1};${r2};${r3};${r4};${r5};${r6};" >> result.csv
				r6=""
			fi
		elif [ "$a" == "time" ]; then
			#if [ "$g" == "data" ]; then
				# r4="${l:1:${#l}-2}" #dados recebidos
				# r4=$((10#${r4}))

				# ((sum_received_data+=r4))
				# received_data[$counter]=$r4

				
			#else
				if [ $timer == "0" ]; then
					r1="${b:0:${#b}-1}" #tempo da morte do primeiro nó
					r1=$((10#${r1}))
					((sum_first_dead_node+=r1))
					first_dead_node[$counter]=$r1

#					echo "$file >>>>>> $r1"
				fi
				timer=$b
			#fi
			
			# echo "time"
		elif [ "$a" == "Control" ]; then
			r2="${timer:0:${#timer}-1}" #tempo em que a rede particionou
			((sum_lan_partitioned+=r2))
			lan_partitioned[$counter]=$r2

			r5="${d}" #total de pacotes de controle enviados
			r5=$((10#${r5}))
			((sum_sent_control+=r5))
			sent_control[$counter]=$r5

			r6="${f}" #total de pacotes de controle enviados
			r6=$((10#${r6}))
			((sum_received_control+=r6))
			received_control[$counter]=$r6			

		elif [ "$a" == "Data" ]; then
			r4="${f}" #dados recebidos
			r4=$((10#${r4}))

			((sum_received_data+=r4))
			received_data[$counter]=$r4

			r3="${d}" #total de pacotes de dados enviados
			r3=$((10#${r3}))
			((sum_sent_data+=r3))
			sent_data[$counter]=$r3

			control_overhead[$counter]=$(bc <<< "scale=6; ${sum_sent_control} / (${sum_sent_control} + ${sum_sent_data})")
			sum_control_overhead=$(bc <<< "scale=6;${control_overhead[$counter]} + ${sum_control_overhead}")

			tmp_delivery_rate_data=$(bc <<< "scale=6;${received_data[$counter]} / ${sent_data[$counter]}")
			
			sum_delivery_rate_data=$(bc <<< "scale=6;${tmp_delivery_rate_data} + ${sum_delivery_rate_data}")
			
			delivery_rate_data[$counter]=$tmp_delivery_rate_data			

		elif [ "$a" == "Energy" ]; then
			r7="${c}" #dados recebidos
			r7=$((10#${r7}))

			((sum_energy_consumed+=r7))
			energy_consumed[$counter]=$r7

		# elif [ "$a" == "[" ]; then

		# 	if [ $change_simulation -ne $counter ]; then
		# 		counter_routes[$change_simulation]=$sim_counter_routes
		# 		change_simulation=$counter
		# 		sim_counter_routes=0
		# 	fi
			
		# 	((sum_counter_routes++))
		# 	((sim_counter_routes++))
			
		# 	r6+="$a $b $c $d $e $f $g $h $i $j $k $l $m"
		fi		

	done < ${files[file]}

	#counter_routes[$change_simulation]=$sim_counter_routes
	#change_simulation=$counter
	#sim_counter_routes=0

	if [ $counter -gt -1 ]; then

		((total_simulations=counter+1))

	fi

	average_first_dead_node=$(bc <<< "scale=6;${sum_first_dead_node} / ${total_simulations}")
	average_received_data=$(bc <<< "scale=6;${sum_received_data} / ${total_simulations}")
	average_sent_control=$(bc <<< "scale=6;${sum_sent_control} / ${total_simulations}")
	average_received_control=$(bc <<< "scale=6;${sum_received_control} / ${total_simulations}")
	average_sent_data=$(bc <<< "scale=6;${sum_sent_data} / ${total_simulations}")
	average_lan_partitioned=$(bc <<< "scale=6;${sum_lan_partitioned} / ${total_simulations}")
	average_delivery_rate_data=$(bc <<< "scale=6;${sum_delivery_rate_data} / ${total_simulations}")
	#average_counter_routes=$(bc <<< "scale=6;${sum_counter_routes} / ${total_simulations}")
	average_control_overhead=$(bc <<< "scale=6;${sum_control_overhead} / ${total_simulations}")
	average_energy_consumed=$(bc <<< "scale=6;${sum_energy_consumed} / ${total_simulations}")

	# average_counter_routes=$(bc <<< "scale=6;${sum_counter_routes} / ${total_simulations}")
	# echo "AVERAGE ${average_first_dead_node}"

	dif_average_first_dead_node=0
	dif_average_received_data=0
	dif_average_sent_data=0
	dif_average_sent_control=0
	dif_average_received_control=0
	dif_average_lan_partitioned=0
	dif_average_delivery_rate_data=0
	dif_average_counter_routes=0
	dif_average_control_overhead=0
	dif_average_energy_consumed=0

	for ((k=0; k<=$counter; k++)); do
		# echo $(bc <<< "scale=6;(${first_dead_node[$k]} - ${average_first_dead_node})")
		dif_average_first_dead_node=$(bc <<< "scale=6;(${first_dead_node[$k]} - ${average_first_dead_node}) ^ 2 + ${dif_average_first_dead_node}")
		dif_average_received_data=$(bc <<< "scale=6;(${received_data[$k]} - ${average_received_data}) ^ 2 + ${dif_average_received_data}")
		dif_average_sent_data=$(bc <<< "scale=6;(${sent_data[$k]} - ${average_sent_data}) ^ 2 + ${dif_average_sent_data}")
		dif_average_sent_control=$(bc <<< "scale=6;(${sent_control[$k]} - ${average_sent_control}) ^ 2 + ${dif_average_sent_control}")
		dif_average_received_control=$(bc <<< "scale=6;(${received_control[$k]} - ${average_received_control}) ^ 2 + ${dif_average_received_control}")
		dif_average_lan_partitioned=$(bc <<< "scale=6;(${lan_partitioned[$k]} - ${average_lan_partitioned}) ^ 2 + ${dif_average_lan_partitioned}")
		dif_average_delivery_rate_data=$(bc <<< "scale=6;(${delivery_rate_data[$k]} - ${average_delivery_rate_data}) ^ 2 + ${dif_average_delivery_rate_data}")
		#dif_average_counter_routes=$(bc <<< "scale=6;(${counter_routes[$k]} - ${average_counter_routes}) ^ 2 + ${dif_average_counter_routes}")
		dif_average_control_overhead=$(bc <<< "scale=6;(${control_overhead[$k]} - ${average_control_overhead}) ^ 2 + ${dif_average_control_overhead}")
		dif_average_energy_consumed=$(bc <<< "scale=6;(${energy_consumed[$k]} - ${average_energy_consumed}) ^ 2 + ${dif_average_energy_consumed}")

		# ((sum_variance+=average_first_dead_node[$k]*average_first_dead_node[$k]))
		# echo "${dif_average_first_dead_node}"
	done

	deviation_first_dead_node=$(bc <<< "scale=6;sqrt(${dif_average_first_dead_node} / ${total_simulations})")
	deviation_received_data=$(bc <<< "scale=6;sqrt(${dif_average_received_data} / ${total_simulations})")
	deviation_sent_data=$(bc <<< "scale=6;sqrt(${dif_average_sent_data} / ${total_simulations})")
	deviation_sent_control=$(bc <<< "scale=6;sqrt(${dif_average_sent_control} / ${total_simulations})")
	deviation_received_control=$(bc <<< "scale=6;sqrt(${dif_average_received_control} / ${total_simulations})")
	deviation_lan_partitioned=$(bc <<< "scale=6;sqrt(${dif_average_lan_partitioned} / ${total_simulations})")
	deviation_delivery_rate_data=$(bc <<< "scale=6;sqrt(${dif_average_delivery_rate_data} / ${total_simulations})")
	#deviation_counter_routes=$(bc <<< "scale=6;sqrt(${dif_average_counter_routes} / ${total_simulations})")
	deviation_control_overhead=$(bc <<< "scale=6;sqrt(${dif_average_control_overhead} / ${total_simulations})")
	deviation_energy_consumed=$(bc <<< "scale=6;sqrt(${dif_average_energy_consumed} / ${total_simulations})")

	average_delivery_rate_data=$(bc <<< "scale=6; ${average_delivery_rate_data} * 100")
	deviation_delivery_rate_data=$(bc <<< "scale=6; ${deviation_delivery_rate_data} * 100")

	average_control_overhead=$(bc <<< "scale=6; ${average_control_overhead} * 100")
	deviation_control_overhead=$(bc <<< "scale=6; ${deviation_control_overhead} * 100")

	#deviation_energy_consumed=$(bc <<< "scale=6; ${deviation_energy_consumed} * 100")

	# SAVE CSV FILE
	# echo "${r1};${r2};${r3};${r4};${r5};${r6};" >> result.csv

	# echo "First Dead Node ${average_first_dead_node} ${deviation_first_dead_node}"	
	# echo ">>> ${average_first_dead_node}"
	yvalues_first_dead_node[$file]="${average_first_dead_node}"
	sd_first_dead_node[$file]="${deviation_first_dead_node}"

	# echo "Lan Patitioned ${average_lan_partitioned} ${deviation_lan_partitioned}"	
	yvalues_lan_partitioned[$file]="${average_lan_partitioned}"
	sd_lan_partitioned[$file]="${deviation_lan_partitioned}"	

	# echo "Sent Data ${average_sent_data} ${deviation_sent_data}"	
	# echo "Received Data ${average_received_data} ${deviation_received_data}"
	yvalues_received_data[$file]="${average_received_data}"
	sd_received_data[$file]="${deviation_received_data}"	

	# echo "Changing Routes ${average_counter_routes} ${deviation_counter_routes}"	
	yvalues_counter_routes[$file]="${average_counter_routes}"
	sd_counter_routes[$file]="${deviation_counter_routes}"	

	# echo "Delivery Rate Data ${average_delivery_rate_data} ${deviation_delivery_rate_data}"
	yvalues_delivery_rate_data[$file]="${average_delivery_rate_data}"
	sd_delivery_rate_data[$file]="${deviation_delivery_rate_data}"	

	# echo "Control Overhead ${average_control_overhead} ${deviation_control_overhead}"		
	yvalues_control_overhead[$file]="${average_control_overhead}"
	sd_control_overhead[$file]="${deviation_control_overhead}"	
	# echo "Sent Control ${average_sent_control} ${deviation_sent_control}"	

	yvalues_sent_control[$file]="${average_sent_control}"
	sd_sent_control[$file]="${deviation_sent_control}"	

	yvalues_received_control[$file]="${average_received_control}"
	sd_received_control[$file]="${deviation_received_control}"	

	yvalues_sent_data[$file]="${average_sent_data}"
	sd_sent_data[$file]="${deviation_sent_data}"	

	yvalues_energy_consumed[$file]="${average_energy_consumed}"
	sd_energy_consumed[$file]="${deviation_energy_consumed}"	

done


	IFS=","

	echo 'title = "1o. nó morto"'
	echo 'yLabel = "Tempo em segundos"'
	# echo 'title = "#1 Death Node"'
	# echo 'yLabel = "Time in seconds"'
	echo 'xValues = [1, 2, 3, 4, 5, 6]'
	echo -n "yValues = [ "
	echo -n "${yvalues_first_dead_node[*]}"
	echo "]"
	echo -n "sd = [ "
	echo -n "${sd_first_dead_node[*]}"
	echo "]"
	echo "xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']"
	# echo "xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']"
	echo "figName = 'first_dead_node.eps'"
	echo ""
	echo "plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 5500, 7500);"
	echo ""

	yvalues="  yValues = \[${yvalues_first_dead_node[*]}\]"

	sed -i.bak "s/^  yValues = \[.*/$yvalues/" plot-bar.py

	sdvalues="  sd = \[${sd_first_dead_node[*]}\]"

	sed -i.bak "s/^  sd = \[.*/$sdvalues/" plot-bar.py

	echo 'title = "Particionamento da Rede"'
	echo 'yLabel = "Tempo em segundos"'
	#echo 'title = "Lan Partitioned"'
	#echo 'yLabel = "Time in seconds"'
	echo 'xValues = [1, 2, 3, 4, 5, 6]'
	echo -n "yValues = [ "
	echo -n "${yvalues_lan_partitioned[*]}"
	echo "]"
	echo -n "sd = [ "
	echo -n "${sd_lan_partitioned[*]}"
	echo "]"	
	echo "xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']"
	# echo "xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']"
	echo "figName = 'lan_partitioned.eps'"
	echo ""
	echo "plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 6800, 8000);"
	echo ""

	yvalues="  y2Values = \[${yvalues_lan_partitioned[*]}\]"

	sed -i.bak "s/^  y2Values = \[.*/$yvalues/" plot-bar.py

	sdvalues="  sd2 = \[${sd_lan_partitioned[*]}\]"

	sed -i.bak "s/^  sd2 = \[.*/$sdvalues/" plot-bar.py


	echo 'title = "Energia Consumida da Rede"'
	echo 'yLabel = "Porcentagem de Energia da Rede"'
	#echo 'title = "Lan Partitioned"'
	#echo 'yLabel = "Time in seconds"'
	echo 'xValues = [1, 2, 3, 4, 5, 6]'
	echo -n "yValues = [ "
	echo -n "${yvalues_energy_consumed[*]}"
	echo "]"
	echo -n "sd = [ "
	echo -n "${sd_energy_consumed[*]}"
	echo "]"	
	echo "xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']"
	# echo "xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']"
	echo "figName = 'energy_consumed.eps'"
	echo ""
	echo "plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 110);"
	echo ""
	# echo 'title = "Mudanças de Rotas"'
	# echo 'yLabel = "Número de Rotas"'
	# #echo 'title = "Changing Routes"'
	# #echo 'yLabel = "Number of Routes"'	
	# echo 'xValues = [1, 2, 3, 4, 5, 6]'
	# echo -n "yValues = [ "
	# echo -n "${yvalues_counter_routes[*]}"
	# echo "]"
	# echo -n "sd = [ "
	# echo -n "${sd_counter_routes[*]}"
	# echo "]"
	# echo "xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']"
	# # echo "xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']"
	# echo "figName = 'changing_routes.eps'"
	# echo ""
	# echo "plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 10);"
	# echo ""

	# echo 'title = "Taxa de Entrega de Dados"'
	# echo 'yLabel = "Taxa de Entrega [%]"'
	# #echo 'title = "Data Delivery Rate"'
	# #echo 'yLabel = "Delivery Rate [%]"'
	# echo 'xValues = [1, 2, 3, 4, 5, 6]'
	# echo -n "yValues = [ "
	# echo -n "${yvalues_delivery_rate_data[*]}"
	# echo "]"
	# echo -n "sd = [ "
	# echo -n "${sd_delivery_rate_data[*]}"
	# echo "]"
	# echo "xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']"
	# # echo "xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']"
	# echo "figName = 'delivery_rate.eps'"
	# echo ""
	# echo "plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 90, 102);"
	# echo ""

	# echo 'title = "Sobrecarga de Controle"'
	# echo 'yLabel = "Controle / Total [%]"'
	# #echo 'title = "Control Overhead"'
	# #echo 'yLabel = "Control / Total [%]"'
	# echo 'xValues = [1, 2, 3, 4, 5, 6]'
	# echo -n "yValues = [ "
	# echo -n "${yvalues_control_overhead[*]}"
	# echo "]"
	# echo -n "sd = [ "
	# echo -n "${sd_control_overhead[*]}"
	# echo "]"
	# echo "xLegends = ['ETX_100%_Energia_0%', 'ETX_80%_Energia_20%', 'ETX_60%_Energia_40%', 'ETX_40%_Energia_60%', 'ETX_20%_Energia_80%', 'ETX_0%_Energia_100%']"
	# # echo "xLegends = ['ETX_100_Energy_0', 'ETX_80_Energy_20', 'ETX_60_Energy_40', 'ETX_40_Energy_60', 'ETX_20_Energy_80', 'ETX_0_Energy_100']"
	# echo "figName = 'control_overhead.eps'"
	# echo ""
	# echo "plot(title, yLabel, xValues, yValues, sd, xLegends, figName, 0, 100);"


	# echo 'title = "Pacotes de Controle Enviados"'
	# echo -n "yValues = [ "
	# echo -n "${yvalues_sent_control[*]}"
	# echo "]"
	# echo -n "sd = [ "
	# echo -n "${sd_sent_control[*]}"
	# echo "]"

	# echo 'title = "Pacotes de Dados Enviados"'
	# echo -n "yValues = [ "
	# echo -n "${yvalues_sent_data[*]}"
	# echo "]"
	# echo -n "sd = [ "
	# echo -n "${sd_sent_data[*]}"
	# echo "]"

	# echo 'title = "Pacotes de Controle Recebidos"'
	# echo -n "yValues = [ "
	# echo -n "${yvalues_received_control[*]}"
	# echo "]"
	# echo -n "sd = [ "
	# echo -n "${sd_received_control[*]}"
	# echo "]"

	# echo 'title = "Pacotes de Dados Recebidos"'
	# echo -n "yValues = [ "
	# echo -n "${yvalues_received_data[*]}"
	# echo "]"
	# echo -n "sd = [ "
	# echo -n "${sd_received_data[*]}"
	# echo "]"

	data_packets_sent="  data_packets_sent = \[${yvalues_sent_data[*]}\]"

	sed -i.bak "s/^  data_packets_sent = \[.*/$data_packets_sent/" plot-bar.py
	
	#echo "data_packets_sent = [${yvalues_sent_data[*]}]"
  data_packets_delivered="  data_packets_delivered = \[${yvalues_received_data[*]}\]"

	sed -i.bak "s/^  data_packets_delivered = \[.*/$data_packets_delivered/" plot-bar.py
	
  control_packets_sent="  control_packets_sent = \[${yvalues_sent_control[*]}\]"

	sed -i.bak "s/^  control_packets_sent = \[.*/$control_packets_sent/" plot-bar.py
  
  control_packets_delivered="  control_packets_delivered = \[${yvalues_received_control[*]}\]"

	sed -i.bak "s/^  control_packets_delivered = \[.*/$control_packets_delivered/" plot-bar.py

	python plot-bar.py
