/*
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain this list of conditions
 *    and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE SOFTWARE PROVIDER OR DEVELOPERS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdlib.h>
#include "sdn-debug.h"

#include "digraph.h"

#include "sdn-flow-table-cache.h"

#include "control-flow-table.h"
#include "data-flow-table.h"
#include "sdn-queue.h"

#include "sdn-send-packet.h"
#include "sdn-process-packets-controller.h"
#include "sdn-packetbuf.h"
#include "mainwindow_wrapper.h"

#if !(defined(SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS) || defined(SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC))
#error SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS nor SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC defined
#endif

//Defines when weight changes can be used in routes.
#define ROUTE_SENSIBILITY 0.2

route_ptr get_flow_id_better_route(uint8_t *source, uint16_t flowid, flow_id_table_ptr flow_id_routing_field);

route_ptr get_better_route(uint8_t *source, uint8_t *target, route_ptr route_better, flow_address_table_ptr flow_address_routing_field);

void process_control_flow_request(sdn_control_flow_request_t *sdn_control_flow_request);

void process_flow_address_table_changes();

void process_flow_id_table_changes();

void process_data_flow_setup(route_ptr route, uint16_t flowid, flow_id_table_ptr routing_field, sdnaddr_t* source_request);

void process_control_flow_setup(route_ptr route, unsigned char *target_request, flow_address_table_ptr routing_field, sdnaddr_t* source_request);



void enqueue_sdn_data_flow_request_to_controller(uint8_t *source);

//void enqueue_sdn_data_flow_setup(sdnaddr_t* packet_destination, flowid_t dest_flowid, sdnaddr_t* route_nexthop, action_t action);

//void enqueue_sdn_control_flow_setup(sdnaddr_t* packet_destination, sdnaddr_t* route_destination, sdnaddr_t* route_nexthop, action_t action);

void process_energy_report(sdn_energy_report_t* sdn_energy_report) {

  int digraph_changed = 0;

  printf("Source ");
  sdnaddr_print(&(sdn_energy_report->header.source));
  printf(" battery level (%u / 255)\n", sdn_energy_report->energy);

  vertice_ptr vertice_source = digraph_find_vertice(sdn_energy_report->header.source.u8);
  if(vertice_source == NULL) {
    vertice_source = digraph_add_vertice(sdn_energy_report->header.source.u8);
    enqueue_sdn_data_flow_request_to_controller(sdn_energy_report->header.source.u8);
    digraph_changed = 1;
  }

  if(digraph_update_energy_vertice(vertice_source, sdn_energy_report->energy) == 1) {
    digraph_changed = 1;
  }

  //Remove the nodes with battery less than 5 percent.
  // if(sdn_energy_report->energy < MAX_BATTERY_LEVEL * 5 / 100) {
  //   printf("Node with low battery.\n");

  //   #ifdef SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS
  //   digraph_mark_edges_to_del_from(vertice_source);
  //   #endif
  //   #ifdef SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC
  //   digraph_mark_edges_to_del_towards(vertice_source);
  //   #endif  

  // //Removes edges not informed.
  //   #ifdef SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS  
  //   if(digraph_del_marked_edges_to_del_towards(vertice_source) == 1) {
  //     digraph_changed = 1;
  //   }
  //   #endif
  //   #ifdef SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC
  //   if(digraph_del_marked_edges_to_del_from(vertice_source) == 1) {
  //     digraph_changed = 1;
  //   }
  //   #endif 

  //   flow_id_table_remove_source(sdn_energy_report->header.source.u8);
  //   flow_address_table_remove_source(sdn_energy_report->header.source.u8);
  //   flow_id_list_table_remove_target(sdn_energy_report->header.source.u8);

  // }

  if(digraph_changed == 1) {
    update_flow_table_cache();
  // } else {
  //   sdn_send_down_once();
  }

#if SDN_TX_RELIABILITY

#ifdef SDN_SOURCE_ROUTED  
  send_src_rtd_ack(&SDN_HEADER(sdn_energy_report)->source, SDN_HEADER(sdn_energy_report)->type, SDN_HEADER(sdn_energy_report)->seq_no);
#else
  send_ack_by_flow_address(&SDN_HEADER(sdn_energy_report)->source, SDN_HEADER(sdn_energy_report)->type, SDN_HEADER(sdn_energy_report)->seq_no); 
#endif

#endif // SDN_TX_RELIABILITY
}

void process_neighbor_report(sdn_neighbor_report_t* sdn_neighbor_report, void* neighbors) {

  vertice_ptr vertice_source = digraph_find_vertice(sdn_neighbor_report->header.source.u8);

  vertice_ptr vertice_dest;
  edge_ptr edge;

  int digraph_changed = 0;
  int index_neighbor;

  if(vertice_source == NULL) {
    vertice_source = digraph_add_vertice(sdn_neighbor_report->header.source.u8);
    enqueue_sdn_data_flow_request_to_controller(sdn_neighbor_report->header.source.u8);
    digraph_changed = 1;
  }

  printf("Source ");
  sdnaddr_print(&(sdn_neighbor_report->header.source));
  printf(" neighbors (%d)\n", sdn_neighbor_report->num_of_neighbors);

  //Marks edges to remove before updates.
#ifdef SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS
  digraph_mark_edges_to_del_from(vertice_source);
#endif
#ifdef SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC
  digraph_mark_edges_to_del_towards(vertice_source);
#endif

//  if(sdn_neighbor_report->battery > MAX_BATTERY_LEVEL * 5 / 100) { //TODO: to analyze.

    for(index_neighbor = 0; index_neighbor < sdn_neighbor_report->num_of_neighbors; index_neighbor++) {

      sdn_neighbor_report_list_t* neighbor_list = (sdn_neighbor_report_list_t*) (neighbors + (sizeof(sdn_neighbor_report_list_t) * index_neighbor));

      vertice_dest = digraph_find_vertice(neighbor_list->neighbor.u8);

      if(vertice_dest == NULL) {
        vertice_dest = digraph_add_vertice(neighbor_list->neighbor.u8);
        //TODO: verificar se é dependente de simulação
        // if (digraph_count_vertice() == sdn_get_cli_nnodes()) {
        //   SDN_DEBUG("Graph complete!\n");
        //   uint8_t *v = malloc(sizeof(uint8_t));
        //   *v = sdn_get_cli_nnodes();
        //   // callSendCustomPacket(v, sizeof(uint8_t), 98);
        //   sdn_send_queue_enqueue_custom(v, sizeof(uint8_t), 98);
        //   sdn_set_cli_nnodes(0);
        //   // free(v);
        // }
        enqueue_sdn_data_flow_request_to_controller(neighbor_list->neighbor.u8);
      }

      /* The code bellow should be used only if links are considered symmetric by default */
