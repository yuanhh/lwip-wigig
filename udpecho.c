#include <string.h>

#include "udpecho.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/ip_addr.h"
#include "lwip/api.h"
#include "lwip/sys.h"

/*-----------------------------------------------------------------------------------*/
    static void
udpsender_thread(void *arg)
{
    struct netconn *conn;
    struct netbuf *buf = 0;
    char buffer[4096];
    ip_addr_t ip;
    err_t err;
    LWIP_UNUSED_ARG(arg);

    memset(&ip, 0, sizeof(ip_addr_t));
    conn = netconn_new(NETCONN_UDP);
    netconn_bind(conn, IP_ADDR_ANY, 8001);
    LWIP_ERROR("udpecho: invalid conn", (conn != NULL), return;);
    IP4_ADDR(&ip,  10, 0, 0, 1);
    netconn_connect(conn, &ip, 8000);

    while (1) {
        memset(buffer, 0, 4096);
        fgets(buffer, 4096, stdin);
        if(netbuf_ref(buf, buffer, sizeof(buffer)) != buf->p->tot_len) {
            LWIP_DEBUGF(LWIP_DBG_ON, ("netbuf_copy failed\n"));
        } else {
            buffer[buf->p->tot_len] = '\0';
            err = netconn_send(conn, buf);
            if(err != ERR_OK) {
                LWIP_DEBUGF(LWIP_DBG_ON, ("netconn_send failed: %d\n", (int)err));
            } else {
                LWIP_DEBUGF(LWIP_DBG_ON, ("got %s\n", buffer));
            }
        }
        netbuf_delete(buf);
    }
}
/*-----------------------------------------------------------------------------------*/
    static void
udpecho_thread(void *arg)
{
    struct netconn *conn;
    struct netbuf *buf;
    char buffer[4096];
    err_t err;
    LWIP_UNUSED_ARG(arg);

    conn = netconn_new(NETCONN_UDP);
    netconn_bind(conn, IP_ADDR_ANY, 8000);
    LWIP_ERROR("udpecho: invalid conn", (conn != NULL), return;);

    while (1) {
        err = netconn_recv(conn, &buf);
        if (err == ERR_OK) {
            /*  no need netconn_connect here, since the netbuf contains the address */
            if(netbuf_copy(buf, buffer, sizeof(buffer)) != buf->p->tot_len) {
                LWIP_DEBUGF(LWIP_DBG_ON, ("netbuf_copy failed\n"));
            } else {
                buffer[buf->p->tot_len] = '\0';
                err = netconn_send(conn, buf);
                if(err != ERR_OK) {
                    LWIP_DEBUGF(LWIP_DBG_ON, ("netconn_send failed: %d\n", (int)err));
                } else {
                    LWIP_DEBUGF(LWIP_DBG_ON, ("got %s\n", buffer));
                }
            }
            netbuf_delete(buf);
        }
    }
}
/*-----------------------------------------------------------------------------------*/
    void
udpecho_init(void *arg)
{
    int mode = *(int *)arg;
    if (mode == 1)
        sys_thread_new("udpecho_thread", udpecho_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
    else
        sys_thread_new("udpsender_thread", udpsender_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

}

#endif /* LWIP_NETCONN */
