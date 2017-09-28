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

/**
 * \file
 *         ...
 * \author
 *         Renan C. A. Alves <ralves@larc.usp.br>
 */

/**
 * \addtogroup SDN
 * @{
 * \addtogroup SDN-contiki
 * @{
 * \defgroup sdn-nt SDN neighbor table
 * @{
 *
 */

#ifndef SDN_NEIGHBOR_TABLE_H_
#define SDN_NEIGHBOR_TABLE_H_

#include "sdn-addr.h"
#include "sdn-constants.h"

enum sdn_neighbor_status {
  SDN_NEIGHBOR_STATIC = 0,
  SDN_NEIGHBOR_EDIT = 1,
  SDN_NEIGHBOR_CHANGED = 2
};

struct sdn_neighbor_entry {
  /* The ->next pointer is needed since we are placing these on a
     Contiki list. */
  struct sdn_neighbor_entry *next;

  sdnaddr_t neighbor_addr;
  uint16_t metric;
  uint8_t status;
};  

void sdn_neighbor_table_edit();

uint8_t sdn_neighbor_table_update();

/**
 * \brief      Initializes the SDN neighbor table
 * \retval     SDN_SUCCESS or SDN_ERROR
 *
 */
uint8_t sdn_neighbor_table_init();

/**
 * \brief               Insert or updates an entry in the the SDN neighbor table
 * \param neighbor_addr Neighbor SDN address
 * \param metric        Metric value associated to the neighbor
 * \retval SDN_SUCCESS  Entry was added or updated
 * \retval SDN_ERROR    It was not possible to create new entry
 *
 */
uint8_t sdn_neighbor_table_insert(sdnaddr_t neighbor_addr, uint16_t metric);

/**
 * \brief Remove an entry in the the SDN neighbor table based on address
 * \param neighbor_addr Neighbor SDN address
 * \retval SDN_SUCCESS  Entry was removed
 * \retval SDN_ERROR    Entry not found
 *
 */
uint8_t sdn_neighbor_table_remove(sdnaddr_t neighbor_addr);

/**
 * \brief Retrieve neighbor information from SDN neighbor table
 * \param neighbor_addr Neighbor SDN address
 * \retval  Non-NULL if the entry was found, NULL otherwise
 *
 */
struct sdn_neighbor_entry * sdn_neighbor_table_get(sdnaddr_t neighbor_addr);

/**
 * \brief Counts the number of neighbors in the SDN neighbor table
 *
 */
uint8_t sdn_neighbor_table_count();

/**
 * \brief Retrieve first element from SDN neighbor table
 * \retval  Non-NULL if table is not empty, NULL otherwise
 *
 */
struct sdn_neighbor_entry * sdn_neighbor_table_head();

/**
 * \brief Retrieve next element from SDN neighbor table
 * \param neighbor_addr Neighbor SDN address
 * \retval  Non-NULL if there is another element, NULL otherwise
 *
 */
struct sdn_neighbor_entry * sdn_neighbor_table_next(sdnaddr_t neighbor_addr);

/**
 * \brief Prints all neighbor data, for debugging purposes
 *
 */
void sdn_neighbor_table_print();

#endif //SDN_NEIGHBOR_TABLE_H_

/** @} */
/** @} */
/** @} */
