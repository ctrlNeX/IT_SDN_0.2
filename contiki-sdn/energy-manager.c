#include "contiki.h"
#include <stdio.h>
#include "sys/energest.h"
#include <inttypes.h>
#include "sdn-debug.h"
#include "sys/etimer.h"
// #include "sdn-neighbor-table.h"
#include "sdn-send-packet.h"
#include "energy-manager.h"
#include "net/netstack.h"

#ifndef MAX_ENERGY_NODE
#define MAX_ENERGY_NODE 24UL
// 2100L - energy in mAh (Duracell battery reference)
#endif

//#define ENERGY_MANAGER_SIMULATE_BATTERY

#define REPORT_BATTERY_LEVEL_EVERY_PERCENT 2

//TODO: Define dinamically the time for energy estimate related to REPORT_BATTERY_LEVEL_EVERY_PERCENT
#define TIME_TO_ENERGY_ESTIMATE_IN_SECONDS 1

#define Vs 3

// static void estimate_energy();
#ifdef ENERGY_MANAGER

static void time_to_estimate(void * ptr);

#endif //ENERGY_MANAGER

unsigned long energy_consumed;

unsigned long energy;

struct ctimer periodic_timer;

float cpu_i = 2.33;
float lpm_i = 0.180;
float rx_i = 22;
float tx_i = 21.7;

uint8_t battery_level_previous;
uint8_t battery_level_last;
uint8_t battery_level_trigger;

extern struct process sdn_core_process;

unsigned long get_consumed_energy();

void em_init() {

#ifdef ENERGY_MANAGER

  SDN_DEBUG("Initializing Energy Manager module.\n");
  SDN_DEBUG("Max Energy Node %lu mJ\n", MAX_ENERGY_NODE);

  energy = MAX_ENERGY_NODE; //mJ

  battery_level_last = em_get_battery(); //battery_level;

  battery_level_previous = 0;

  battery_level_trigger = (uint8_t) (MAX_BATTERY_LEVEL * REPORT_BATTERY_LEVEL_EVERY_PERCENT / 100);

  ctimer_set(&periodic_timer, TIME_TO_ENERGY_ESTIMATE_IN_SECONDS * CLOCK_SECOND, time_to_estimate, &periodic_timer);

#endif //ENERGY_MANAGER
}

uint8_t em_get_battery() {

#ifdef ENERGY_MANAGER

  if(energy > get_consumed_energy()) {

    return (uint8_t) ( (energy - get_consumed_energy()) * MAX_BATTERY_LEVEL / energy);

  } else {

    return 0;
  }
#else
  return MAX_BATTERY_LEVEL;
#endif
}

unsigned long get_consumed_energy() {

  energest_flush();

  return (unsigned long) ((tx_i * energest_type_time(ENERGEST_TYPE_TRANSMIT) / RTIMER_ARCH_SECOND) * Vs);
}

#ifdef ENERGY_MANAGER

static void time_to_estimate(void * ptr) {

#ifdef ENERGY_MANAGER_SIMULATE_BATTERY
  if(battery_level_previous != em_get_battery()) {
    SDN_DEBUG("Battery level %u / 255\n", em_get_battery());
  }

  if(battery_level_last - em_get_battery() > battery_level_trigger) {

    sdn_send_energy_report();

    battery_level_last = em_get_battery();
  }

  if(em_get_battery() > 0UL) {
    ctimer_reset((struct ctimer*) ptr);
  } else {
    printf("SHUTDOWN RADIO\n");
    printf("#A color=RED\n");
    NETSTACK_RDC.off(0);
  }

  battery_level_previous = em_get_battery();
#else
  battery_level_previous++;

  if(battery_level_previous >= 60) {
    battery_level_previous = 0;
    //SDN_DEBUG("Energy consumed: %lu mJ\n", get_consumed_energy());
    SDN_METRIC_ENERGY("E: %lu mJ\n", get_consumed_energy());
  }
  ctimer_reset((struct ctimer*) ptr);

#endif //ENERGY_MANAGER_SIMULATE_BATTERY

}
#endif //ENERGY_MANAGER

uint8_t em_has_battery() {
#ifdef ENERGY_MANAGER_SIMULATE_BATTERY
  return (em_get_battery() > 0UL)? 1 : 0;
#else
  return 1;
#endif //ENERGY_MANAGER
}
