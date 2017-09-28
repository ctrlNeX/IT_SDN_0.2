#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdn-debug.h"
#include "sdn-protocol.h"
#include "sdn-reliability.h"
//#include "sdn-packetbuf.h"
#include "sdn-send-packet.h"
#include "sdn-serial-send.h"
#include "sdn-flow-table-cache.h"
#include "digraph.h"
#include "sdn-process-packets-controller.h"

void sdn_reliability_timer_event() {

  flow_id_table_ptr flow_id_search = sdn_flow_id_table_get();

  while(flow_id_search != NULL) {

    if(flow_id_search->countdown > 0 && flow_id_search->online == 0) {

      flow_id_search->countdown--;

      if(flow_id_search->countdown == 0) {

        flow_id_search->countdown = SDN_RETRANSMIT_TIME_S;

        SDN_DEBUG("### Retransmitting data flow setup with ");

#ifdef SDN_SOURCE_ROUTED
        SDN_DEBUG("SDN_SOURCE_ROUTED\n");
        send_src_rtd_data_flow_setup((sdnaddr_t*)flow_id_search->source, flow_id_search->flowid, (sdnaddr_t*)flow_id_search->next_hop, flow_id_search->action);
#else
        SDN_DEBUG("NOT SDN_SOURCE_ROUTED\n");
        sdn_send_data_flow_setup((sdnaddr_t*)flow_id_search->source, flow_id_search->flowid, (sdnaddr_t*)flow_id_search->next_hop, flow_id_search->action);
#endif
      }
    }

    flow_id_search = flow_id_search->next;
  }

  flow_address_table_ptr flow_address_search = sdn_flow_address_table_get();

  while(flow_address_search != NULL) {

    if(flow_address_search->countdown > 0 && flow_address_search->online == 0) {

      flow_address_search->countdown--;

      if(flow_address_search->countdown == 0) {

        flow_address_search->countdown = SDN_RETRANSMIT_TIME_S;

        SDN_DEBUG("### Retransmitting control flow setup with");

#ifdef SDN_SOURCE_ROUTED
        SDN_DEBUG("SDN_SOURCE_ROUTED\n");
        send_src_rtd_control_flow_setup((sdnaddr_t*)flow_address_search->source, (sdnaddr_t*)flow_address_search->target, (sdnaddr_t*)flow_address_search->next_hop, flow_address_search->action);
#else
        SDN_DEBUG("NOT SDN_SOURCE_ROUTED\n");
        sdn_send_control_flow_setup((sdnaddr_t*)flow_address_search->source, (sdnaddr_t*)flow_address_search->target, (sdnaddr_t*)flow_address_search->next_hop, flow_address_search->action);
#endif
      }
    }

    flow_address_search = flow_address_search->next;

  }  
}