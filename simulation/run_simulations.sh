#!/bin/bash
set -e
set -x
#set -v

tput reset
# Simulation set configuration
# MIN_ITER=1
# MAX_ITER=10
MIN_ITER=1
MAX_ITER=10
LIMIT_ITER=5 #max simulations running in parallel
# nodes_v=(9 16 25)
nodes_v=(16)
# topologies=(GRID-FULL GRID-RND GRID-CTA)
topologies=(GRID-FULL)
# nd_possibilities=(NV CL BL) # Naive Collect and Baseline
nd_possibilities=(CL)
# datarates=(0.2 2)
datarates=(1)

#Controller/Sink positions (C = center, L = left, R = right, U = up, D = down)
cont_sink_position=(CC)

etx_energy_legend=(ETX_100_ENERGY_0 ETX_80_ENERGY_20 ETX_60_ENERGY_40 ETX_40_ENERGY_60 ETX_20_ENERGY_80 ETX_0_ENERGY_100)

#etx_energy_legend=(ETX_100_ENERGY_0)

code["N16_CLU"]="if(0 == 1 || sdn_node_addr.u8[0] == 2 || sdn_node_addr.u8[0] == 3  || sdn_node_addr.u8[0] == 4  || sdn_node_addr.u8[0] == 5  || sdn_node_addr.u8[0] == 6 \
     || sdn_node_addr.u8[0] == 9  || sdn_node_addr.u8[0] == 10  || sdn_node_addr.u8[0] == 13  || sdn_node_addr.u8[0] == 14  || sdn_node_addr.u8[0] == 15\
      || sdn_node_addr.u8[0] == 16  || sdn_node_addr.u8[0] == 17) {"

code["N16_CC"]="if(0 == 1 || sdn_node_addr.u8[0] == 2 || sdn_node_addr.u8[0] == 3  || sdn_node_addr.u8[0] == 4  || sdn_node_addr.u8[0] == 5  || sdn_node_addr.u8[0] == 6 \
     || sdn_node_addr.u8[0] == 9  || sdn_node_addr.u8[0] == 10  || sdn_node_addr.u8[0] == 13  || sdn_node_addr.u8[0] == 14  || sdn_node_addr.u8[0] == 15\
      || sdn_node_addr.u8[0] == 16  || sdn_node_addr.u8[0] == 17 || sdn_node_addr.u8[0] == 18) {"

code["N25_CLU"]="if(0 == 1 || sdn_node_addr.u8[0] == 4 || sdn_node_addr.u8[0] == 5  || sdn_node_addr.u8[0] == 6  || sdn_node_addr.u8[0] == 7  || sdn_node_addr.u8[0] == 8 \
   || sdn_node_addr.u8[0] == 12  || sdn_node_addr.u8[0] == 13  || sdn_node_addr.u8[0] == 17  || sdn_node_addr.u8[0] == 18  || sdn_node_addr.u8[0] == 22\
    || sdn_node_addr.u8[0] == 23  || sdn_node_addr.u8[0] == 24  || sdn_node_addr.u8[0] == 25  || sdn_node_addr.u8[0] == 26  || sdn_node_addr.u8[0] == 27) {"




etx=(1.0 0.8 0.6 0.4 0.2 0.0)
energy=(0.0 0.2 0.4 0.6 0.8 1.0)
## datadistrs=(CTE EXP)
### datadistrs=(CTE)
## schedules=(CTK3 CTK7 IND)
### schedules=(CTK3 CTK7)

contiki_dir="$HOME/contiki" #-3.0"
cooja_dir="/tools/cooja"
simulation_files_dir="$HOME/it-sdn/branches/doriedson/simulation"
simulation_output_dir="$HOME/it-sdn/branches/doriedson/simulation/results"
build_dir="$HOME/it-sdn/branches/doriedson/applications"
controller_dir="$HOME/it-sdn/branches/doriedson/controller-server"
controller_build_dir="$controller_dir"/"build-controller-pc-Desktop_Qt_5_8_0_GCC_64bit-Debug"
QMAKE="/home/dori/Qt/5.8/gcc_64/bin/qmake"
MAKE="make"
export CONTIKI="$HOME/contiki"

pidJava=()
pidController=()

run_cooja() {
	
		cd "${contiki_dir}${4}${cooja_dir}/dist"
		java -mx5120m -jar cooja.jar -nogui="$1" > $2 2>&1 &

#		local pidJava=$!
		pidJava=(${pidJava[@]} $!)
		#echo ${pidJava[@]}

		sleep 5s # wait time so serial socket is available
		"$controller_build_dir"/controller-pc $nnodes --platform offscreen > $3 2>&1 &

		#local pidController=$!
		pidController=(${pidController[@]} $!)

#		wait $pidJava
#		kill $pidController
	
}

# echo "" > makelog.log

mkdir -p $simulation_output_dir