#ifdef SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS
     edge = digraph_add_edge(vertice_source, vertice_dest, neighbor_list->etx);
     if(edge->status != EDGE_STATIC) {
       digraph_changed = 1;
     }
#endif

      // A neighbor report indicates the existence of edges
      // from the nodes in the NR list to the NR sender
#ifdef SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC
      edge = digraph_add_edge(vertice_dest, vertice_source, neighbor_list->etx);
      if(edge->status != EDGE_STATIC) {
        digraph_changed = 1;
      }
#endif

      if(edge->status == EDGE_NEW) {
        uint8_t* temp_packet = malloc(sizeof(sdnaddr_t)*2 + sizeof(uint8_t));
        sdnaddr_copy((sdnaddr_t*)temp_packet, &neighbor_list->neighbor);
        sdnaddr_copy((sdnaddr_t*)(temp_packet + sizeof(sdnaddr_t)), &sdn_neighbor_report->header.source);
        *(temp_packet + 2*sizeof(sdnaddr_t)) = SDN_DEFAULT_TTL - SDN_HEADER(sdn_neighbor_report)->thl;
        SDN_DEBUG("Sending serial packet 99: ");
        sdnaddr_print(&neighbor_list->neighbor);
        SDN_DEBUG(" can reach ");
        sdnaddr_print(&sdn_neighbor_report->header.source);
        SDN_DEBUG(". (%d, %d)\n", SDN_HEADER(sdn_neighbor_report)->thl, SDN_DEFAULT_TTL - SDN_HEADER(sdn_neighbor_report)->thl);
        if (sdn_send_queue_enqueue_custom(temp_packet, sizeof(sdnaddr_t)*2 + sizeof(uint8_t), 99) != SDN_SUCCESS) {
          free(temp_packet);
          SDN_DEBUG_ERROR ("Error on packet enqueue.\n");
        } else {
          if (sdn_send_queue_size() == 1)
            sdn_send_down_once();
        }
        // free(temp_packet);
      }
    }
  //}

  //Removes edges not informed.
#ifdef SDN_NEIGHBORINFO_SRC_TO_NEIGHBORS  
  if(digraph_del_marked_edges_to_del_towards(vertice_source) == 1) {
    digraph_changed = 1;
  }
#endif
#ifdef SDN_NEIGHBORINFO_NEIGHBORS_TO_SRC
  if(digraph_del_marked_edges_to_del_from(vertice_source) == 1) {
    digraph_changed = 1;
  }
#endif

  //printf("Ending routine of neighbor information processing.\n");
  digraph_print();

  if(digraph_changed == 1) {
    update_flow_table_cache();
  } else {
    sdn_send_down_once();
  }
#if SDN_TX_RELIABILITY

#ifdef SDN_SOURCE_ROUTED
  send_src_rtd_ack(&SDN_HEADER(sdn_neighbor_report)->source, SDN_HEADER(sdn_neighbor_report)->type, SDN_HEADER(sdn_neighbor_report)->seq_no);
#else
  send_ack_by_flow_address(&SDN_HEADER(sdn_neighbor_report)->source, SDN_HEADER(sdn_neighbor_report)->type, SDN_HEADER(sdn_neighbor_report)->seq_no);
#endif

#endif // SDN_TX_RELIABILITY
}

