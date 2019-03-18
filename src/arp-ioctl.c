/*
 * arp-ioctl.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef HAVE_STREAMS_MIB2
# include <sys/sockio.h>
# include <sys/stream.h>
# include <sys/tihdr.h>
# include <sys/tiuser.h>
# include <inet/common.h>
# include <inet/mib2.h>
# include <inet/ip.h>
# undef IP_ADDR_LEN
#endif

#include <net/if.h>
#include <net/if_arp.h>
#ifdef HAVE_STREAMS_MIB2
# include <netinet/in.h>
# include <sys/ioctl.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dnet.h"

struct arp_handle {
	int	 fd;
};

arp_t *
arp_open(void)
{
	arp_t *a;
	
	if ((a = calloc(1, sizeof(*a))) != NULL) {
#ifdef HAVE_STREAMS_MIB2
		if ((a->fd = open(IP_DEV_NAME, O_RDWR)) < 0)
#elif defined(HAVE_STREAMS_ROUTE)
		if ((a->fd = open("/dev/route", O_WRONLY, 0)) < 0)
#else
		if ((a->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
#endif
			return (arp_close(a));
	}
	return (a);
}

int
arp_add(arp_t *a, const struct arp_entry *entry)
{
	struct arpreq ar;

	memset(&ar, 0, sizeof(ar));

	if (addr_ntos(&entry->arp_pa, &ar.arp_pa) < 0)
		return (-1);

	/* XXX - see arp(7) for details... */
#ifdef __linux__
	if (addr_ntos(&entry->arp_ha, &ar.arp_ha) < 0)
		return (-1);
	ar.arp_ha.sa_family = ARP_HRD_ETH;
#else
	/* XXX - Solaris, HP-UX, IRIX, other Mentat stacks? */
	ar.arp_ha.sa_family = AF_UNSPEC;
	memcpy(ar.arp_ha.sa_data, &entry->arp_ha.addr_eth, ETH_ADDR_LEN);
#endif

	ar.arp_flags = ATF_PERM | ATF_COM;
	if (ioctl(a->fd, SIOCSARP, &ar) < 0)
		return (-1);

#ifdef HAVE_STREAMS_MIB2
	/* XXX - force entry into ipNetToMediaTable. */
	{
		struct sockaddr_in sin;
		int fd;
		
		addr_ntos(&entry->arp_pa, (struct sockaddr *)&sin);
		sin.sin_port = htons(666);
		
		if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			return (-1);
		
		if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
			close(fd);
			return (-1);
		}
		write(fd, NULL, 0);
		close(fd);
	}
#endif
	return (0);
}

int
arp_delete(arp_t *a, const struct arp_entry *entry)
{
	struct arpreq ar;

	memset(&ar, 0, sizeof(ar));
	
	if (addr_ntos(&entry->arp_pa, &ar.arp_pa) < 0)
		return (-1);
	
	if (ioctl(a->fd, SIOCDARP, &ar) < 0)
		return (-1);

	return (0);
}

int
arp_get(arp_t *a, struct arp_entry *entry)
{
	struct arpreq ar;

	memset(&ar, 0, sizeof(ar));
	
	if (addr_ntos(&entry->arp_pa, &ar.arp_pa) < 0)
		return (-1);
	
	if (ioctl(a->fd, SIOCGARP, &ar) < 0)
		return (-1);

	if ((ar.arp_flags & ATF_COM) == 0) {
		errno = ESRCH;
		return (-1);
	}
	return (addr_ston(&ar.arp_ha, &entry->arp_ha));
}

