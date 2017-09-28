/*
* Header Here!
*/

/*
 * Page used for otimizing size compilation.
 * https://github.com/contiki-os/contiki/wiki/Reducing-Contiki-OS-firmware-size
 */

/* Disable the name strings from being stored. */
#define PROCESS_CONF_NO_PROCESS_NAMES 1

/* Reducing uIP/IPv6 Stack Size by not using TCP */
#define UIP_CONF_TCP 0

/* Reducing uIP/IPv6 Stack Size by not using UDP */
// #define UIP_CONF_UDP 0

/*
 * The RDC protocols require additional program and RAM to
 * store neighbor information. Reduce the sequence number
 * arrray size used for duplicate packet detection.
 * Turn off phase optimization in contikimac to eliminate
 * the neighbor wake time table
 */
#ifdef NETSTACK_CONF_RDC
#undef NETSTACK_CONF_RDC
#endif

/*#define SDN_CONF_ND dummy_sdn_nd*/
#define SDN_CONF_ND collect_sdn_nd
/*#define SDN_CONF_ND collect_sdn_nd*/
/*#define SDN_CONF_CD null_sdn_cd*/
#define SDN_CONF_CD null_sdn_cd

// Need to uncomment this line if "CONTIKI_WITH_RIME = 0" in Makefile
// #undef NETSTACK_CONF_NETWORK
// #define NETSTACK_CONF_NETWORK sdn_network_driver

#define NETSTACK_CONF_RDC nullrdc_driver
#ifdef NETSTACK_CONF_FRAMER
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154
#endif
// #define NETSTACK_CONF_RDC contikimac_driver

#define SDN_CONF_TX_RELIABILITY 1
// #define NULLRDC_CONF_802154_AUTOACK_HW 0
// #define NULLRDC_CONF_802154_AUTOACK_HW 1
// #undef NULLRDC_CONF_802154_AUTOACK
// #define NULLRDC_CONF_802154_AUTOACK 1
/*#define RDC_CONF_HARDWARE_ACK 0*/
/*#ifdef CC2420_CONF_AUTOACK
#undef CC2420_CONF_AUTOACK
#endif
#define CC2420_CONF_AUTOACK 0*/

#define NETSTACK_CONF_LLSEC commutator_protocol_driver

/* Size of SDN packetbuf pool. */
#define SDN_PACKETBUF_POOL_SIZE 12
/* Size of SDN send queue. */
#define SDN_SEND_QUEUE_SIZE 10
/* Size of SDN recv queue. */
#define SDN_RECV_QUEUE_SIZE 10

/* The Max size of the SDN packet is 127 bytes (MTU of 802.15.4) - 12 bytes:
  10 bytes (Header of 802.15.4: 2 bytes frame control field, 2 bytes PAN ID,
     1 byte seq num, 2 bytes src addr, 2 bytes dst addr);
  2 bytes (Footer of 802.15.4: CRC). */
#define SDN_MAX_PACKET_SIZE (127 - 12)
/**
* Source: $(CONTIKI)/core/net/ipv6/uip-nd6.h
*
* The time that a neighbor remains with status REACHABLE.

#ifdef UIP_CONF_ND6_REACHABLE_TIME
#undef UIP_CONF_ND6_REACHABLE_TIME
#endif
#define UIP_CONF_ND6_REACHABLE_TIME	30000
*/
