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

#include "sdn-send-packet.h"
#include "sdn-packetbuf.h"
#include "sdn-debug.h"
#include "sdn-constants.h"
#include "sdn-queue.h"
#include "string.h"
#ifndef SDN_CONTROLLER_PC
#include "sdn-neighbor-table.h"
#include "sdn-send.h"

#ifdef ENERGY_MANAGER
#include "energy-manager.h"
#endif // ENERGY_MANAGER

#else // SDN_CONTROLLER_PC
#include "stdlib.h"
#include "sdn-serial-send.h"
#endif // SDN_CONTROLLER_PC

#ifdef SDN_CONTROLLER_NODE
#include "sdn-serial-packet.h"
#include "sdn-serial.h"
#endif // SDN_CONTROLLER_NODE



#ifdef DEMO
#include "leds.h"
#endif // DEMO

#ifndef SDN_CONTROLLER_PC
static uint8_t sdn_neighbor_locked = 0;
#endif

uint8_t sdn_seq_no = 0;

uint8_t sdn_send_control_flow_setup(sdnaddr_t* packet_destination, sdnaddr_t* route_destination, sdnaddr_t* route_nexthop, action_t action) {
  sdn_control_flow_setup_t *sdn_packet = (sdn_control_flow_setup_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_control_flow_setup_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_CONTROL_FLOW_SETUP, SDN_DEFAULT_TTL);
  sdnaddr_copy(&sdn_packet->route_destination, route_destination);
  sdnaddr_copy(&sdn_packet->destination, packet_destination);
  sdnaddr_copy(&sdn_packet->action_parameter, route_nexthop);
  sdn_packet->action_id = action;

  //printf("Sending packet: ");
  //print_packet((uint8_t *)sdn_packet, packet_len);

  ENQUEUE_AND_SEND(sdn_packet, packet_len);
}

uint8_t sdn_send_data_flow_setup(sdnaddr_t* packet_destination, flowid_t dest_flowid, sdnaddr_t* route_nexthop, action_t action) {
  sdn_data_flow_setup_t *sdn_packet = (sdn_data_flow_setup_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_data_flow_setup_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_DATA_FLOW_SETUP, SDN_DEFAULT_TTL);
  flowid_copy(&sdn_packet->flowid, &dest_flowid);
  sdnaddr_copy(&sdn_packet->destination, packet_destination);
  sdnaddr_copy(&sdn_packet->action_parameter, route_nexthop);
  sdn_packet->action_id = action;

  ENQUEUE_AND_SEND(sdn_packet, packet_len);
}

uint8_t sdn_send_control_flow_request(sdnaddr_t address) {
  sdn_control_flow_request_t *sdn_packet = (sdn_control_flow_request_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_control_flow_request_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_CONTROL_FLOW_REQUEST, SDN_DEFAULT_TTL);
  sdnaddr_copy(&sdn_packet->address, &address);
  flowid_copy(&sdn_packet->dest_flowid, &sdn_controller_flow);

  ENQUEUE_AND_SEND(sdn_packet, packet_len);
}

uint8_t sdn_send_data_flow_request(flowid_t flowid) {
  sdn_data_flow_request_t *sdn_packet = (sdn_data_flow_request_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_data_flow_request_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_DATA_FLOW_REQUEST, SDN_DEFAULT_TTL);
  flowid_copy(&sdn_packet->flowid, &flowid);
  flowid_copy(&sdn_packet->dest_flowid, &sdn_controller_flow);

  ENQUEUE_AND_SEND(sdn_packet, packet_len);
}

