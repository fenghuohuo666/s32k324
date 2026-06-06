#ifndef LWIP_UDP_H
#define LWIP_UDP_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* Minimal LWIP UDP stub */
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t err_t;

typedef struct ip_addr {
    uint32_t addr;
} ip_addr_t;

typedef struct udp_pcb {
    void *dummy;
} udp_pcb;

typedef struct pbuf {
    void *payload;
    u16_t len;
} pbuf;

#define PBUF_TRANSPORT 0
#define PBUF_RAM       0
#define ERR_OK         0

static inline struct pbuf *pbuf_alloc(int layer, u16_t length, int type)
{
    (void)layer;
    (void)type;
    struct pbuf *p = (struct pbuf *)malloc(sizeof(struct pbuf));
    if (p) {
        p->payload = malloc(length);
        p->len = length;
    }
    return p;
}

static inline void pbuf_free(struct pbuf *p)
{
    if (p) {
        free(p->payload);
        free(p);
    }
}

static inline err_t udp_sendto(struct udp_pcb *pcb, struct pbuf *p,
                               const ip_addr_t *dst_ip, u16_t dst_port)
{
    (void)pcb;
    (void)p;
    (void)dst_ip;
    (void)dst_port;
    return ERR_OK;
}

static inline void ip4addr_aton(const char *cp, ip_addr_t *addr)
{
    (void)cp;
    addr->addr = 0;
}

#endif /* LWIP_UDP_H */