void process_data_flow_request_packet(sdn_data_flow_request_t *sdn_data_flow_request) {

  // printf("FUNCTION process_data_flow_request.\n");

  flow_id_table_ptr routing_field;

  route_ptr route = NULL;

  routing_field = sdn_flow_id_table_find(sdn_data_flow_request->header.source.u8, sdn_data_flow_request->flowid);

  if(routing_field != NULL) {
#if !SDN_TX_RELIABILITY    
      process_data_flow_setup(NULL, sdn_data_flow_request->flowid, routing_field, &sdn_data_flow_request->header.source);
#endif //!SDN_TX_RELIABILITY
  } else {

    route = get_flow_id_better_route(sdn_data_flow_request->header.source.u8, sdn_data_flow_request->flowid, NULL);

    // If there are no routes to flow id:
    if(route == NULL) {

      printf("Route to flowid %d was not found.\n", sdn_data_flow_request->flowid);

      routing_field = sdn_flow_id_table_add(sdn_data_flow_request->header.source.u8, sdn_data_flow_request->flowid, sdn_data_flow_request->header.source.u8, SDN_ACTION_DROP, 0, 0);

      if(routing_field->changed == 1) {

        process_data_flow_setup(NULL, sdn_data_flow_request->flowid, routing_field, &sdn_data_flow_request->header.source);

        routing_field->changed = 0;
      }

    // If there is at least one route to flow id:
    } else {
      printf("Route was found to flowId %d.\n", sdn_data_flow_request->flowid);

      process_data_flow_setup(route, sdn_data_flow_request->flowid, NULL, &sdn_data_flow_request->header.source);
    }
  }

  sdn_send_down_once();
}

void process_control_flow_request(sdn_control_flow_request_t *sdn_control_flow_request) {

  printf("Control Flow Request: ");
  sdnaddr_print(&(sdn_control_flow_request->header.source));
  printf(" -> ");
  sdnaddr_print(&(sdn_control_flow_request->address));
  printf("\n");

  flow_address_table_ptr routing_field;

  route_ptr route = NULL;

  routing_field = sdn_flow_address_table_find(sdn_control_flow_request->header.source.u8, sdn_control_flow_request->address.u8);

  if(routing_field != NULL) {
    //printf("Dori request control flow, set flow should be in queue.\n");
#if !SDN_TX_RELIABILITY
    process_control_flow_setup(NULL, NULL, routing_field, &sdn_control_flow_request->header.source);
#endif //!SDN_TX_RELIABILITY  
  } else {

    route = get_better_route(sdn_control_flow_request->header.source.u8, sdn_control_flow_request->address.u8, route, NULL);

    if(route == NULL) {
      printf("Route not found.\n");

      routing_field = sdn_flow_address_table_add(sdn_control_flow_request->header.source.u8, sdn_control_flow_request->address.u8, sdn_control_flow_request->header.source.u8, SDN_ACTION_DROP, 0, 0);

      if(routing_field->changed == 1) {
        process_control_flow_setup(NULL, NULL, routing_field, &sdn_control_flow_request->header.source);
        // sdn_send_control_flow_setup((sdnaddr_t*)routing_field->source, (sdnaddr_t*)routing_field->target, (sdnaddr_t*)routing_field->next_hop, routing_field->action);
        routing_field->changed = 0;
      }

    } else {

      printf("Route was found.\n");

      process_control_flow_setup(route, sdn_control_flow_request->address.u8, NULL, &sdn_control_flow_request->header.source);
    }
  }

  sdn_send_down_once();
}

route_ptr get_flow_id_better_route(uint8_t *source, uint16_t flowid, flow_id_table_ptr flow_id_routing_field) {

  route_ptr route_better = NULL;
  route_ptr route_find;
  char keep_route_cache = 0;

//   flow_id_table_ptr flow_id_routing_field = sdn_flow_id_table_find((unsigned char*) source, flowid);

// if (flow_id_routing_field != NULL) {
//   printf("Dori route exist\n");
// } else {
//   printf("Dori route does not exist\n");
// }

  flow_id_list_table_ptr flowid_search = sdn_flow_id_list_table_get();

  // Getting all possible targets from specific flow id to calculate better route.
  while(flowid_search != NULL) {

    if(flowid_search->flowid == flowid) {
      // SDN_DEBUG("Found FlowId %d on address ", flowid);
      // sdnaddr_print((sdnaddr_t *)flowid_search->target);
      // SDN_DEBUG("\n");

      route_better = get_better_route(source, flowid_search->target, route_better, NULL);

      if( flow_id_routing_field != NULL && route_better != NULL) {

        route_find = route_better;

        while (memcmp(route_find->next->vertice_id, source, SDNADDR_SIZE) != 0) {
          // printf("Finding ");
          // sdnaddr_print((sdnaddr_t *) route_find->vertice_id);
          // printf(" \n");
          route_find = route_find->next;
        }
         // printf("Finding X ");
         //  sdnaddr_print((sdnaddr_t *) route_find->next->vertice_id);
         //  printf(" \n");
        if(memcmp(route_find->vertice_id, flow_id_routing_field->next_hop, SDNADDR_SIZE) == 0) {
          keep_route_cache = 1;

          //Update cache route: weight and hop
          sdn_flow_id_table_add(source, flowid, flow_id_routing_field->next_hop, SDN_ACTION_FORWARD, route_better->route_weight, route_better->hops);

          // printf("Dori target better route ");
          // sdnaddr_print((sdnaddr_t *) route_better->vertice_id);
          // printf("Dori next hop better route ");
          // sdnaddr_print((sdnaddr_t *) route_find->vertice_id);
          // printf(" (%d) next hop cache ", route_better->route_weight);
          // sdnaddr_print((sdnaddr_t *) flow_id_routing_field->next_hop);
          // printf(" (%d)\n", flow_id_routing_field->route_weight);
        }
      }
    }

    flowid_search = flowid_search->next;
  }

  if(keep_route_cache == 1) {

    if(route_better->route_weight >= flow_id_routing_field->route_weight * (1.0 - ROUTE_SENSIBILITY) &&
      route_better->route_weight <= flow_id_routing_field->route_weight * (1.0 + ROUTE_SENSIBILITY)) {
      
      route_better->keep_route_cache = 1;
    }
  }

  return route_better;
}