#ifndef SDN_CONTROLLER_PC
uint8_t sdn_send_neighbor_report() {
  sdn_neighbor_report_t* sdn_packet;
  uint16_t packet_len = sizeof(sdn_neighbor_report_t);
  struct sdn_neighbor_entry* sne;
  sdn_neighbor_report_list_t nrl;

#ifdef SDN_ENABLED_NODE
  if (sdn_neighbor_locked) {
    SDN_DEBUG("Could not send SDN_PACKET_NEIGHBOR_REPORT: locked\n");
    return SDN_ERROR;
  }
#endif

  if (sdn_neighbor_table_count() == 0) {
    SDN_DEBUG("No neighbors to send.\n");
    return SDN_ERROR;
  }

  sdn_packet = (sdn_neighbor_report_t *) sdn_packetbuf_pool_get();

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_NEIGHBOR_REPORT, SDN_DEFAULT_TTL);
  sdn_packet->flowid = SDN_CONTROLLER_FLOW;
  sdn_packet->num_of_neighbors = 0;

  for (sne = sdn_neighbor_table_head(); sne != NULL && packet_len < SDN_MAX_PACKET_SIZE; \
       sne = sdn_neighbor_table_next(sne->neighbor_addr)) {
    // copy_info
    sdnaddr_copy(&nrl.neighbor, &sne->neighbor_addr);
    nrl.etx = sne->metric;
    memcpy(((uint8_t *)sdn_packet) + packet_len, (uint8_t *) &nrl, sizeof(nrl));
    sdn_packet->num_of_neighbors ++;
    packet_len += sizeof(sdn_neighbor_report_list_t);
  }

  if (sne != NULL) {
    SDN_DEBUG_ERROR("Too many neighbors to fit in a packet.\n");
  }

#if defined(SDN_ENABLED_NODE) && SDN_TX_RELIABILITY
  sdn_neighbor_locked = 1;
#endif

#ifdef SDN_ENABLED_NODE
  #ifdef DEMO
  printf("LED BLUE NEIGHBOR\n");
  leds_toggle(LEDS_BLUE);
  #endif

  ENQUEUE_AND_SEND(sdn_packet, packet_len);
#elif defined(SDN_CONTROLLER_NODE)
  sdn_serial_packet_t serial_pkt;

  memcpy(serial_pkt.payload, (uint8_t *)sdn_packet, packet_len);

  sdn_packetbuf_pool_put((sdn_packetbuf*)sdn_packet);

  serial_pkt.header.msg_type = SDN_SERIAL_MSG_TYPE_RADIO;
  serial_pkt.header.payload_len = packet_len;

  sdn_serial_send(&serial_pkt);
#endif

  return SDN_SUCCESS;
}

void sdn_send_unlock_neighbor() {
  sdn_neighbor_locked = 0;
}

#ifdef ENERGY_MANAGER
uint8_t sdn_send_energy_report() {

  sdn_energy_report_t* sdn_packet;
  uint16_t packet_len = sizeof(sdn_energy_report_t);

  sdn_packet = (sdn_energy_report_t *) sdn_packetbuf_pool_get();

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_ENERGY_REPORT, SDN_DEFAULT_TTL);
  sdn_packet->flowid = SDN_CONTROLLER_FLOW;

  sdn_packet->energy = em_get_battery();

#ifdef SDN_ENABLED_NODE
  ENQUEUE_AND_SEND(sdn_packet, packet_len);
#elif defined(SDN_CONTROLLER_NODE)
  sdn_serial_packet_t serial_pkt;

  memcpy(serial_pkt.payload, (uint8_t *)sdn_packet, packet_len);

  sdn_packetbuf_pool_put((sdn_packetbuf*)sdn_packet);

  serial_pkt.header.msg_type = SDN_SERIAL_MSG_TYPE_RADIO;
  serial_pkt.header.payload_len = packet_len;

  sdn_serial_send(&serial_pkt);
#endif

  return SDN_SUCCESS;
}
#endif

uint8_t sdn_send_register_flowid(flowid_t flowid) {
  sdn_data_flow_request_t* sdn_packet = (sdn_data_flow_request_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_data_flow_request_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_REGISTER_FLOWID, SDN_DEFAULT_TTL)

  flowid_copy(&sdn_packet->flowid, &flowid);
  flowid_copy(&sdn_packet->dest_flowid, &sdn_controller_flow);

  ENQUEUE_AND_SEND(sdn_packet, packet_len);

  return SDN_SUCCESS;
}

#endif // SDN_CONTROLLER_PC