for ((index_route = 0; index_route < ${#etx_energy_legend[@]}; index_route++)); do
	for iter in `seq $MIN_ITER $MAX_ITER`; do
		for nnodes in "${nodes_v[@]}"; do
			for topo in "${topologies[@]}"; do
				for nd in "${nd_possibilities[@]}"; do
					for position in "${cont_sink_position[@]}"; do

						port=$((60000+$iter));
						#for datarate in "${datarates[@]}"; do
							# s - schedule (CTK - contiki default, IND - individual slots for each node)
							# n - number of nodes
							# d - data traffic type
							# l - lambda (data rate = 1/packet interval)
							# i - iteration (experimetn number)

							cooja_file="$simulation_files_dir"/n${nnodes}_${topo}_${position}.csc
							cooja_motes_out_file="$simulation_output_dir"/"cooja_"${etx_energy_legend[$index_route]}"_n"$nnodes"_top"$topo"_nd"$nd"_${position}_i"$iter'.txt'
							cooja_log_file="$simulation_output_dir"/"logcooja_"${etx_energy_legend[$index_route]}"_n"$nnodes"_top"$topo"_nd"$nd"_${position}_i"$iter'.txt'
							controller_out_file="$simulation_output_dir"/"controller_"${etx_energy_legend[$index_route]}"_n"$nnodes"_top"$topo"_nd"$nd"_${position}_i"$iter'.txt'
							controller_log_file="$simulation_output_dir"/"logcontroller_"${etx_energy_legend[$index_route]}"_n"$nnodes"_top"$topo"_nd"$nd"_${position}_i"$iter'.txt'

							echo Cooja simulation file: $cooja_file
							echo Cooja logfile name: $cooja_motes_out_file
							echo Controller logfile name: $controller_out_file

							# Change the COOJA simuation file to save simulation log with a different name
							sed -i.bak "s,my_output =.*,my_output = new FileWriter(\"$cooja_motes_out_file\");," ${cooja_file}

							# SIM_TIME_MS=1200000
							# SIM_TIME_MS=1800000
							# SIM_TIME_MS=60000
							# sed -i.bak "s,TIMEOUT.*,TIMEOUT($SIM_TIME_MS);," ${cooja_file}

							# Clean current binaries
							(
								cd "${build_dir}"

								sed -i.bak "s,if(0 == 1.*,${code[N${topo}_${position}]}," enabled-node.c

								cd "${controller_dir}/controller-pc"
								
								sed -i.bak "s,DEFINES += WEIGHT_ETX.*,DEFINES += WEIGHT_ETX=${etx[$index_route]}," controller-pc.pro

								sed -i.bak "s,DEFINES += WEIGHT_ENERGY.*,DEFINES += WEIGHT_ENERGY=${energy[$index_route]}," controller-pc.pro
								
								sed -i.bak "s,<number>60.*,<number>${port}</number>," mainwindow.ui

								cd "${simulation_files_dir}"
								
								sed -i.bak "s,<port>60.*,<port>${port}</port>," n${nnodes}_${topo}_${position}.csc

								cd "$build_dir"

								# alterar project-conf.h
								SDN_CONF_CD=""
								SDN_CONF_ND=""
								if [ "$nd" == "NV" ]; then
									SDN_CONF_CD="naive_directed_cd"
									SDN_CONF_ND="naive_sdn_nd"
									sed -i.bak "s,^//\s*#undef NETSTACK_CONF_NETWORK,#undef NETSTACK_CONF_NETWORK," project-conf.h
									sed -i.bak "s,^//\s*#define NETSTACK_CONF_NETWORK sdn_network_driver,#define NETSTACK_CONF_NETWORK sdn_network_driver," project-conf.h
									sed -i.bak "s,^CONTIKI_WITH_RIME = 1,CONTIKI_WITH_RIME = 0," Makefile_enabled_node
									sed -i.bak "s,^CONTIKI_WITH_RIME = 1,CONTIKI_WITH_RIME = 0," Makefile_controller_node
									sed -i.bak "s,^CONTIKI_SOURCEFILES += collect-nd.c,# CONTIKI_SOURCEFILES += collect-nd.c," Makefile_enabled_node
									sed -i.bak "s,^CONTIKI_SOURCEFILES += collect-nd.c,# CONTIKI_SOURCEFILES += collect-nd.c," Makefile_controller_node
								elif [ "$nd" == "CL" ]; then
									SDN_CONF_CD="null_sdn_cd"
									SDN_CONF_ND="collect_sdn_nd"
									sed -i.bak "s,^\s*#undef NETSTACK_CONF_NETWORK,// #undef NETSTACK_CONF_NETWORK," project-conf.h
									sed -i.bak "s,^\s*#define NETSTACK_CONF_NETWORK sdn_network_driver,// #define NETSTACK_CONF_NETWORK sdn_network_driver," project-conf.h
									sed -i.bak "s,^CONTIKI_WITH_RIME = 0,CONTIKI_WITH_RIME = 1," Makefile_enabled_node
									sed -i.bak "s,^CONTIKI_WITH_RIME = 0,CONTIKI_WITH_RIME = 1," Makefile_controller_node
									sed -i.bak "s,^#\s*CONTIKI_SOURCEFILES += collect-nd.c,CONTIKI_SOURCEFILES += collect-nd.c," Makefile_enabled_node
									sed -i.bak "s,^#\s*CONTIKI_SOURCEFILES += collect-nd.c,CONTIKI_SOURCEFILES += collect-nd.c," Makefile_controller_node

								# elif [ "$nd" == "BL" ]; then
								# 	SDN_CONF_CD=""
								# 	SDN_CONF_ND=""
								else
									echo "there is no such ND algorithm"
									continue
								fi
								sed -i.bak "s,^#define SDN_CONF_CD.*,#define SDN_CONF_CD $SDN_CONF_CD," project-conf.h
								sed -i.bak "s,^#define SDN_CONF_ND.*,#define SDN_CONF_ND $SDN_CONF_ND," project-conf.h

								echo "$SDN_CONF_CD""$SDN_CONF_ND"

								(make TARGET=wismote clean -f Makefile_enabled_node >> $controller_log_file 2>&1)
								(make TARGET=wismote -f Makefile_enabled_node sink-node >> $controller_log_file 2>&1)

								(make TARGET=wismote clean -f Makefile_enabled_node >> $controller_log_file 2>&1)
								(make TARGET=wismote -f Makefile_enabled_node enabled-node >> $controller_log_file 2>&1)

								(make TARGET=wismote clean -f Makefile_controller_node >> $controller_log_file 2>&1)
								(make TARGET=wismote -f Makefile_controller_node >> $controller_log_file 2>&1)

								# Recompile the node code
								# SCHEDULE=$schedulenum \
								# 	SCHEDULE_LEN=$schedule_len \
								# 	DISTR=$datadistrnum \
								# 	DATARATE=$datarate \
								# 	NNODES=$nnodes \
								# 	TEST_TSCH_SEED="$(od -vAn -N2 -tu4 < /dev/urandom | xargs)" \
								# 	make \
								# 	> makelog.log
							)

							#controller recompilation
							(
								cd $controller_build_dir
								DEFINES=""
								if [ "$nd" == "NV" ]; then
									# DEFINES="DEFINES+=SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC"
									sed -i.bak "s,#\s*DEFINES += SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC,DEFINES += SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC," ../controller-pc/controller-pc.pro
									sed -i.bak "s,DEFINES += SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS,#DEFINES += SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS," ../controller-pc/controller-pc.pro
								elif [ "$nd" == "CL" ]; then
									# DEFINES="DEFINES+=SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC"
									sed -i.bak "s,#\s*DEFINES += SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC,DEFINES += SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC," ../controller-pc/controller-pc.pro
									sed -i.bak "s,#\s*DEFINES += SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS,DEFINES += SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS," ../controller-pc/controller-pc.pro
								# elif [ "$nd" == "BL" ]; then
								# 	SDN_CONF_CD=""
								# 	SDN_CONF_ND=""
								else
									echo "there is no such ND algorithm"
									continue
								fi
								# $QMAKE ../controller-pc/controller-pc.pro "$DEFINES"
								($QMAKE ../controller-pc/controller-pc.pro >> $controller_log_file 2>&1)
								(touch ../sdn-process-packets-controller.c >> $controller_log_file 2>&1)
								($MAKE >> $controller_log_file 2>&1)
							)

							# if [ $? != 0 ]; then
							# 	echo error: Compilation problem! \($outfile\)
							# else
							# Run the COOJA simulation
							run_cooja `readlink -f ${cooja_file}` "$cooja_log_file" "$controller_out_file" $iter
							#echo
							# fi
		#					sed -i.bak "s,my_output =.*,my_output = new FileWriter(\"/dev/null\");," ${cooja_file}
							# python sdn_get_statistics_preproc.py  $cooja_motes_out_file &

					done
				done
			done
		done

		if [ ${#pidJava[@]} -eq $LIMIT_ITER ]; then
	#		echo "Waiting simulations finish. ${pidJava[@]}"
			wait ${pidJava[@]}

			#echo "Finishing controllers. ${pidController[@]}"
			kill ${pidController[@]}
			#killall controller-pc

			pidJava=()
			pidController=()
		fi
	done

	#echo "Waiting simulations finish. ${pidJava[@]}"
	wait ${pidJava[@]}

	#echo "Finishing controllers. ${pidController[@]}"
	if [ ${#pidController[@]} -gt 0 ]; then
		kill ${pidController[@]}
	fi
	#killall controller-pc

	cd "${simulation_output_dir}"
	
	mkdir -p ${etx_energy_legend[$index_route]}

	mv *.txt ${etx_energy_legend[$index_route]}
done




sed -i.bak "s,my_output =.*,my_output = new FileWriter(\"/dev/null\");," ${cooja_file}

#echo "tschsimlog_s"$schedule"_n"$nnodes"_d"$datatype"_l"$datarate"_i"$iter
echo END
