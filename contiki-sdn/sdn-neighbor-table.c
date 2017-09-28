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

#include "lib/memb.h"
#include "lib/list.h"
#include "sdn-neighbor-table.h"
#include "stdio.h"

#ifndef SDN_CONF_NEIGHBOR_TABLE_SIZE
#define SDN_NEIGHBOR_TABLE_SIZE 10
#else //SDN_CONF_NEIGHBOR_TABLE_SIZE
#define SDN_NEIGHBOR_TABLE_SIZE SDN_CONF_NEIGHBOR_TABLE_SIZE
#endif//SDN_CONF_NEIGHBOR_TABLE_SIZE

#define SDN_NEIGHBOR_ETX_SENSIBILITY 200 //in percent
#define SDN_NEIGHBOR_ETX_DELTA 100 //

/* Declaring MEMB to hold data or control flow table entries. */
MEMB(neighbor_table_memb, struct sdn_neighbor_entry, SDN_NEIGHBOR_TABLE_SIZE);

LIST(sdn_neighbor_table);

uint8_t sdn_neighbor_table_init() {
    list_init(sdn_neighbor_table);
    return SDN_SUCCESS;
}

void sdn_neighbor_table_edit() {

  struct sdn_neighbor_entry *n;

  for(n = list_head(sdn_neighbor_table); n != NULL; n = list_item_next(n)) {

    n->status = SDN_NEIGHBOR_EDIT;
  }
}

uint8_t sdn_neighbor_table_update() {

  uint8_t ret = SDN_ERROR;

  struct sdn_neighbor_entry *neighbor;

  struct sdn_neighbor_entry *neighbor_next;

  for(neighbor = list_head(sdn_neighbor_table); neighbor != NULL; ) {

    if(neighbor->status == SDN_NEIGHBOR_EDIT) {

      neighbor_next = list_item_next(neighbor);

      list_remove(sdn_neighbor_table, neighbor);
      memb_free(&neighbor_table_memb, neighbor);

      ret = SDN_SUCCESS;

      neighbor = neighbor_next;

    } else {
      if(neighbor->status == SDN_NEIGHBOR_CHANGED) ret = SDN_SUCCESS;

      neighbor = list_item_next(neighbor);
    }
  }

  return ret;
}

uint8_t sdn_neighbor_table_insert(sdnaddr_t neighbor_addr, uint16_t metric) {
  struct sdn_neighbor_entry *n;

  /* Check if the entry already exists. */
  n = sdn_neighbor_table_get(neighbor_addr);

  /* If n is NULL, this entry was not found in our list, and we allocate
  a new struct memb memory pool. */
  if(n == NULL) {
    n = memb_alloc(&neighbor_table_memb);

    /* If we could not allocate a new entry, we give up. */
    if(n == NULL) {
      return SDN_ERROR;
    }

    n->metric = metric;
    n->status = SDN_NEIGHBOR_CHANGED;
  } else {

    // printf("XXX %d %2d \n", (n->metric * ( 1.0 + SDN_NEIGHBOR_ETX_SENSIBILITY / 100.0 )) * 100, (n->metric / ( 1.0 + SDN_NEIGHBOR_ETX_SENSIBILITY / 100.0 ))*100);

    if ( (metric < n->metric - SDN_NEIGHBOR_ETX_DELTA || metric > n->metric + SDN_NEIGHBOR_ETX_DELTA) &&
      (metric > n->metric * ( 1.0 + SDN_NEIGHBOR_ETX_SENSIBILITY / 100.0 ) || metric < n->metric / ( 1.0 + SDN_NEIGHBOR_ETX_SENSIBILITY / 100.0 ))) {

      n->metric = metric;
      n->status = SDN_NEIGHBOR_CHANGED;
    } else {

      n->status = SDN_NEIGHBOR_STATIC;
    }
  }

  /* Initialize the fields. */
  sdnaddr_copy(&n->neighbor_addr, &neighbor_addr);

  /* Place the entry in the data flow table list. */
  list_add(sdn_neighbor_table, n);

  return SDN_SUCCESS;
}

uint8_t sdn_neighbor_table_remove(sdnaddr_t neighbor_addr) {
  struct sdn_neighbor_entry *n;
  n = sdn_neighbor_table_get(neighbor_addr);

  if (n != NULL) {
    list_remove(sdn_neighbor_table, n);
    memb_free(&neighbor_table_memb, n);
    return SDN_SUCCESS;
  }

  return SDN_ERROR;
}

struct sdn_neighbor_entry * sdn_neighbor_table_get(sdnaddr_t neighbor_addr) {
  struct sdn_neighbor_entry *n;

  for(n = list_head(sdn_neighbor_table); n != NULL; n = list_item_next(n)) {
    /* We break out of the loop if the address matches */
    if(sdnaddr_cmp(&n->neighbor_addr, &neighbor_addr) == SDN_EQUAL) {
      return n;
    }
  }

  return NULL;
}

uint8_t sdn_neighbor_table_count() {
  struct sdn_neighbor_entry *n;
  uint8_t i = 0;

  for(n = list_head(sdn_neighbor_table); n != NULL; n = list_item_next(n)) {
    i++;
  }

  return i;
}

struct sdn_neighbor_entry * sdn_neighbor_table_head() {
  return list_head(sdn_neighbor_table);
}

struct sdn_neighbor_entry * sdn_neighbor_table_next(sdnaddr_t neighbor_addr) {
  return list_item_next(sdn_neighbor_table_get(neighbor_addr));
}

void sdn_neighbor_table_print() {
#ifdef DEBUG_SDN
  struct sdn_neighbor_entry *n;
  printf("neighbor, metric, next ptr\n");
  for(n = list_head(sdn_neighbor_table); n != NULL; n = list_item_next(n)) {
    printf("(");
    sdnaddr_print(&n->neighbor_addr);
    printf(") (%02d) (%p)\n", n->metric, n->next);
  }
#endif
}
