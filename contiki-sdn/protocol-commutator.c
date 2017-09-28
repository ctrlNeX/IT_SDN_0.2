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

#include "protocol-commutator.h"
#include "sdn-network.h"
#include "sdn-debug.h"
#include "sdn-addr.h"
#include "net/mac/frame802154.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#ifdef ENERGY_MANAGER
#include "energy-manager.h"
#endif

static uint8_t *packetbuf_ptr;

uint8_t sdn_control_packet_sent_counter;
uint8_t sdn_data_packet_sent_counter;
uint8_t not_sdn_packet_sent_counter;

// Variables for lower layer access control
static uint8_t sending;
static mac_callback_t sent_cb_copy;

/**
 * Function to initialize the protocol.
 *
 */
static void
init(llsec_on_bootstrapped_t on_bootstrapped)
{
  sdn_control_packet_sent_counter = 0;
  sdn_data_packet_sent_counter = 0;
  not_sdn_packet_sent_counter = 0;
  sending = 0;
  sent_cb_copy = NULL;
  if (on_bootstrapped)
    on_bootstrapped();
}

static int
on_frame_created(void)
{
  return 1;
}

static uint8_t
get_overhead(void)
{
  return 0;
}

/**
 * Wrapper function to upper layer callbacks.
 *
 * @param ptr           Upper layer defined pointer
 * @param status        MAC status
 * @param transmissions Number of TX attempts
 */
static void
llsec_sent(void *ptr, int status, int transmissions)
{
  if (sending != 1) {
    SDN_DEBUG("Setting status to MAC_TX_ERR (sending = %d)\n", sending);
    status = MAC_TX_ERR;
  }

  if (sent_cb_copy) {
    sending = 0;
    sent_cb_copy(ptr, status, transmissions);
    sent_cb_copy = NULL;
  } else {
    SDN_DEBUG_ERROR("sent_cb_copy not defined.\n");
  }
}

/**
 * Sends the data to the radio.
 *
 * @param sent  The callback function after the message has been sent.
 * @param *ptr  The pointer to the data array to send.
 */
static void
send(mac_callback_t sent, void *ptr)
{
#ifdef ENERGY_MANAGER
  if(em_has_battery() == 0) return;
#endif

  sdn_header_t h;
  if (sending == 0) {
    sending ++;
    sent_cb_copy = sent;
    packetbuf_ptr = packetbuf_dataptr();
    // considering all non-sdn packets as ND packets, for statistics
    if (packetbuf_ptr[1] != 0xFF) {
      h.type = SDN_PACKET_ND;
      sdnaddr_copy(&(h.source), &sdn_node_addr);
      SDN_METRIC_TX(&h);
      SDN_DEBUG("Collection Protocol Sending.\n");
    }
    packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_DATAFRAME);
    NETSTACK_MAC.send(llsec_sent, ptr);
  } else {
    sending ++;
    SDN_DEBUG_ERROR("Possible data corruption (send without send_done)\n");
    sent(ptr, MAC_TX_ERR, 0);
  }
}

/**
 * Function from callback MAC to signal packet received from radio.
 *
 */
static void
input(void)
{
#ifdef ENERGY_MANAGER
  if(em_has_battery() == 0) return;
#endif

  packetbuf_ptr = packetbuf_dataptr();
  // printf("PROTOCOL-COMMUTATOR %02X %02X\n", packetbuf_ptr[0], packetbuf_ptr[1]);
  /* Filters SDN packets according to "reserved" header field. */
  if (packetbuf_ptr[1] == 0xFF) {
    sdn_network_driver.input();
  } else {
    NETSTACK_NETWORK.input();
    SDN_DEBUG("Collection Protocol Receiving.\n");
  }
}

/**
 * Struct to store protocol functions.
 */
const struct llsec_driver commutator_protocol_driver = {
  "commutator-protocol",
  init,
  send,
  on_frame_created,
  input,
  get_overhead
};