#ifdef HAVE_STREAMS_MIB2
int
arp_loop(arp_t *r, arp_handler callback, void *arg)
{
	struct arp_entry entry;
	struct strbuf msg;
	struct T_optmgmt_req *tor;
	struct T_optmgmt_ack *toa;
	struct T_error_ack *tea;
	struct opthdr *opt;
	mib2_ipNetToMediaEntry_t *arp, *arpend;
	u_char buf[8192];
	int flags, rc, atable, ret;

	tor = (struct T_optmgmt_req *)buf;
	toa = (struct T_optmgmt_ack *)buf;
	tea = (struct T_error_ack *)buf;

	tor->PRIM_type = T_OPTMGMT_REQ;
	tor->OPT_offset = sizeof(*tor);
	tor->OPT_length = sizeof(*opt);
	tor->MGMT_flags = T_CURRENT;
	
	opt = (struct opthdr *)(tor + 1);
	opt->level = MIB2_IP;
	opt->name = opt->len = 0;
	
	msg.maxlen = sizeof(buf);
	msg.len = sizeof(*tor) + sizeof(*opt);
	msg.buf = buf;
	
	if (putmsg(r->fd, &msg, NULL, 0) < 0)
		return (-1);
	
	opt = (struct opthdr *)(toa + 1);
	msg.maxlen = sizeof(buf);
	
	for (;;) {
		flags = 0;
		if ((rc = getmsg(r->fd, &msg, NULL, &flags)) < 0)
			return (-1);

		/* See if we're finished. */
		if (rc == 0 &&
		    msg.len >= sizeof(*toa) &&
		    toa->PRIM_type == T_OPTMGMT_ACK &&
		    toa->MGMT_flags == T_SUCCESS && opt->len == 0)
			break;

		if (msg.len >= sizeof(*tea) && tea->PRIM_type == T_ERROR_ACK)
			return (-1);
		
		if (rc != MOREDATA || msg.len < (int)sizeof(*toa) ||
		    toa->PRIM_type != T_OPTMGMT_ACK ||
		    toa->MGMT_flags != T_SUCCESS)
			return (-1);
		
		atable = (opt->level == MIB2_IP && opt->name == MIB2_IP_22);
		
		msg.maxlen = sizeof(buf) - (sizeof(buf) % sizeof(*arp));
		msg.len = 0;
		flags = 0;
		
		do {
			rc = getmsg(r->fd, NULL, &msg, &flags);
			
			if (rc != 0 && rc != MOREDATA)
				return (-1);
			
			if (!atable)
				continue;
			
			arp = (mib2_ipNetToMediaEntry_t *)msg.buf;
			arpend = (mib2_ipNetToMediaEntry_t *)
			    (msg.buf + msg.len);

			entry.arp_pa.addr_type = ADDR_TYPE_IP;
			entry.arp_pa.addr_bits = IP_ADDR_BITS;
			
			entry.arp_ha.addr_type = ADDR_TYPE_ETH;
			entry.arp_ha.addr_bits = ETH_ADDR_BITS;

			for ( ; arp < arpend; arp++) {
				entry.arp_pa.addr_ip =
				    arp->ipNetToMediaNetAddress;
				
				memcpy(&entry.arp_ha.addr_eth,
				    arp->ipNetToMediaPhysAddress.o_bytes,
				    ETH_ADDR_LEN);
				
				if ((ret = callback(&entry, arg)) != 0)
					return (ret);
			}
		} while (rc == MOREDATA);
	}
	return (0);
}
#elif defined(HAVE_SYS_MIB_H)
#define MAX_ARPENTRIES	512	/* XXX */

int
arp_loop(arp_t *r, arp_handler callback, void *arg)
{
	struct nmparms nm;
	struct arp_entry entry;
	mib_ipNetToMediaEnt arpentries[MAX_ARPENTRIES];
	int fd, i, n, ret;
	
	nm.objid = ID_ipNetToMediaTable;
	nm.buffer = arpentries;
	n = sizeof(arpentries);
	nm.len = &n;
	
	if (get_mib_info(fd, &nm) < 0) {
		close_mib(fd);
		return (-1);
	}
	close_mib(fd);

	entry.arp_pa.addr_type = ADDR_TYPE_IP;
	entry.arp_pa.addr_bits = IP_ADDR_BITS;

	entry.arp_ha.addr_type = ADDR_TYPE_ETH;
	entry.arp_ha.addr_bits = ETH_ADDR_BITS;
	
	n /= sizeof(*arpentries);
	ret = 0;
	
	for (i = 0; i < n; i++) {
		if (arpentries[i].Type == INTM_INVALID ||
		    arpentries[i].PhysAddr.o_length != ETH_ADDR_LEN)
			continue;
		
		entry.arp_pa.addr_ip = arpentries[i].NetAddr;
		memcpy(&entry.arp_ha.addr_eth, arpentries[i].PhysAddr.o_bytes,
		    ETH_ADDR_LEN);
		
		if ((ret = callback(&entry, arg)) != 0)
			break;
	}
	return (ret);
}
#elif defined(HAVE_NET_RADIX_H) && !defined(_AIX)
/* XXX - Tru64, others? */
#include <netinet/if_ether.h>
#include <nlist.h>