route_ptr get_better_route(uint8_t *source, uint8_t *target, route_ptr route_better, flow_address_table_ptr flow_address_routing_field) {

  route_ptr route_search = dijkstra_shortest_path(source, target);
  route_ptr route_find;

//TODO: comment this, used only for debug.
    // route_ptr route_head = route_search;

    // while (route_search != NULL) {
    //   digraph_print_vertice_id(route_search->vertice_id);
    //   printf("(%d) <- ", route_search->route_weight);

    //   route_search = route_search->next;
    // }

    // printf("\n");

    // route_search = route_head;
//end TODO

  if(route_search != NULL) {

    if(route_better == NULL) {

      route_better = route_search;
    } else {

      if(route_search->route_weight < route_better->route_weight) {

        dijkstra_free_route(route_better);

        route_better = route_search;
      } else {

        dijkstra_free_route(route_search);
      }
    }
  }

  //For route in flow table cache, if exist
  if(flow_address_routing_field != NULL) {

    route_find = route_better;

    while (memcmp(route_find->next->vertice_id, source, SDNADDR_SIZE) != 0) {
      route_find = route_find->next;
    }

    if(memcmp(route_find->vertice_id, flow_address_routing_field->next_hop, SDNADDR_SIZE) == 0) {

      //Update cache route: weight and hop
      sdn_flow_address_table_add(source, target, flow_address_routing_field->next_hop, SDN_ACTION_FORWARD, route_better->route_weight, route_better->hops);

      if(route_better->route_weight >= flow_address_routing_field->route_weight * (1.0 - ROUTE_SENSIBILITY) &&
        route_better->route_weight <= flow_address_routing_field->route_weight * (1.0 + ROUTE_SENSIBILITY)) {
        
        route_better->keep_route_cache = 1;
      }
    }    
  }

  return route_better;
}

void update_flow_table_cache() {

  printf("\nUpdating flow table cache with digraph changes.\n");

  process_flow_id_table_changes();

  process_flow_address_table_changes();

  sdn_send_down_once();
}

void process_flow_address_table_changes() {

  route_ptr route = NULL;

  flow_address_table_ptr flow_address_table_head = sdn_flow_address_table_get();
//TODO: comment this
  while(flow_address_table_head != NULL) {

    if(flow_address_table_head->changed == 1) {
      printf("ERROR: (inconsistency) SOURCE ");
      sdnaddr_print((sdnaddr_t *)flow_address_table_head->source);
      printf(" TARGET ");
      sdnaddr_print((sdnaddr_t *)flow_address_table_head->target);
      printf(" CHANGED = %d\n", flow_address_table_head->changed);
    }

    flow_address_table_head = flow_address_table_head->next;
  }

  flow_address_table_head = sdn_flow_address_table_get();
//end TODO
  while(flow_address_table_head != NULL) {

    if(flow_address_table_head->changed == 0) { //Control for it does not read and change the registry more than once.

      route = get_better_route(flow_address_table_head->source, flow_address_table_head->target, route, flow_address_table_head);

      // If there are no routes to address:
      if(route == NULL) {

        // printf("Route was not found:");
        // printf(" source ");
        // sdnaddr_print((sdnaddr_t *)flow_address_table_head->source);
        // printf(" -> flowid %d\n", flow_address_table_head->flowid);

        // flow_id_table_field =
        sdn_flow_address_table_add(flow_address_table_head->source, flow_address_table_head->target, flow_address_table_head->source, SDN_ACTION_DROP, 0, 0);

      // If there is at least one route to flow id:
      } else if (route->keep_route_cache == 0) {
        // printf("Route was found:");
        // printf(" source ");
        // sdnaddr_print((sdnaddr_t *)flow_address_table_head->source);
        // printf(" -> flowid %d\n", flow_address_table_head->flowid);

// route_ptr route_head = route;

// while (route != NULL) {
//   digraph_print_vertice_id(route->vertice_id);
//   printf("(%d) <- ", route->route_weight);
//   route = route->next;
// }

// printf("\n");

// route = route_head;

        while (route != NULL) {

          if(route->next != NULL) {
            // flow_id_table_field =
            sdn_flow_address_table_add(route->next->vertice_id, flow_address_table_head->target, route->vertice_id, SDN_ACTION_FORWARD, route->next->route_weight, route->next->hops);
          }

          route = route->next;
        }
// route = route_head;
// route_head = NULL;
        dijkstra_free_route(route);
        route = NULL;
      }
    }

    flow_address_table_head = flow_address_table_head->next;
  }

  flow_address_table_head = sdn_flow_address_table_get();

  while(flow_address_table_head != NULL) {

    if(flow_address_table_head->changed == 1) {
      // printf("!!!! SOURCE ");
      // sdnaddr_print((sdnaddr_t *)flow_address_table_head->source);
      // printf(" FLOW ID [%d] CHANGED = %d.\n", flow_address_table_head->flowid, flow_address_table_head->changed);

      //sdn_send_control_flow_setup((sdnaddr_t*)flow_address_table_head->source, (sdnaddr_t*)flow_address_table_head->target, (sdnaddr_t*)flow_address_table_head->next_hop, flow_address_table_head->action);
      process_control_flow_setup(NULL, flow_address_table_head->target, flow_address_table_head, &sdnaddr_null);
      flow_address_table_head->changed = 0;
    }

    flow_address_table_head = flow_address_table_head->next;
  }
}

