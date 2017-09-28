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

#include "sdn-protocol.h"
#include "sdn-receive.h"
#include "net/packetbuf.h"
#include "sdn-debug.h"
#include "sdn-packetbuf.h"
#include "sdn-constants.h"
#include "sdn-queue.h"
#include "sdn-send.h"
#include "sdn-process-packets.h"

extern struct process sdn_core_process;

#ifdef SDN_ENABLED_NODE
void sdn_receive() {

  int index;
  sdn_packetbuf* sdn_packet;
  uint8_t rcv_queue_is_empty;

  SDN_DEBUG ("Packet Received from [%02X", packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0]);

  for (index = 1; index < SDNADDR_SIZE; index++) {
    SDN_DEBUG (":%02X", packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[index]);
  }

  SDN_DEBUG ("] ");

  print_packet((uint8_t*)packetbuf_dataptr(), packetbuf_datalen());

  if (sdn_seqno_is_duplicate(SDN_HEADER(packetbuf_dataptr())) == SDN_YES) {
    SDN_DEBUG("Packet is duplicated, dropping.\n");
    return;
  }
  sdn_packet = sdn_packetbuf_pool_get();
  if (sdn_packet == NULL) { \
    SDN_DEBUG_ERROR ("SDN packetbuf pool is empty.\n");
  } else {
    memcpy(sdn_packet, packetbuf_dataptr(), packetbuf_datalen());
    //TODO: DAGO colocar essa verificação antes de reservar memória do pool
    if (SDN_HEADER(sdn_packet)->thl == 0) {
      SDN_DEBUG ("THL expired.\n");
      sdn_packetbuf_pool_put(sdn_packet);
      return;
    }
    SDN_HEADER(sdn_packet)->thl --;

    // if(SDN_HEADER(sdn_packet)->type == SDN_PACKET_DATA) {
    rcv_queue_is_empty = sdn_recv_queue_empty();

      if (sdn_recv_queue_enqueue((uint8_t *) sdn_packet, packetbuf_datalen()) != SDN_SUCCESS) {
        sdn_packetbuf_pool_put(sdn_packet);
        SDN_DEBUG_ERROR ("Error on packet enqueue.\n");
      } else {
        sdn_seqno_register(SDN_HEADER(sdn_packet));


        if(rcv_queue_is_empty == 1) {
          process_post(&sdn_core_process, SDN_EVENT_NEW_PACKET, 0);
        }
      }
    // } else {
    //   sdnaddr_t *addr_ptr;
    //   addr_ptr = sdn_treat_packet((uint8_t *)sdn_packet, packetbuf_datalen());
    //   if (addr_ptr != NULL) {
    //     if (sdn_send_queue_enqueue((uint8_t *) sdn_packet, packetbuf_datalen()) == SDN_SUCCESS) {
    //       sdn_seqno_register(SDN_HEADER(sdn_packet));
    //       sdn_send_down_once();
    //     } else {
    //       SDN_DEBUG_ERROR ("Error on packet enqueue.\n");
    //       sdn_packetbuf_pool_put(sdn_packet);
    //     }
    //   } else {
    //     sdn_seqno_register(SDN_HEADER(sdn_packet));
    //     sdn_packetbuf_pool_put(sdn_packet);
    //   }
    // }
  }

}
#endif

struct seqno {
  sdnaddr_t sender;
  uint8_t seqno;
};


// Code bellow adapted from mac-sequence.c file (from Contiki core)
static struct seqno received_seqnos[SDN_MAX_SEQNOS];

/*---------------------------------------------------------------------------*/
int
sdn_seqno_is_duplicate(sdn_header_t *sdnhdr)
{
  int i;

  if(sdnaddr_cmp(&sdnhdr->source, &sdn_node_addr) == SDN_EQUAL) {
    SDN_DEBUG("### Packet Duplicated by return to source\n.");
    return SDN_YES;
  }
  /*
   * Check for duplicate packet by comparing the sequence number of the incoming
   * packet with the last few ones we saw.
   */
  for(i = 0; i < SDN_MAX_SEQNOS; ++i) {
    if(sdnaddr_cmp(&sdnhdr->source, &received_seqnos[i].sender) == SDN_EQUAL) {
      if(sdnhdr->seq_no == received_seqnos[i].seqno) {
        /* Duplicate packet. */
        return SDN_YES;
      }
      break;
    }
  }
  return SDN_NO;
}
/*---------------------------------------------------------------------------*/
void
sdn_seqno_register(sdn_header_t *sdnhdr)
{
  int i, j;

  /* Locate possible previous sequence number for this address. */
  for(i = 0; i < SDN_MAX_SEQNOS; ++i) {
    if(sdnaddr_cmp(&sdnhdr->source, &received_seqnos[i].sender) == SDN_EQUAL) {
      i++;
      break;
    }
  }

  for(j = i - 1; j > 0; --j) {
    memcpy(&received_seqnos[j], &received_seqnos[j - 1], sizeof(struct seqno));
  }
  received_seqnos[0].seqno = sdnhdr->seq_no;
  sdnaddr_copy(&received_seqnos[0].sender, &sdnhdr->source);
}
/*---------------------------------------------------------------------------*/
void sdn_seqno_print()
{
  int i;
  for(i = 0; i < SDN_MAX_SEQNOS; ++i) {
    printf("(%d) ", i);
    sdnaddr_print(&received_seqnos[i].sender);
    printf(" => %d\n", received_seqnos[i].seqno);
  }
}