static int
_kread(int fd, void *addr, void *buf, int len)
{
	if (lseek(fd, (off_t)addr, SEEK_SET) == (off_t)-1L)
		return (-1);
	return (read(fd, buf, len) == len ? 0 : -1);
}

static int
_radix_walk(int fd, struct radix_node *rn, arp_handler callback, void *arg)
{
	struct radix_node rnode;
	struct rtentry rt;
	struct sockaddr_in sin;
	struct arptab at;
	struct arp_entry entry;
	int ret = 0;
 again:
	_kread(fd, rn, &rnode, sizeof(rnode));
	if (rnode.rn_b < 0) {
		if (!(rnode.rn_flags & RNF_ROOT)) {
			_kread(fd, rn, &rt, sizeof(rt));
			_kread(fd, rt_key(&rt), &sin, sizeof(sin));
			addr_ston((struct sockaddr *)&sin, &entry.arp_pa);
			_kread(fd, rt.rt_llinfo, &at, sizeof(at));
			if (at.at_flags & ATF_COM) {
				addr_pack(&entry.arp_ha, ADDR_TYPE_ETH,
				    ETH_ADDR_BITS, at.at_hwaddr, ETH_ADDR_LEN);
				if ((ret = callback(&entry, arg)) != 0)
					return (ret);
			}
		}
		if ((rn = rnode.rn_dupedkey))
			goto again;
	} else {
		rn = rnode.rn_r;
		if ((ret = _radix_walk(fd, rnode.rn_l, callback, arg)) != 0)
			return (ret);
		if ((ret = _radix_walk(fd, rn, callback, arg)) != 0)
			return (ret);
	}
	return (ret);
}

int
arp_loop(arp_t *r, arp_handler callback, void *arg)
{
	struct ifnet *ifp, ifnet;
	struct ifnet_arp_cache_head ifarp;
	struct radix_node_head *head;
	
	struct nlist nl[2];
	int fd, ret = 0;

	memset(nl, 0, sizeof(nl));
	nl[0].n_name = "ifnet";
	
	if (knlist(nl) < 0 || nl[0].n_type == 0 ||
	    (fd = open("/dev/kmem", O_RDONLY, 0)) < 0)
		return (-1);

	for (ifp = (struct ifnet *)nl[0].n_value;
	    ifp != NULL; ifp = ifnet.if_next) {
		_kread(fd, ifp, &ifnet, sizeof(ifnet));
		if (ifnet.if_arp_cache_head != NULL) {
			_kread(fd, ifnet.if_arp_cache_head,
			    &ifarp, sizeof(ifarp));
			/* XXX - only ever one rnh, only ever AF_INET. */
			if ((ret = _radix_walk(fd, ifarp.arp_cache_head.rnh_treetop,
				 callback, arg)) != 0)
				break;
		}
	}
	close(fd);
	return (ret);
}
#else
int
arp_loop(arp_t *a, arp_handler callback, void *arg)
{
	errno = ENOSYS;
	return (-1);
}
#endif

arp_t *
arp_close(arp_t *a)
{
	if (a != NULL) {
		if (a->fd >= 0)
			close(a->fd);
		free(a);
	}
	return (NULL);
}