void process_flow_id_table_changes() {

  route_ptr route;

  flow_id_table_ptr flow_id_table_head = sdn_flow_id_table_get();
//TODO: comment this
  while(flow_id_table_head != NULL) {

    if(flow_id_table_head->changed == 1) {
      printf("ERROR: (inconsistency) SOURCE ");
      sdnaddr_print((sdnaddr_t *)flow_id_table_head->source);
      printf(" FLOW ID [%d] CHANGED = %d.\n", flow_id_table_head->flowid, flow_id_table_head->changed);
    }

    flow_id_table_head = flow_id_table_head->next;
  }

  flow_id_table_head = sdn_flow_id_table_get();
//end TODO
  while(flow_id_table_head != NULL) {

    if(flow_id_table_head->changed == 0) {

      route = get_flow_id_better_route(flow_id_table_head->source, flow_id_table_head->flowid, flow_id_table_head);

      // If there are no routes to flow id:
      if(route == NULL) {

        // printf("Route was not found:");
        // printf(" source ");
        // sdnaddr_print((sdnaddr_t *)flow_id_table_head->source);
        // printf(" -> flowid %d\n", flow_id_table_head->flowid);

        // flow_id_table_field =
        sdn_flow_id_table_add(flow_id_table_head->source, flow_id_table_head->flowid, flow_id_table_head->source, SDN_ACTION_DROP, 0, 0);

      // If there is at least one route to flow id:
      } else if(route->keep_route_cache == 0) {
        // printf("Route was found:");
        // printf(" source ");
        // sdnaddr_print((sdnaddr_t *)flow_id_table_head->source);
        // printf(" -> flowid %d\n", flow_id_table_head->flowid);

        while (route != NULL) {

          if(route->next != NULL) {
            // flow_id_table_field =
            sdn_flow_id_table_add(route->next->vertice_id, flow_id_table_head->flowid, route->vertice_id, SDN_ACTION_FORWARD, route->next->route_weight, route->next->hops);
          }

          route = route->next;
        }
// route = route_head;
// route_head = NULL;
        dijkstra_free_route(route);
      }
    }

    flow_id_table_head = flow_id_table_head->next;
  }

  flow_id_table_head = sdn_flow_id_table_get();

  while(flow_id_table_head != NULL) {

    if(flow_id_table_head->changed == 1) {
      // printf("!!!! SOURCE ");
      // sdnaddr_print((sdnaddr_t *)flow_id_table_head->source);
      // printf(" FLOW ID [%d] CHANGED = %d.\n", flow_id_table_head->flowid, flow_id_table_head->changed);

      // sdn_send_data_flow_setup((sdnaddr_t*)flow_id_table_head->source, flow_id_table_head->flowid, (sdnaddr_t*)flow_id_table_head->next_hop, flow_id_table_head->action);
      process_data_flow_setup(NULL, flow_id_table_head->flowid, flow_id_table_head, &sdnaddr_null);
      flow_id_table_head->changed = 0;
    }

    flow_id_table_head = flow_id_table_head->next;
  }
}

