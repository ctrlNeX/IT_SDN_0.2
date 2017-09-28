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
#include <string.h>
#include "dijkstra.h"

// #define INFINITY 32768

//#define DIJKSTRA_DEBUG

#ifdef DIJKSTRA_DEBUG
#define DEBUG_DIJKSTRA(...) printf(__VA_ARGS__);
// #define DEBUG_DIJKSTRA(format, ...) printf("%s:%d: " format, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define DEBUG_DIJKSTRA(...)
#endif /* DIJKSTRA_DEBUG */

typedef struct dijkstra_queue *dijkstra_queue_ptr;

struct dijkstra_queue {
  vertice_ptr vertice;
  dijkstra_queue_ptr next;
};

dijkstra_queue_ptr dijkstra_queue_head = NULL;
dijkstra_queue_ptr dijkstra_queue_tail = NULL;

//static int get_weight(vertice_ptr vertice, edge_ptr edge);

void dijkstra_queue_push(vertice_ptr vertice) {

  dijkstra_queue_ptr new_entry = malloc(sizeof *new_entry);
  new_entry->vertice = vertice;
  new_entry->next = NULL;

  dijkstra_queue_ptr queue_search;

  if(dijkstra_queue_head == NULL) {

    dijkstra_queue_head = new_entry;
    dijkstra_queue_tail = new_entry;
#ifdef DIJKSTRA_DEBUG
    DEBUG_DIJKSTRA("Queue insert on HEAD  (NULL)");
    digraph_print_vertice_id(new_entry->vertice->vertice_id);
    DEBUG_DIJKSTRA(" %d.\n", new_entry->vertice->route_weight);
#endif
  } else {
    queue_search = dijkstra_queue_head;

    if(new_entry->vertice->route_weight < dijkstra_queue_head->vertice->route_weight) {

      new_entry->next = dijkstra_queue_head;
      dijkstra_queue_head = new_entry;
#ifdef DIJKSTRA_DEBUG
      DEBUG_DIJKSTRA("Queue insert on HEAD [%d < %d] ", new_entry->vertice->route_weight, dijkstra_queue_head->vertice->route_weight);
      digraph_print_vertice_id(new_entry->vertice->vertice_id);
      DEBUG_DIJKSTRA(" %d.\n", new_entry->vertice->route_weight);
#endif
    } else {
      if(queue_search->next != NULL) {

        do {

          if(new_entry->vertice->route_weight < queue_search->next->vertice->route_weight) {
            new_entry->next = queue_search->next;
            queue_search->next = new_entry;
  #ifdef DIJKSTRA_DEBUG
            DEBUG_DIJKSTRA("Queue insert on middle ");
            digraph_print_vertice_id(new_entry->vertice->vertice_id);
            DEBUG_DIJKSTRA(" %d.\n", new_entry->vertice->route_weight);
  #endif
          }

          // if(queue_search->next != NULL) {
            queue_search = queue_search->next;
          // }

        } while(queue_search->next != NULL && queue_search != new_entry);
      }
    // queue_search != new_entry &&
      if(queue_search->next == NULL) {
        queue_search->next = new_entry;
        dijkstra_queue_tail = new_entry;
  #ifdef DIJKSTRA_DEBUG
        DEBUG_DIJKSTRA("Queue insert on end ");
        digraph_print_vertice_id(new_entry->vertice->vertice_id);
        DEBUG_DIJKSTRA(" %d.\n", new_entry->vertice->route_weight);
  #endif
      }
    }
  }

  // dijkstra_queue_tail = new_entry;

}

vertice_ptr dijkstra_queue_pop() {

  if(dijkstra_queue_head == NULL) {
    return NULL;
  }

  dijkstra_queue_ptr queue_head = dijkstra_queue_head;

  vertice_ptr vertice = dijkstra_queue_head->vertice;

  if(dijkstra_queue_head == dijkstra_queue_tail) {
    dijkstra_queue_head = NULL;
    dijkstra_queue_tail = NULL;
  } else {
    dijkstra_queue_head = dijkstra_queue_head->next;
  }

  free(queue_head);

  return vertice;
}

