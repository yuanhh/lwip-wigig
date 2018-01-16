CC=g++
CCDEP=$(CC)

#CFLAGS += -DLWIP_DEBUG
ARFLAGS=rs

TOPDIR=.
TARGETDIR=$(TOPDIR)/tmplib
LWIPARCH=$(TOPDIR)

#Set this to where you have the lwip core module checked out from CVS
#default assumes it's a dir named lwip at the same level as the contrib module
LWIPDIR=$(TOPDIR)/lwip/src

ML_LIBDIR = -Llib/x86_64
LDFLAGS = -pthread
ML_LIBS = -lMrLoopBF -lusb-1.0 -lpthread

CFLAGS += -Wall	-g -std=c++11							\
	-I$(LWIPDIR)/include -I.							\
	-I$(LWIPARCH)/include/lwip -I$(LWIPARCH)/include	\
	-I$(LWIPDIR)/include/ipv4

ML_LFLAGS = -DRF_STATUS $(ML_LIBDIR) $(ML_LIBS)

# COREFILES, CORE4FILES: The minimum set of files needed for lwIP.
COREFILES=$(LWIPDIR)/core/def.c 		\
    $(LWIPDIR)/core/dns.c 				\
    $(LWIPDIR)/core/inet_chksum.c 		\
    $(LWIPDIR)/core/init.c              \
    $(LWIPDIR)/core/ip.c				\
    $(LWIPDIR)/core/mem.c 				\
    $(LWIPDIR)/core/memp.c 				\
    $(LWIPDIR)/core/netif.c 			\
    $(LWIPDIR)/core/pbuf.c 				\
    $(LWIPDIR)/core/raw.c 				\
    $(LWIPDIR)/core/stats.c 			\
    $(LWIPDIR)/core/sys.c               \
    $(LWIPDIR)/core/tcp.c 				\
    $(LWIPDIR)/core/tcp_in.c 			\
    $(LWIPDIR)/core/tcp_out.c 			\
    $(LWIPDIR)/core/timeouts.c 			\
    $(LWIPDIR)/core/udp.c				\
	$(LWIPARCH)/src/sys_arch.c

CORE4FILES=$(LWIPDIR)/core/ipv4/autoip.c 	\
    $(LWIPDIR)/core/ipv4/dhcp.c				\
    $(LWIPDIR)/core/ipv4/etharp.c			\
    $(LWIPDIR)/core/ipv4/icmp.c				\
    $(LWIPDIR)/core/ipv4/ip4.c				\
    $(LWIPDIR)/core/ipv4/ip4_addr.c			\
    $(LWIPDIR)/core/ipv4/ip4_frag.c

APIFILES=$(LWIPDIR)/api/tcpip.c			\
		 $(LWIPDIR)/api/api_lib.c		\
		 $(LWIPDIR)/api/api_msg.c		\
		 $(LWIPDIR)/api/err.c			\
		 $(LWIPDIR)/api/netbuf.c		\
		 $(LWIPDIR)/api/netdb.c			\
		 $(LWIPDIR)/api/netifapi.c		\
		 $(LWIPDIR)/api/sockets.c

# NETIFFILES: Files implementing various generic network interface functions.'
NETIFFILES=$(LWIPDIR)/netif/ethernet.c

MAIN_FILES=ethernetif_driver.c lowlevel_wigig.c mem.c lwip-wigig.c

# LWIPFILES: All the above.
LWIPFILES=$(MAIN_FILES) $(COREFILES) $(CORE4FILES) $(NETIFFILES) $(APIFILES)
OBJS=$(LWIPFILES:.c=.o)

LWIPMAIN=lwip-wigig

all compile: $(LWIPMAIN)
	mkdir -p $(TARGETDIR)
#	install $(LWIPLIB) $(TARGETDIR)

.PHONY: all depend compile clean

%.o:
	$(CC) $(LDFLAGS) $(CFLAGS) -c $(@:.o=.c) -o $@

clean:
	rm -f *.o $(LWIPMAIN) .depend*
	find . -name \*.o |xargs --no-run-if-empty rm

$(LWIPMAIN): $(OBJS)
	$(CC) -o $(LWIPMAIN) $? $(ML_LFLAGS)

depend dep: .depend

include .depend

.depend: $(LWIPFILES)
	$(CCDEP) $(CFLAGS) -MM $^ > .depend || rm -f .depend
