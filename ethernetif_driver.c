#include <stdint.h>
#include <string.h>
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "netif/etharp.h"

#include "ethernetif.h"
#include "lowlevel_wigig.h"

#define IFNAME0 'w'
#define IFNAME1 'g'

#ifndef ETHERNETIF_MAXFRAMES
#define ETHERNETIF_MAXFRAMES 0
#endif

void ethernetif_input(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    struct ethernetif *ethernetif;
    struct eth_hdr *ethhdr;
    struct pbuf *p, *q;
    int len;
    int frames = 0;
    uint16_t offset = 0;

    ethernetif = (struct ethernetif *)netif->state;
    do {
        if((len = ethernetif->low_level_startinput(ethernetif->internals)) == 0)
            break;
        /* move received packet into a new pbuf */
#if ETH_PAD_SIZE
        len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        if (p != NULL) {
#if ETH_PAD_SIZE
            pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
            for(q = p; q != NULL; q = q->next) {
                ethernetif->low_level_input(ethernetif->internals,
                        q->payload, q->len, offset);
                offset += q->len;
            }
            ethernetif->low_level_endinput(ethernetif->internals);
#if ETH_PAD_SIZE
            pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
            LINK_STATS_INC(link.recv);
        } else {
#if ETH_PAD_SIZE
            len -= ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
            ethernetif->low_level_input_nomem(ethernetif->internals, len);
            LINK_STATS_INC(link.memerr);
            LINK_STATS_INC(link.drop);
            return;
        }
        ethhdr = (struct eth_hdr *)p->payload;

        switch (htons(ethhdr->type)) {
            case ETHTYPE_IP:
            case ETHTYPE_ARP:
                if (netif->input(p, netif)!=ERR_OK)
                { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
                    pbuf_free(p);
                    p = NULL;
                }
                break;

            default:
                pbuf_free(p);
                p = NULL;
                break;
        }
    } while((!ETHERNETIF_MAXFRAMES) || (++frames < ETHERNETIF_MAXFRAMES));
}   

static err_t ethernetif_linkoutput(struct netif *netif, struct pbuf *p)
{
    struct ethernetif *ethernetif = (struct ethernetif *)netif->state;
    struct pbuf *q;
    uint16_t offset = 0;

    if(!ethernetif->low_level_startoutput(ethernetif->internals))
        return ERR_IF;

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    for(q = p; q != NULL; q = q->next) {
        ethernetif->low_level_output(ethernetif->internals,
                q->payload, q->len, offset);
        offset += q->len;
    }
    ethernetif->low_level_endoutput(ethernetif->internals, p->tot_len);

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.xmit);

    return ERR_OK;
}

err_t ethernetif_init(struct netif *netif)
{
    struct ethernetif *ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));
    LWIP_ASSERT("state != NULL", (netif->state != NULL));

    ethernetif = (struct ethernetif *)netif->state;

    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 1000000000);

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = etharp_output;
    netif->linkoutput = ethernetif_linkoutput;
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    memcpy(netif->hwaddr, ethernetif->address, ETHARP_HWADDR_LEN);

    netif->mtu = ETHERNET_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    ethernetif->low_level_init(&ethernetif->internals, ethernetif->address, NULL);

    sys_thread_new("ethernetif_thread", ethernetif_input, netif, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    return ERR_OK;
}
