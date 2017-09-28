#!/bin/bash

#source "${0%/*}/anim.sh"
function Render(){
	a=0
}

for folder in */ ; do
#folder="./"

 	file_process="${folder///}.txt"
 	>$file_process

	for files in $(ls -1 $folder | grep '^cooja_.*.txt'); do
		# cat cooja-1.log | grep "Packet Counter"
		echo "Simulation: ${files}"
		echo "Simulation: ${files}" >> $file_process

		energy_consumed=0
		energy_total=0

		for ((k=2; k<=17; k++)); do
			bat=(`cat "${folder}${files}" | grep -E ":${k}:Battery" | tail -n1`)

			bat=${bat[2]}

			((bat = 255 - bat))

			((energy_consumed += bat))

			((energy_total += 255))

			#echo "$k $bat ${energy_consumed}"
		done

		((energy_consumed = energy_consumed * 100 / energy_total))

		#echo "consumed ${energy_consumed}"

		file_number=${files#*_}

		file_number=${file_number%.*}

		server_file="${folder}controller_${file_number}.txt"

		control_packet_sent=$(cat $server_file | grep -Ec "Packet Sent .+ \[(SDN_PACKET_CONTROL_FLOW_SETUP|SDN_PACKET_DATA_FLOW_SETUP|SDN_PACKET_SRC_ROUTED_CONTROL_FLOW_SETUP|SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP|SDN_PACKET_MULTIPLE_CONTROL_FLOW_SETUP|SDN_PACKET_MULTIPLE_DATA_FLOW_SETUP|SDN_PACKET_ACK|SDN_PACKET_SRC_ROUTED_ACK)")

		control_flow_request_received=$(cat $server_file | grep -Ec "Packet Received: \[SDN_PACKET_DATA_FLOW_REQUEST\]")
		((control_flow_request_received+=$(cat $server_file | grep -Ec "Packet Received: \[SDN_PACKET_CONTROL_FLOW_REQUEST\]")))

		neighbor_report_received=$(cat $server_file | grep -Ec "Packet Received: \[SDN_PACKET_NEIGHBOR_REPORT\]")
		((neighbor_report_received-=$(cat $server_file | grep -Ec "Packet Received: \[SDN_PACKET_NEIGHBOR_REPORT\].+01 00\]")))
		#control_flow_request_sent=$(cat $server_file | grep -Ec "CONTROL_FLOW_REQUEST. origin: 0")
control_flow_request_sent=0
		#neighbor_report_sent=$(cat $server_file | grep -Ec "Neighbor information from 0")

		control_packet_received=0

		#((control_packet_received+=$(cat "${folder}${files}" | grep -Ec "Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\]")))
		#((control_packet_received+=$(cat "${folder}${files}" | grep -Ec "Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\]")))

		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":2:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 02")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":3:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 03")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":4:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 04")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":5:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 05")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":6:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 06")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":7:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 07")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":8:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 08")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":9:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 09")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":10:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0A")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":11:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0B")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":12:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0C")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":13:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0D")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":14:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0E")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":15:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0F")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":16:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 10")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":17:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_ACK\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 11")))

		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":2:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 02")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":3:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 03")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":4:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 04")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":5:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 05")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":6:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 06")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":7:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 07")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":8:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 08")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":9:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 09")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":10:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0A")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":11:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0B")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":12:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0C")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":13:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0D")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":14:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0E")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":15:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 0F")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":16:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 10")))
		((control_packet_received+=$(cat "${folder}${files}" | grep -Ec ":17:Packet Received from .+ \[SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP\].+Payload \[[0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} 11")))

		link_estimator_sent=$(cat "${folder}${files}" | grep -Ec "Collection Protocol Sending")

		link_estimator_received=$link_estimator_sent
		#link_estimator_received=$(cat "${folder}${files}" | grep -Ec "Collection Protocol Receiving")

		((control_packet_received+=link_estimator_received))

		((control_packet_sent+=link_estimator_sent))

		`cat "${folder}${files}" | grep "data packet has been received" > tmp`
		# `sed -i -e 's/ //g' tmp`
		`sed -i -e 's/\:/ /g' tmp`
		# `sed -i -e 's/)/ /g' tmp`
		# `sed -i -e 's/(/ /g' tmp`

			unset nodeIdResult
			unset simTimeResult
			unset dataResult
			# unset sdnControl
			# unset sdnData
			# unset data

		# while read a b c d e f g h i j; do
		# 	notSdn[$b]=$e
		# 	sdnControl[$b]=$g
		# 	sdnData[$b]=$i
		# 	data[$b]="$a $b $c $d $e $f $g $h $i $j"
		# done < tmp	

		while read simTime nodeId data; do
			nodeIdResult[$nodeId]=$nodeId
			simTimeResult[$nodeId]=$simTime
			dataResult[$nodeId]=$data
		done < tmp	

		# notSdnTotal=0
		# sdnControlTotal=0
		# sdnDataTotal=0
		# total=0
		nodeIdResult=(${nodeIdResult[@]})

		for ((k = 0; k < ${#nodeIdResult[@]}; k++)); do
			Render
		# 	((notSdnTotal+=notSdn[k]))
		# 	((sdnControlTotal+=sdnControl[k]))
		# 	((sdnDataTotal+=sdnData[k]))
			echo "time $((simTimeResult[${nodeIdResult[$k]}] / 1000000))s node: ${nodeIdResult[$k]} - ${dataResult[${nodeIdResult[$k]}]}" >> $file_process	
		done

		cat "${folder}${files}" | grep "CTP : CONTROL FLOW REQUEST to node" > tmp

		while read a b c d e f g flowid i; do
			if [ "$flowid" -gt "16" ]; then
					echo "flow $flowid"
			#echo "time $((simTime / 1000000))s node: $nodeId - $data" >> $file_process
			fi
		done < tmp


		`cat "${folder}${files}" | grep "Battery level 0" > tmp`

		`sed -i -e 's/\:/ /g' tmp`

		while read simTime nodeId data; do
			Render
			echo "time $((simTime / 1000000))s node: $nodeId - $data" >> $file_process
		done < tmp	

		((control_flow_request_sent+=$(cat "${folder}${files}" | grep -Ec "Packet Sent to .+ \[SDN_PACKET_DATA_FLOW_REQUEST\]"))) #'FLOW REQUEST'`

		#filename=`sed 's/^[^_]*[_]/server_/' <<< "${folder}${files}"` #;s/[.].*$//

		#count2=`cat ${filename} | grep -c 'Packet Sent to Serial'`

		#echo "Control Packet Sent $((count+count2))" >> $file_process

		((neighbor_report_sent=$(cat "${folder}${files}" | grep -Ec "Packet Sent to .+ \[SDN_PACKET_NEIGHBOR_REPORT\]")))

		echo "Link Estimator Sent: ${link_estimator_sent} Received: ${link_estimator_received}" >> $file_process

		echo "Flow Request Sent: ${control_flow_request_sent} Received: ${control_flow_request_received}" >> $file_process

		echo "Flow Response Sent: ${control_packet_sent} Received: ${control_packet_received}" >> $file_process

		echo "Neighbor Report Sent: ${neighbor_report_sent} Received: ${neighbor_report_received}" >> $file_process

		echo "Control Packet Sent: $((control_packet_sent+control_flow_request_sent+neighbor_report_sent)) Received: $((control_packet_received+control_flow_request_received+neighbor_report_received))" >> $file_process

		count_sent=$(cat "${folder}${files}" | grep -Ec 'Packet Sent .+ \[SDN_PACKET_DATA')
		
		# count_sent=0
		
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":2:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 02 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":3:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 03 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":4:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 04 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":5:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 05 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":6:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 06 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":7:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 07 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":8:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 08 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":9:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 09 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":10:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 0A 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":11:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 0B 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":12:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 0C 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":13:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 0D 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":14:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 0E 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":15:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 0F 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":16:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 10 00] Payload")))   
		# ((count_sent+=$(cat "${folder}${files}" | grep -Ec ":17:Packet Sent .+ \[SDN_PACKET_DATA\] .+ 11 00] Payload")))   

		count_received=$(cat "${folder}${files}" | grep -Ec 'Receiving message from')

		echo "Data Packet Sent: ${count_sent} Received: ${count_received}" >> $file_process

		echo "Energy Consumed: ${energy_consumed}" >> $file_process

		# `cat "${folder}${files}" | grep -E '16:Ack Confirmed to Packet Sent.+\[ 0 0.+7 e0' > tmp`
		# `sed -i -e 's/\:/ /g' tmp`

		# packetIdBefore="";
		
		# if [ -f tmp2 ]; then
		# 	rm tmp2
		# fi
		# >tmp2

		# while read simTime nodeId a b c d e f g nodeTarget i j k l m n o p q r s packetId packetId2 u; do
		# 	Render
		# 	if [ "$packetIdBefore" != "${packetId} ${packetId2}" ] ; then

		# 		packetIdBefore="${packetId} ${packetId2}"

		# 		filter="Packet Received.+\[ [a-z0-9]{1,2} [a-z0-9]{1,2} [a-z0-9]{1,2} [a-z0-9]{1,2} 93 0 [a-z0-9]{1,2} [a-z0-9]{1,2} [a-z0-9]{1,2} [a-z0-9]{1,2} ${packetIdBefore} ]"
		# 		# echo $filter

		# 		`cat "${folder}${files}" | grep -E "${filter}" >> tmp2`

		# 	fi

		# done < tmp

		# `sed -i -e 's/\:/ /g' tmp2`

		# `cat "${folder}${files}" | grep -E 'Packet Received.+\[ [a-z0-9]{1,2} [a-z0-9]{1,2} [a-z0-9]{1,2} [a-z0-9]{1,2} 93 0' > tmp`
		# `sed -i -e 's/\:/ /g' tmp`

		#packetIdBefore="";
		#lastBuffer=""
		#buffer=""
		#lastTarget=0
		#count=0

	# 	echo "Packet routes:" >> $file_process

	# 	while read simTime nodeId a b c d nodeTarget f g h packetId i; do
	# 		Render		

	# 		if [ "$packetIdBefore" != "$packetId" ] ; then

	# 			packetIdBefore=$packetId
	# 			buffer+="]"

	# 			if [ "$lastTarget" -eq 1 ]; then			
	# 				if [ "$lastBuffer" != "$buffer" ]; then

	# 					echo "$lastBuffer - ${count}" >> $file_process			
	# 					count=0
	# 					lastBuffer=${buffer}
	# 				fi
	# 				((count++))
								
	# 			fi

	# 			lastTarget=0
	# 			buffer="[ "
	# 		fi

	# 		lastTarget=$nodeId
	# 		buffer+="${nodeId} "
	# 	done < tmp2

	# 	if [ "$lastBuffer" != "$buffer" ]; then

	# 		echo "$lastBuffer - ${count}" >> $file_process			
	# 		count=0
	# 		lastBuffer=${buffer}
	# 	fi

		echo "" >> $file_process
	done;

done

# rm tmp
# rm tmp2

echo -e "\nThat's all Folks!\n"