uint8_t sdn_send_data_flow_setup_ack(__attribute__((unused)) sdnaddr_t* packet_destination, __attribute__((unused)) flowid_t dest_flowid, __attribute__((unused)) sdnaddr_t* route_nexthop, __attribute__((unused)) action_t action) {

//}
//uint8_t sdn_send_ack(uint8_t seq_no, uint8_t packet_type) {

#ifdef SDN_ENABLED_NODE
  sdn_data_flow_setup_t *sdn_packet = (sdn_data_flow_setup_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_data_flow_setup_t) + sizeof(sdn_ack_by_flow_id_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_DATA_FLOW_SETUP, SDN_DEFAULT_TTL)
  flowid_copy(&sdn_packet->flowid, &dest_flowid);
  sdnaddr_copy(&sdn_packet->destination, packet_destination);
  sdnaddr_copy(&sdn_packet->action_parameter, route_nexthop);
  sdn_packet->action_id = action;

  // SDN_DEBUG("### ACK FLOW SETUP: flowid %u action %u destination ", sdn_packet->flowid, sdn_packet->action_id);
  // sdnaddr_print(&sdn_packet->destination);
  // SDN_DEBUG("\n");

  memmove((uint8_t *)sdn_packet + sizeof(sdn_ack_by_flow_id_t), (uint8_t *)sdn_packet, sizeof(sdn_data_flow_setup_t));

  MAKE_SDN_HEADER(SDN_PACKET_ACK_BY_FLOW_ID, SDN_DEFAULT_TTL)

  sdn_ack_by_flow_id_t *sdn_packet_ack = (sdn_ack_by_flow_id_t *) sdn_packet;

  flowid_copy(&(sdn_packet_ack->dest_flowid), &sdn_controller_flow);

  ENQUEUE_AND_SEND(sdn_packet, packet_len);
#endif

  return SDN_SUCCESS;
}

uint8_t sdn_send_control_flow_setup_ack(__attribute__((unused)) sdnaddr_t* packet_destination, __attribute__((unused)) sdnaddr_t* route_destination, __attribute__((unused)) sdnaddr_t* route_nexthop, __attribute__((unused)) action_t action) {

#ifdef SDN_ENABLED_NODE
  sdn_control_flow_setup_t *sdn_packet = (sdn_control_flow_setup_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_control_flow_setup_t) + sizeof(sdn_ack_by_flow_id_t);

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_CONTROL_FLOW_SETUP, SDN_DEFAULT_TTL);
  sdnaddr_copy(&sdn_packet->route_destination, route_destination);
  sdnaddr_copy(&sdn_packet->destination, packet_destination);
  sdnaddr_copy(&sdn_packet->action_parameter, route_nexthop);
  sdn_packet->action_id = action;

  memmove((uint8_t *)sdn_packet + sizeof(sdn_ack_by_flow_id_t), (uint8_t *)sdn_packet, sizeof(sdn_data_flow_setup_t));

  MAKE_SDN_HEADER(SDN_PACKET_ACK_BY_FLOW_ID, SDN_DEFAULT_TTL)

  sdn_ack_by_flow_id_t *sdn_packet_ack = (sdn_ack_by_flow_id_t *) sdn_packet;

  flowid_copy(&(sdn_packet_ack->dest_flowid), &sdn_controller_flow);

  ENQUEUE_AND_SEND(sdn_packet, packet_len);
#endif

  return SDN_SUCCESS;
}

uint8_t sdn_send_data(uint8_t *data, uint16_t len, flowid_t flowid) {
  sdn_data_t* sdn_packet = (sdn_data_t *) sdn_packetbuf_pool_get();
  uint16_t packet_len = sizeof(sdn_data_t) + len;

  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
    return SDN_ERROR;
  }

  MAKE_SDN_HEADER(SDN_PACKET_DATA, SDN_DEFAULT_TTL)

  flowid_copy(&sdn_packet->flowid, &flowid);

  memcpy(((uint8_t *)sdn_packet) + sizeof(sdn_data_t), data, len);

  ENQUEUE_AND_SEND(sdn_packet, packet_len);

  return SDN_SUCCESS;
}

uint8_t sdn_send_nd(struct sdn_packetbuf* sdn_nd_packet, uint16_t packet_len) {
  sdn_data_t* sdn_packet = (sdn_data_t*)sdn_nd_packet;
  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("Null packet.\n");
    return SDN_ERROR;
  }
  MAKE_SDN_HEADER(SDN_PACKET_ND, 1)
  ENQUEUE_AND_SEND(sdn_packet, packet_len);
  return SDN_SUCCESS;
}

uint8_t sdn_send_cd(struct sdn_packetbuf* sdn_cd_packet, uint16_t packet_len) {
  sdn_data_t* sdn_packet = (sdn_data_t*)sdn_cd_packet;
  if (sdn_packet == NULL) {
    SDN_DEBUG_ERROR ("Null packet.\n");
    return SDN_ERROR;
  }
  MAKE_SDN_HEADER(SDN_PACKET_CD, 1)
  ENQUEUE_AND_SEND(sdn_packet, packet_len);
  return SDN_SUCCESS;
}
