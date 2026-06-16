/**
 * @file  lwipopts.h
 * @brief Configuracion de lwIP para Raspberry Pi Pico W (modo polling, sin RTOS)
 *
 * Basado en los ejemplos oficiales del Pico SDK:
 * pico-sdk/src/rp2_common/pico_lwip/include/arch/cc.h
 */
#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

/* --- Sistema operativo: ninguno (bare metal polling) --- */
#define NO_SYS                      1
#define SYS_LIGHTWEIGHT_PROT        0
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_NETIF_API              0

/* --- Memoria --- */
#define MEM_LIBC_MALLOC             0
#define MEMP_MEM_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24
#define PBUF_POOL_BUFSIZE           1514

/* --- ARP --- */
#define LWIP_ARP                    1
#define ARP_TABLE_SIZE              10
#define ARP_QUEUEING                0

/* --- IP --- */
#define IP_FORWARD                  0
#define IP_OPTIONS_ALLOWED          1
#define IP_REASSEMBLY               1
#define IP_FRAG                     1
#define IP_DEFAULT_TTL              255

/* --- ICMP --- */
#define LWIP_ICMP                   1

/* --- DHCP --- */
#define LWIP_DHCP                   1
#define DHCP_DOES_ARP_CHECK         0

/* --- DNS --- */
#define LWIP_DNS                    1

/* --- UDP --- */
#define LWIP_UDP                    1
#define UDP_TTL                     255

/* --- TCP --- */
#define LWIP_TCP                    1
#define TCP_TTL                     255
#define TCP_QUEUE_OOSEQ             0
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_MAXRTX                  12
#define TCP_SYNMAXRTX               4
#define LWIP_TCP_KEEPALIVE          1

/* --- Callbacks / estados --- */
#define LWIP_CALLBACK_API           1
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1

/* --- Estadisticas: deshabilitadas para ahorrar memoria --- */
#define LWIP_STATS                  0
#define LWIP_STATS_DISPLAY          0

/* --- Debugging: deshabilitado en produccion --- */
#define LWIP_DEBUG                  0

/* --- Checksum: calcular en software --- */
#define CHECKSUM_GEN_IP             1
#define CHECKSUM_GEN_UDP            1
#define CHECKSUM_GEN_TCP            1
#define CHECKSUM_CHECK_IP           1
#define CHECKSUM_CHECK_UDP          1
#define CHECKSUM_CHECK_TCP          1
#define CHECKSUM_CHECK_ICMP         1

#endif /* _LWIPOPTS_H */
