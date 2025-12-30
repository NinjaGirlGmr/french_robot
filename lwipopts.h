#pragma once

// Minimal LWIP options for Pico W Wi-Fi (threadsafe background arch).
// Based on Pico SDK example defaults.

#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0

#define LWIP_IPV4 1
#define LWIP_IPV6 0
#define LWIP_DHCP 1
#define LWIP_UDP 1
#define LWIP_TCP 1
#define LWIP_ICMP 1
#define LWIP_DNS 1

#define LWIP_TCPIP_CORE_LOCKING_INPUT 1
#define SYS_LIGHTWEIGHT_PROT 1

#define MEM_ALIGNMENT 4
#define MEM_SIZE 4000
#define MEMP_NUM_TCP_PCB 5
#define PBUF_POOL_SIZE 8

#define TCP_MSS 1460
#define TCP_SND_BUF (8 * TCP_MSS)
#define TCP_SND_QUEUELEN 16
#define MEMP_NUM_TCP_SEG TCP_SND_QUEUELEN
#define TCP_WND (8 * TCP_MSS)
#define LWIP_TCP_KEEPALIVE 1

#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_CHECKSUM_CTRL_PER_NETIF 1

#define LWIP_STATS 0
#define LWIP_PROVIDE_ERRNO 1

#define LWIP_TIMEVAL_PRIVATE 0

#define PPP_SUPPORT 0

#define DHCP_DOES_ARP_CHECK 0