void process_data_flow_setup(route_ptr route, uint16_t flowid, flow_id_table_ptr routing_field, sdnaddr_t* source_request) {

  if (route == NULL) {

#ifdef SDN_SOURCE_ROUTED
    send_src_rtd_data_flow_setup((sdnaddr_t*)routing_field->source, routing_field->flowid, (sdnaddr_t*)routing_field->next_hop, routing_field->action);
#else
    sdn_send_data_flow_setup((sdnaddr_t*)routing_field->source, routing_field->flowid, (sdnaddr_t*)routing_field->next_hop, routing_field->action);
#endif

  } else {
    route_ptr route_head = route;
//TODO: comment this, used only for debug.
    while (route != NULL) {
      digraph_print_vertice_id(route->vertice_id);
      printf("(w %d h %d) <- ", route->route_weight, route->hops);

      route = route->next;
    }

    printf("\n");

    route = route_head;
//end TODO
    while (route != NULL) {

      if(route->next != NULL) {
        //Using Source Route packet to set data flow.

        routing_field = sdn_flow_id_table_add(route->next->vertice_id, flowid, route->vertice_id, SDN_ACTION_FORWARD, route->next->route_weight, route->next->hops);

        if(routing_field->changed == 1 || memcmp(routing_field->source, source_request, sizeof(sdnaddr_t)) == 0) {

#ifdef SDN_SOURCE_ROUTED
          send_src_rtd_data_flow_setup((sdnaddr_t*)routing_field->source, routing_field->flowid, (sdnaddr_t*)routing_field->next_hop, routing_field->action);
#else
          sdn_send_data_flow_setup((sdnaddr_t*)routing_field->source, routing_field->flowid, (sdnaddr_t*)routing_field->next_hop, routing_field->action);
#endif

          routing_field->changed = 0;
        }
      }

      route = route->next;
    }

    route = NULL;

    dijkstra_free_route(route_head);
  }
}

void process_control_flow_setup(route_ptr route, unsigned char *target_request, flow_address_table_ptr routing_field, sdnaddr_t* source_request) {

  if(route == NULL) {

#ifdef SDN_SOURCE_ROUTED
    send_src_rtd_control_flow_setup((sdnaddr_t*)routing_field->source, (sdnaddr_t*)routing_field->target, (sdnaddr_t*)routing_field->next_hop, routing_field->action);
#else
    sdn_send_control_flow_setup((sdnaddr_t*)routing_field->source, (sdnaddr_t*)routing_field->target, (sdnaddr_t*)routing_field->next_hop, routing_field->action);
#endif  
  } else {

    route_ptr route_head = route;
//TODO: Comment this, use only for debug.
    while (route != NULL) {
      digraph_print_vertice_id(route->vertice_id);
      printf("(w %d h %d) <- ", route->route_weight, route->hops);

      route = route->next;
    }

    printf("\n");

    route = route_head;
//end TODO
    while (route != NULL) {

      if(route->next != NULL) {

        routing_field = sdn_flow_address_table_add(route->next->vertice_id, target_request, route->vertice_id, SDN_ACTION_FORWARD, route->next->route_weight, route->next->hops);

        if(routing_field->changed == 1 || memcmp(routing_field->source, source_request, sizeof(sdnaddr_t)) == 0) {

#ifdef SDN_SOURCE_ROUTED
          send_src_rtd_control_flow_setup((sdnaddr_t*)routing_field->source, (sdnaddr_t*)routing_field->target, (sdnaddr_t*)routing_field->next_hop, routing_field->action);
#else
          sdn_send_control_flow_setup((sdnaddr_t*)routing_field->source, (sdnaddr_t*)routing_field->target, (sdnaddr_t*)routing_field->next_hop, routing_field->action);
#endif
          routing_field->changed = 0;
        }
      }
      route = route->next;
    }
    route = NULL;

    dijkstra_free_route(route_head);
  }
}