route_ptr dijkstra_shortest_path(unsigned char *vertice_id_from, unsigned char *vertice_id_to) {

  route_ptr route_head = NULL;

  int edge_weight;

  vertice_ptr vertice_root = digraph_get_vertice(vertice_id_from);

  vertice_ptr vertice = vertice_root;

  do {
#ifdef DIJKSTRA_DEBUG
    DEBUG_DIJKSTRA("Looking edges on vertice ");
    digraph_print_vertice_id(vertice->vertice_id);
    DEBUG_DIJKSTRA("\n");
#endif
    vertice->visited = 1;

    edge_ptr edge = vertice->edge_header;

    if(edge != NULL) {

      do {

        if(edge->vertice->visited == 0) {

          //edge_weight = get_weight(vertice_root, edge);
          edge_weight = edge->weight;

          if (edge->vertice->route_root == NULL) {

            edge->vertice->route_weight = vertice->route_weight + edge_weight;
            edge->vertice->route_root = vertice;
            dijkstra_queue_push(edge->vertice);
#ifdef DIJKSTRA_DEBUG
            DEBUG_DIJKSTRA("Calculating route: ");
            digraph_print_vertice_id(vertice->vertice_id);
            DEBUG_DIJKSTRA(" -> ");
            digraph_print_vertice_id(edge->vertice->vertice_id);
            DEBUG_DIJKSTRA(" [%d]\n", edge->vertice->route_weight);
#endif
          } else {
            if(vertice->route_weight + edge_weight < edge->vertice->route_weight) {
              edge->vertice->route_weight = vertice->route_weight + edge_weight;
              edge->vertice->route_root = vertice;
#ifdef DIJKSTRA_DEBUG
              DEBUG_DIJKSTRA("* Calculating route: ");
              digraph_print_vertice_id(vertice->vertice_id);
              DEBUG_DIJKSTRA(" -> ");
              digraph_print_vertice_id(edge->vertice->vertice_id);
              DEBUG_DIJKSTRA(" [%d]\n", edge->vertice->route_weight);
#endif
            }
          }
        }

        edge = edge->next;
      } while(edge != vertice->edge_header);
    }

    vertice = dijkstra_queue_pop();

  } while(vertice != NULL);

  vertice_ptr vertice_target = digraph_get_vertice(vertice_id_to);

  if(vertice_target->route_root == NULL) {

    DEBUG_DIJKSTRA("Route not found.\n");

  } else {

    DEBUG_DIJKSTRA("Route found.\n");

    DEBUG_DIJKSTRA("(%d) ", vertice_target->route_weight);

    route_ptr route_tail;
    route_ptr route;
    int hops = 0;
    int route_weight = vertice_target->route_weight;

    route = malloc(sizeof *route);
    memcpy(route->vertice_id, vertice_target->vertice_id, DIGRAPH_VERTICE_ID_SIZE);
    route->next = NULL;
    route->route_weight = vertice_target->route_weight;
    route->hops = hops;
    route->keep_route_cache = 0;

#ifdef DIJKSTRA_DEBUG
    digraph_print_vertice_id(vertice_target->vertice_id);
#endif

    route_head = route;
    route_tail = route;

    vertice_target = vertice_target->route_root;

    while(vertice_target != NULL) {

      hops++;

#ifdef DIJKSTRA_DEBUG
      DEBUG_DIJKSTRA(" <- ");
      digraph_print_vertice_id(vertice_target->vertice_id);
#endif

      route = malloc(sizeof *route);
      memcpy(route->vertice_id, vertice_target->vertice_id, DIGRAPH_VERTICE_ID_SIZE);
      route->next = NULL;
      route->route_weight = route_weight - vertice_target->route_weight;
      route->hops = hops;
      route->keep_route_cache = 0;

      route_tail->next = route;
      route_tail = route;

      vertice_target = vertice_target->route_root;
    }

    DEBUG_DIJKSTRA("\n");
  }

  /* cleaning attributes */
  vertice_ptr vertice_header = digraph_get();

  vertice = vertice_header;

  do {

    vertice->visited = 0;
    vertice->route_weight = 0;
    vertice->route_root = NULL;

    vertice = vertice->next;

  } while(vertice != vertice_header);

  return route_head;

}

void dijkstra_free_route(route_ptr route) {

  route_ptr route_remove;

  while (route != NULL) {

    route_remove = route;
    route = route->next;

    free(route_remove);

  }

}

// static int get_weight(vertice_ptr vertice, edge_ptr edge) {

//   float etx;
//   int weight;
//   float energy;

//   //Gets energy smaller value
//   if(vertice->energy < edge->vertice->energy) {
//     energy = vertice->energy;
//   } else {
//     energy = edge->vertice->energy;
//   }

//   //Scale ETX [0,1]
//   //etx = edge->etx / 65535;
//   //for tests compatibility with tinySDN
//   if (edge->etx > 1000) {
//     etx = 1;
//   } else {
//     etx = edge->etx / 1000;
//   }

//   //Scale Energy [0,1] and invert
//   energy = 1 - (energy / 255);

//   weight = (etx * WEIGHT_ETX + energy * WEIGHT_ENERGY) * 100;

//   return weight;
// }