int send_src_rtd_data_flow_setup(sdnaddr_t* packet_destination, flowid_t dest_flowid, sdnaddr_t* route_nexthop, action_t action) {

  //If this packet is for me, sends the control flow setup
  if(memcmp(&sdn_node_addr, packet_destination, SDNADDR_SIZE) == 0) {
    return sdn_send_data_flow_setup(packet_destination, dest_flowid, route_nexthop, action);
  }

  //Data flow setup packet.
  sdn_data_flow_setup_t *sdn_packet = (sdn_data_flow_setup_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_src_rtd_data_flow_setup_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  //Source Routed data flow setup packet
  route_ptr route = NULL;
  route = get_better_route(sdn_node_addr.u8, packet_destination->u8, route, NULL);

  if(route == NULL) {
    printf("ERROR: route not found to send sdn source data flow setup to address: ");
    sdnaddr_print(packet_destination);
    printf("\n");

    sdn_packetbuf_pool_put((struct sdn_packetbuf *)sdn_packet);
    return SDN_ERROR;
  }

  route_ptr route_head = route;
  int count_hops = 0;

  while (route != NULL) {
    count_hops++;

    digraph_print_vertice_id(route->vertice_id);
    printf("(w %d h %d) <- ", route->route_weight, route->hops);
    
    route = route->next;
  }

  printf("\n");

  route = route_head;

  MAKE_SDN_HEADER(SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP, SDN_DEFAULT_TTL);

  flowid_copy(&sdn_packet->flowid, &dest_flowid);
  sdnaddr_copy(&sdn_packet->action_parameter, route_nexthop);
  sdn_packet->action_id = action;

  sdnaddr_copy( &((sdn_src_rtd_data_flow_setup_t*)sdn_packet)->real_destination, packet_destination);

  int index_hops = count_hops - 2;

  ((sdn_src_rtd_data_flow_setup_t*)sdn_packet)->path_len = index_hops;

// printf("Count Hops %d.\n", count_hops);

// route = route->next;

  for(;index_hops > 0; index_hops--) {

    sdnaddr_copy( (sdnaddr_t*) (((uint8_t*)sdn_packet) + packet_len), (sdnaddr_t *)route->vertice_id);
    packet_len += sizeof(sdnaddr_t);

    route = route->next;
  }

  sdnaddr_copy(&sdn_packet->destination, (sdnaddr_t *)route->vertice_id);

  route_head = NULL;
  dijkstra_free_route(route);
  
  ENQUEUE_AND_SEND(sdn_packet, packet_len);
}

int send_src_rtd_control_flow_setup(sdnaddr_t* packet_destination, sdnaddr_t* route_destination, sdnaddr_t* route_nexthop, action_t action) {

  //If this packet is for me, sends the control flow setup
  if(memcmp(&sdn_node_addr, packet_destination, SDNADDR_SIZE) == 0) {
    return sdn_send_control_flow_setup(packet_destination, route_destination, route_nexthop, action);
  }

  // SDN_DEBUG("test_send_src_rt_csf\n");
  sdn_control_flow_setup_t *sdn_packet = (sdn_control_flow_setup_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_src_rtd_control_flow_setup_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  //Source Routed data flow setup packet
  route_ptr route = NULL;
  route = get_better_route(sdn_node_addr.u8, packet_destination->u8, route, NULL);

  if(route == NULL) {
    printf("ERROR: route not found to send sdn source routed control flow setup: ");
    sdnaddr_print(&sdn_node_addr);
    printf(" -> ");
    sdnaddr_print(packet_destination);
    printf("\n");

    sdn_packetbuf_pool_put((struct sdn_packetbuf *)sdn_packet);
    return SDN_ERROR;
  }

  route_ptr route_head = route;
  int count_hops = 0;

  while (route != NULL) {
    count_hops++;

    digraph_print_vertice_id(route->vertice_id);
    printf("(w %d h %d) <- ", route->route_weight, route->hops);

    route = route->next;
  }

  printf("\n");

  route = route_head;

  MAKE_SDN_HEADER(SDN_PACKET_SRC_ROUTED_CONTROL_FLOW_SETUP, SDN_DEFAULT_TTL);
  sdnaddr_copy(&sdn_packet->route_destination, route_destination);
  sdnaddr_copy(&sdn_packet->action_parameter, route_nexthop);
  sdn_packet->action_id = action;

  sdnaddr_copy( &((sdn_src_rtd_control_flow_setup_t*)sdn_packet)->real_destination, packet_destination);

  int index_hops = count_hops - 2;

  ((sdn_src_rtd_control_flow_setup_t*)sdn_packet)->path_len = index_hops;

  for(;index_hops > 0; index_hops--) {

    sdnaddr_copy( (sdnaddr_t*) (((uint8_t*)sdn_packet) + packet_len), (sdnaddr_t *)route->vertice_id);
    packet_len += sizeof(sdnaddr_t);

    route = route->next;
  }

  sdnaddr_copy(&sdn_packet->destination, (sdnaddr_t *)route->vertice_id);

  route_head = NULL;
  dijkstra_free_route(route);
  
  ENQUEUE_AND_SEND(sdn_packet, packet_len);
}

#if SDN_TX_RELIABILITY
int send_ack_by_flow_address(sdnaddr_t* packet_destination, uint8_t acked_packed_type, uint8_t acked_packed_seqno) {

  if(sdnaddr_cmp(&sdn_node_addr, packet_destination) == SDN_EQUAL) {
//    SDN_DEBUG("Do not send ack for myself\n");
    return SDN_SUCCESS;
  }
  sdn_ack_by_flow_address_t *sdn_packet = (sdn_ack_by_flow_address_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_ack_by_flow_address_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_ACK_BY_FLOW_ADDRESS, SDN_DEFAULT_TTL);

  sdn_packet->acked_packed_type = acked_packed_type;
  sdn_packet->acked_packed_seqno = acked_packed_seqno;

  sdnaddr_copy( &((sdn_ack_by_flow_address_t*)sdn_packet)->destination, packet_destination);

  ENQUEUE_AND_SEND(sdn_packet, packet_len);
}

int send_src_rtd_ack(sdnaddr_t* packet_destination, uint8_t acked_packed_type, uint8_t acked_packed_seqno) {

  if(sdnaddr_cmp(&sdn_node_addr, packet_destination) == SDN_EQUAL) {
//    SDN_DEBUG("Do not send ack for myself\n");
    return SDN_SUCCESS;
  }
  sdn_src_rtd_ack_t *sdn_packet = (sdn_src_rtd_ack_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_src_rtd_ack_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  //Source Routed ack
  route_ptr route = NULL;
  route = get_better_route(sdn_node_addr.u8, packet_destination->u8, route, NULL);

  if(route == NULL) {
    printf("ERROR: route not found to send sdn_src_rtd_ack_t: ");
    sdnaddr_print(&sdn_node_addr);
    printf(" -> ");
    sdnaddr_print(packet_destination);
    printf("\n");

    sdn_packetbuf_pool_put((struct sdn_packetbuf *)sdn_packet);
    return SDN_ERROR;
  }

  route_ptr route_head = route;
  int count_hops = 0;
  while (route != NULL) {
    count_hops++;
    digraph_print_vertice_id(route->vertice_id);
    printf("(w %d h %d) <- ", route->route_weight, route->hops);

    route = route->next;
  }
  printf("\n");
  route = route_head;

  MAKE_SDN_HEADER(SDN_PACKET_SRC_ROUTED_ACK, SDN_DEFAULT_TTL);

  sdn_packet->acked_packed_type = acked_packed_type;
  sdn_packet->acked_packed_seqno = acked_packed_seqno;

  sdnaddr_copy( &((sdn_src_rtd_ack_t*)sdn_packet)->real_destination, packet_destination);

  int index_hops = count_hops - 2;

  ((sdn_src_rtd_ack_t*)sdn_packet)->path_len = index_hops;

  for(;index_hops > 0; index_hops--) {

    sdnaddr_copy( (sdnaddr_t*) (((uint8_t*)sdn_packet) + packet_len), (sdnaddr_t *)route->vertice_id);
    packet_len += sizeof(sdnaddr_t);

    route = route->next;
  }

  sdnaddr_copy(&sdn_packet->destination, (sdnaddr_t *)route->vertice_id);

  route_head = NULL;
  dijkstra_free_route(route);

  ENQUEUE_AND_SEND(sdn_packet, packet_len);
}
#endif //SDN_TX_RELIABILITY

void enqueue_sdn_data_flow_request_to_controller(uint8_t *source) {

  sdn_data_flow_request_t *sdn_packet = malloc(sizeof(sdn_data_flow_request_t));

  MAKE_SDN_HEADER(SDN_PACKET_DATA_FLOW_REQUEST, SDN_DEFAULT_TTL);
  sdn_packet->flowid = SDN_CONTROLLER_FLOW;
  sdn_packet->dest_flowid = SDN_CONTROLLER_FLOW;

  memcpy(sdn_packet->header.source.u8, source, SDNADDR_SIZE);

  // process_data_flow_request(sdn_packet);

  if (sdn_send_queue_enqueue((uint8_t *) sdn_packet, sizeof(sdn_data_flow_request_t)) != SDN_SUCCESS) {
    printf("ERROR: It was not possible enqueue the packet sdn_data_flow_request_t.\n");
    free(sdn_packet);
  }

}

// void enqueue_sdn_data_flow_setup(sdnaddr_t* packet_destination, flowid_t dest_flowid, sdnaddr_t* route_nexthop, action_t action) {

//   sdn_data_flow_setup_t *sdn_packet = malloc(sizeof(sdn_data_flow_setup_t));

//   if (sdn_packet == NULL) {
//     SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
//     return;
//   }

//   MAKE_SDN_HEADER(SDN_PACKET_DATA_FLOW_SETUP, SDN_DEFAULT_TTL);
//   flowid_copy(&sdn_packet->flowid, &dest_flowid);
//   sdnaddr_copy(&sdn_packet->destination, packet_destination);
//   sdnaddr_copy(&sdn_packet->action_parameter, route_nexthop);
//   sdn_packet->action_id = action;

//   if (sdn_send_queue_enqueue((uint8_t *) sdn_packet, sizeof(sdn_data_flow_setup_t)) != SDN_SUCCESS) {
//     printf("ERROR: It was not possible enqueue the packet sdn_data_flow_setup_t.\n");
//     free(sdn_packet);
//   }
// }

// void enqueue_sdn_control_flow_setup(sdnaddr_t* packet_destination, sdnaddr_t* route_destination, sdnaddr_t* route_nexthop, action_t action) {

//   sdn_control_flow_setup_t *sdn_packet = malloc(sizeof(sdn_control_flow_setup_t));

//   if (sdn_packet == NULL) {
//     SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
//     return;
//   }

//   MAKE_SDN_HEADER(SDN_PACKET_CONTROL_FLOW_SETUP, SDN_DEFAULT_TTL);
//   sdnaddr_copy(&sdn_packet->route_destination, route_destination);
//   sdnaddr_copy(&sdn_packet->destination, packet_destination);
//   sdnaddr_copy(&sdn_packet->action_parameter, route_nexthop);
//   sdn_packet->action_id = action;

//   if (sdn_send_queue_enqueue((uint8_t *) sdn_packet, sizeof(sdn_control_flow_setup_t)) != SDN_SUCCESS) {
//     printf("ERROR: It was not possible enqueue the packet sdn_control_flow_setup_t.\n");
//     free(sdn_packet);
//   }
// }
